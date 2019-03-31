#!/usr/bin/python3

import sys
import itertools
import dlx
import argparse

def iter_directions():
  for a, b in itertools.product([-1, 0, 1], repeat=2):
    if a != 0 or b != 0:
      yield a, b

class WordSearch:
  def __init__(self, words, size, depth):
    self.words = words
    self.size = size
    self.depth = depth

  def iter_vector(self, word):
    for jj, ii in iter_directions():
      for j, i in itertools.product(range(self.size), repeat=2):
        nj = j + jj * (len(word) - 1)
        ni = i + ii * (len(word) - 1)
        if 0 <= nj < self.size and 0 <= ni < self.size:
          yield (j, i, jj, ii)

  def encode(self, j):
    return chr(65 + j)

  def iter_words(self):
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

  def option_word(self, nw, word, j, i, jj, ii, sym):
    option = ["W%d" % nw]
    if sym:
      option.append("sym")
    for k, letter in enumerate(word):
      pj = j + k * jj
      pi = i + k * ii
      option.append("p%d_%d:%s" % (pj, pi, letter))
      option.append("wj%d_%d:%s" % (nw, k, self.encode(pj)))
      option.append("wi%d_%d:%s" % (nw, k, self.encode(pi)))
    yield " ".join(option)  

  def collect_words(self):
    for word in self.iter_words():
      yield from self.option_word(*word)

  def collect_free_square(self):
    for j, i in itertools.product(range(self.size), repeat=2):
      yield "P p%d_%d:." % (j, i)

  def collect_word_numbers(self):
    yield "N0 w0:0"
    for i in range(1, len(self.words)):
      for j in range(1, self.depth):
        option = ["N%d" % i]
        option.append("w%d:%d" % (i, j))
        yield " ".join(option)

  def solve(self):
    options = []
    options.append("DW%s" % self.size)
    options.append("DH%s" % self.size)
    options.extend(self.collect_words())
    options.extend(self.collect_free_square())
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
  solver = WordSearch(words, size, depth)
  print("\n".join(solver.solve()))

main()
