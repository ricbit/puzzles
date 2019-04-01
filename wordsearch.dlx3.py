#!/usr/bin/python3

import sys
import itertools
import dlx
import argparse
import collections

def iter_directions():
  for a, b in itertools.product([-1, 0, 1], repeat=2):
    if a != 0 or b != 0:
      yield a, b

class WordSearch:
  def __init__(self, words, size, depth, freecell, crossing, verbose):
    self.words = words
    self.size = size
    self.depth = depth
    self.freecell = freecell
    self.grid = self.build_grid()
    self.crossing = crossing
    self.word_vectors = self.build_valid_word_vectors()
    self.word_indexes = self.build_word_indexes()
    self.word_hashes = self.build_word_hashes()
    self.word_crossings = self.build_word_crossings()
    if verbose:
      self.print_info()

  def print_info(self):
    for nw, wordvecs in self.word_vectors.items():
      print("%s: %d" % (self.words[nw], len(wordvecs)), file=sys.stderr)

  def iter_vector(self, word):
    for jj, ii in iter_directions():
      for j, i in itertools.product(range(self.size), repeat=2):
        nj = j + jj * (len(word) - 1)
        ni = i + ii * (len(word) - 1)
        if 0 <= nj < self.size and 0 <= ni < self.size:
          yield (j, i, jj, ii)

  def iter_word_vector(self):
    for nw, word in enumerate(self.words):
      for j, i, jj, ii in self.iter_vector(word):
        sym = False
        if nw == 0:
          if jj < 0 or ii < 0:
            continue
          if jj == 0 and j < self.size // 2:
            continue
          if ii == 0:
            continue
          if jj == 1 and ii == 1:
            sym = True
        if nw == 1 and jj == 0:
          sym = True
        yield nw, word, j, i, jj, ii, sym

  def iter_word_letters(self, word, j, i, jj, ii):
    for k, letter in enumerate(word):
      pj = j + k * jj
      pi = i + k * ii
      yield k, pj, pi, letter

  def iter_valid_word_vector(self):
    for nw, wordvecs in self.word_vectors.items():
      for word, j, i, jj, ii, sym in wordvecs:
        yield nw, word, j, i, jj, ii, sym

  def iter_word_crossings(self):
    for pj, pi in dlx.iter_grid(self.size, self.size):
      for letter, words in self.grid[pj][pi].items():
        for nw1, nw2 in itertools.combinations(words, 2):
          yield pj, pi, letter, nw1, nw2, words[nw1], words[nw2]

  def encode(self, j):
    return chr(65 + j)

  def encode_hash(self, nw, j, i, jj, ii):
    word_index = self.word_indexes[(nw, j, i, jj, ii)]
    if word_index:
      return dlx.encode(word_index, len(self.word_vectors[nw]))
    else:
      return None

  def build_grid(self):
    grid = [[{} for w in range(self.size)] for h in range(self.size)]
    for nw, word, j, i, jj, ii, sym in self.iter_word_vector():
      for k, pj, pi, letter in self.iter_word_letters(word, j, i, jj, ii):
        grid[pj][pi].setdefault(letter, {}).setdefault(nw, []).append(
            (k, j, i, jj, ii, sym))
    return grid

  def build_valid_word_vectors(self):
    valid_word_vectors = {}
    for pj, pi in dlx.iter_grid(self.size, self.size):
      for letter, word_vectors in self.grid[pj][pi].items():
        if len(word_vectors) > 2:
          for nw, vectors in word_vectors.items():
            for k, j, i, jj, ii, sym in vectors:
              valid_word_vectors.setdefault(nw, set()).add(
                  (self.words[nw], j, i, jj, ii, sym))
    return {nw: list(wordvecs) for nw, wordvecs in valid_word_vectors.items()}

  def build_word_indexes(self):
    indexes = collections.defaultdict(lambda: None)
    for nw, wordvecs in self.word_vectors.items():
      for k, (word, j, i, jj, ii, sym) in enumerate(wordvecs):
        indexes[(nw, j, i, jj, ii)] = k
    return indexes

  def build_word_hashes(self):
    hashes = collections.defaultdict(lambda: None)
    for nw, wordvecs in self.word_vectors.items():
      for k, (word, j, i, jj, ii, sym) in enumerate(wordvecs):
        vec = self.encode_hash(nw, j, i, jj, ii)
        hashes[(nw, j, i, jj, ii)] = vec
    return hashes

  def build_word_crossings(self):
    crossings = {}
    for pj, pi in dlx.iter_grid(self.size, self.size):
      for letter, words in self.grid[pj][pi].items():
        for nw1, nw2 in itertools.combinations(sorted(words), 2):
          for word1, word2 in itertools.product(words[nw1], words[nw2]):
            _, j1, i1, jj1, ii1, _ = word1
            _, j2, i2, jj2, ii2, _ = word2
            hash1 = self.word_hashes[(nw1, j1, i1, jj1, ii1)]
            hash2 = self.word_hashes[(nw2, j2, i2, jj2, ii2)]
            if hash1 is None or hash2 is None:
              continue
            crossings.setdefault((nw1, nw2), set()).add((hash1, hash2))
    return crossings

  def option_word(self, nw, word, j, i, jj, ii, sym):
    option = ["W%d" % nw]
    if sym:
      option.append("sym")
    used = set()
    for k, pj, pi, letter in self.iter_word_letters(word, j, i, jj, ii):
      option.append("p%d_%d:%s" % (pj, pi, letter))
      used.add((pj, pi))
    if self.crossing:
      word_hash = self.encode_hash(nw, j, i, jj, ii)
      for _, kj, ki, kjj, kii, _ in self.word_vectors[nw]:
        k_hash = self.encode_hash(nw, kj, ki, kjj, kii)
        option.append("h%d_%s:%d" % (nw, k_hash, int(word_hash == k_hash)))
    yield " ".join(option)

  def collect_words(self):
    if self.crossing:
      word_iter = self.iter_valid_word_vector
    else:
      word_iter = self.iter_word_vector
    for word in word_iter():
      yield from self.option_word(*word)

  def collect_free_cell(self):
    for j, i in itertools.product(range(self.size), repeat=2):
      yield "P p%d_%d:." % (j, i)

  def collect_word_numbers(self):
    yield "N0 w0:0"
    for i in range(1, len(self.words)):
      for j in range(1, self.depth):
        option = ["N%d" % i]
        option.append("w%d:%d" % (i, j))
        yield " ".join(option)

  def collect_crossings(self):
    for (nw1, nw2), cross in self.word_crossings.items():
      seen = {}
      for hash1, hash2 in cross:
        seen.setdefault(hash1, set()).add(hash2)
        option = ["C%d_%d" % (nw1, nw2)]
        option.append("h%d_%s:1" % (nw1, hash1))
        option.append("h%d_%s:1" % (nw2, hash2))
        option.append("x%d_%d:1" % (nw1, nw2))
        yield " ".join(option)

  def solve(self):
    options = []
    options.append("DW%s" % self.size)
    options.append("DH%s" % self.size)
    options.extend(self.collect_words())
    if self.freecell:
      options.extend(self.collect_free_cell())
    if self.crossing:
      options.extend(self.collect_crossings())
    #options.extend(self.collect_word_numbers())
    yield from dlx.build_dlx(options, sorted_items=True)

def main():
  parser = argparse.ArgumentParser(
      description="Generate a word search dlx file from a list of words")
  parser.add_argument("--size", nargs=1, type=int, default=[6],
      help="Size of a square grid")
  parser.add_argument("--words", nargs=1, type=int, default=None,
      help="Size of a square grid")
  parser.add_argument("--depth", nargs=1, type=int, default=None,
      help="Maximum depth of the word tree")
  parser.add_argument("--freecell", action="store_true",
      help="Force at least one cell to be free")
  parser.add_argument("--crossing", action="store_true",
      help="All words must be connected")
  parser.add_argument("--verbose", action="store_true",
      help="Display debug information")
  args = parser.parse_args()
  size = args.size[0]
  words = []
  for line in sys.stdin:
    word = line.strip()
    if word:
      words.append(word)
  if args.words:
    words = words[:args.words[0]]
  words.sort(key=lambda x: len(x), reverse=True)
  if args.depth:
    depth = args.depth[0]
  else:
    depth = len(words) // 2
  if any(len(word) > size for word in words):
    print ("Invalid grid", file=sys.stderr)
    return
  solver = WordSearch(words, size, depth, args.freecell, args.crossing, args.verbose)
  print("\n".join(solver.solve()))

main()
