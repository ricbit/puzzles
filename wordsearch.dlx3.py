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
    self.xlimit = len(words[0])
    self.freecell = freecell
    self.crossing = crossing
    self.grid = self.build_grid()
    self.word_vectors = self.build_valid_word_vectors()
    self.word_indexes = self.build_word_indexes()
    self.word_hashes = self.build_word_hashes()
    self.word_crossings = self.build_word_crossings()
    if verbose:
      self.print_info()

  def print_info(self):
    for nw, wordvecs in self.word_vectors.items():
      print("%s: %d" % (self.words[nw], len(wordvecs)), file=sys.stderr)
    return
    for nw, hashes in self.word_hashes.items():
      for _, wh in hashes.items():
        print("hash %d %s" % (nw, wh), file=sys.stderr)

  def valid_rect(self, j, i, nj, ni):
    if nj < 1 - self.size or nj >= self.size:
      return False
    if j < 1 - self.size or j >= self.size:
      return False
    if ni >= self.size or i >= self.size:
      return False
    if i < self.xlimit - self.size or ni < self.xlimit - self.size:
      return False
    return True

  def iter_yrange(self):
    return range(1 - self.size, self.size)

  def iter_xrange(self):
    return range(self.xlimit - self.size, self.size)

  def iter_range(self):
    return itertools.product(self.iter_yrange(), self.iter_xrange())

  def iter_vector(self, word):
    for jj, ii in iter_directions():
      for j, i in self.iter_range():
        nj = j + jj * (len(word) - 1)
        ni = i + ii * (len(word) - 1)
        if self.valid_rect(j, i, nj, ni):
          yield (j, i, jj, ii)

  def iter_word_vector(self):
    for nw, word in enumerate(self.words):
      if nw == 0:
        yield nw, word, 0, 0, 0, 1, False
      else:
        for j, i, jj, ii in self.iter_vector(word):
          if nw > 1 or j > 0 or (j == 0 and jj > 0):
            yield nw, word, j, i, jj, ii, False

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
    for pj, pi in self.iter_range():
      for letter, words in self.grid[pj][pi].items():
        for nw1, nw2 in itertools.combinations(words, 2):
          yield pj, pi, letter, nw1, nw2, words[nw1], words[nw2]

  def encode(self, j):
    return chr(65 + j)

  def encode_hash(self, nw, j, i, jj, ii):
    word_index = self.word_indexes[nw][(j, i, jj, ii)]
    return dlx.encode(word_index, len(self.word_vectors[nw]))

  def build_grid(self):
    limits = list(range(1 - self.size, 1 + self.size))
    grid = [[{} for w in limits] for h in limits]
    for nw, word, j, i, jj, ii, sym in self.iter_word_vector():
      for k, pj, pi, letter in self.iter_word_letters(word, j, i, jj, ii):
        grid[pj][pi].setdefault(letter, {}).setdefault(nw, []).append(
            (k, j, i, jj, ii, sym))
    return grid

  def build_valid_word_vectors(self):
    valid_word_vectors = set()
    for pj, pi in self.iter_range():
      for letter, word_vectors in self.grid[pj][pi].items():
        if len(word_vectors) > 2:
          for nw, vectors in word_vectors.items():
            for k, j, i, jj, ii, sym in vectors:
              valid_word_vectors.add(
                  (nw, self.words[nw], j, i, jj, ii, sym))
    ans = {}
    for nw, word in enumerate(self.words):
      ans[nw] = [vec[1:] for vec in valid_word_vectors if vec[0] == nw]
    return ans

  def build_word_indexes(self):
    indexes = {}
    for nw, wordvecs in self.word_vectors.items():
      indexes[nw] = {}
      for k, (word, j, i, jj, ii, sym) in enumerate(wordvecs):
        indexes[nw][(j, i, jj, ii)] = k
    return indexes

  def build_word_hashes(self):
    hashes = {}
    for nw, wordvecs in self.word_indexes.items():
      hashes[nw] = {}
      for j, i, jj, ii in wordvecs:
        word_hash = self.encode_hash(nw, j, i, jj, ii)
        hashes[nw][(j, i, jj, ii)] = word_hash
    return hashes

  def build_word_crossings(self):
    crossings = {}
    for pj, pi in self.iter_range():
      for letter, words in self.grid[pj][pi].items():
        for nw1, nw2 in itertools.combinations(sorted(words), 2):
          for word1, word2 in itertools.product(words[nw1], words[nw2]):
            _, j1, i1, jj1, ii1, _ = word1
            _, j2, i2, jj2, ii2, _ = word2
            hash1 = self.word_indexes[nw1].get((j1, i1, jj1, ii1), None)
            hash2 = self.word_indexes[nw2].get((j2, i2, jj2, ii2), None)
            if (hash1 is None) or (hash2 is None):
              continue
            crossings.setdefault((nw1, nw2), set()).add((hash1, hash2))
    return crossings

  def add_columns(self, j, i, nj, ni):
    mini, maxi = min(i, ni), max(i, ni)
    minj, maxj = min(j, nj), max(j, nj)
    for i2 in range(mini, 1 + maxi):
      yield "c%d:on" % i2
    for i2 in self.iter_xrange():
      if i2 <= maxi - self.size or i2 >= mini + self.size:
        yield "c%d:off" % i2
    for j2 in range(minj, 1 + maxj):
      yield "r%d:on" % j2
    for j2 in self.iter_yrange():
      if j2 <= maxj - self.size or j2 >= minj + self.size:
        yield "r%d:off" % j2

  def option_word(self, nw, word, j, i, jj, ii, sym):
    option = ["W%d" % nw]
    if sym:
      option.append("sym")
    for k, pj, pi, letter in self.iter_word_letters(word, j, i, jj, ii):
      option.append("p%d_%d:%d" % (pj, pi, ord(letter)))
    nj = j + jj * (len(word) - 1)
    ni = i + ii * (len(word) - 1)
    option.extend(self.add_columns(j, i, nj, ni))
    if self.crossing:
      option.append("w%d:%d" % (nw, 100 + self.word_indexes[nw][(j, i, jj, ii)]))
      if nw == 0:
        option.append("t0:1")
    yield " ".join(option)

  def collect_words(self):
    if self.crossing:
      word_iter = self.iter_valid_word_vector
    else:
      word_iter = self.iter_word_vector
    for word in word_iter():
      yield from self.option_word(*word)

  def collect_free_cell(self):
    for j, i in self.iter_range():
      items = ["P",  "p%d_%d:%d" % (j, i, ord("."))]
      items.extend(self.add_columns(j, i, j, i))
      yield " ".join(items)

  def flip(self, seq):
    for a, b in seq:
      yield a, b
      yield b, a

  def valid_word_pairs(self):
    for (nw1a, nw2a), cross in self.word_crossings.items():
      wordpairs = [(nw1a, nw2a)]
      if nw1a > 0:
        wordpairs = self.flip(wordpairs)
      for nw1, nw2 in wordpairs:
        yield (nw1, nw2), cross

  def collect_crossings(self):
    for (nw1, nw2), cross in self.valid_word_pairs():
      seen = collections.defaultdict(lambda: set())
      for hash1, hash2 in cross:
        seen[hash1].add(hash2)
        base = ["C%d_%d" % (nw1, nw2)]
        base.append("w%d:%d" % (nw1, 100 + hash1))
        base.append("w%d:%d" % (nw2, 100 + hash2))
        base.append("x%d_%d:on" % (nw1, nw2))
        for d in range(2, self.depth):
          option = base[:]
          if (nw1 > 0 and d > 2) or (nw1 == 0 and d == 2):
            option.append("t%d:%d" % (nw1, d - 1))
            option.append("t%d:%d" % (nw2, d))
            yield " ".join(option)
      option = ["C%d_%d" % (nw1, nw2)]
      option.append("x%d_%d:off" % (nw1, nw2))
      yield " ".join(option)

  def collect_trees(self):
    for nw1, nw2 in self.flip(self.word_crossings):
      if nw1 == 0:
        continue
      mindepth = 1 if nw2 == 0 else 2
      maxdepth = 2 if nw2 == 0 else self.depth
      for i in range(mindepth, maxdepth):
        option = ["T%d" % nw1]
        if nw1 < nw2:
          option.append("x%d_%d:1" % (nw1, nw2))
        else:
          option.append("x%d_%d:1" % (nw2, nw1))
        option.append("t%d:%d" % (nw1, i))
        option.append("t%d:%d" % (nw2, i - 1))
        yield " ".join(option)

  def solve(self):
    options = []
    options.extend(self.collect_words())
    if self.freecell:
      options.extend(self.collect_free_cell())
    if self.crossing:
      options.extend(self.collect_crossings())
      #options.extend(self.collect_word_numbers())
      #options.extend(self.collect_trees())
    yield from dlx.build_dlx(options, on_off=True, sorted_items=True)

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
