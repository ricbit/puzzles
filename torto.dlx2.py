#!/usr/bin/python3
#
# Takes a list of words and build a torto model to be used with Knuth's dlx2 solver.
#
# If you want to use plain dlx2:
#   ./torto.dlx2.py < words.txt > model.dlx
#
# If you want to use "Partial Cover":
#   ./torto.dlx2.py --sharp < words.txt > model.dlx
#
# If you want to use higher ngrams:
#   ./torto.dlx2.py --ngrams  < words.txt > model.dlx

import sys
import itertools
import argparse
from collections import Counter

class IterTorto(object):
  def ngram_positions(self, n):
    self.diag = []
    for j in range(6):
      for i in range(3):
        self.ngram = [(j, i)]
        for ngram in self.iter(n - 1, j, i):
          yield ngram

  def iter_neigh(self, j, i):
    for dj, di in itertools.product(range(-1, 2), repeat=2):
      if dj == di == 0:
        continue
      nj, ni = dj + j, di + i
      if 0 <= ni < 3 and 0 <= nj < 6:
        yield (nj, ni)

  def iter(self, n, j, i):
    if n == 0:
      yield self.ngram
      return
    for nj, ni in self.iter_neigh(j, i):
      if (nj, ni) in self.ngram:
        continue
      has_diag = nj != j and ni != i
      if has_diag:
        bj = min(j, nj)
        bi = min(i, ni)
        if (bj, bi) in self.diag:
          continue
        else:
          self.diag.append((bj, bi))
      self.ngram.append((nj, ni))
      for ngram in self.iter(n - 1, nj, ni):
        yield ngram
      self.ngram.pop()
      if has_diag:
        self.diag.pop()

def readable_pos(p):
  return "%d%d" % p

def encode(x):
  limit = 11 + ord('z') - ord('a')
  if x < 10:
    return str(x)
  elif x < limit:
    return chr(x - 10 + ord('a'))
  else:
    return chr(x - limit + ord('A'))

def encode_pos(pos):
  return chr(ord('a') + pos[0] * 3 + pos[1])

def item_letter(n, i, pos):
  return "c%d_%d:%s" % (n, i, encode_pos(pos))

def diag(n, pa, pb):
  if pa[0] == pb[0] or pa[1] == pb[1]:
    return ""
  mj = min(pa[0], pb[0])
  mi = min(pa[1], pb[1])
  return "d%d_%d%d " % (n, mj, mi)

def id_generator(seq):
  yield from seq

def flipx(p):
  return (p[0], 2 - p[1])

def flipy(p):
  return (5 - p[0], p[1])

def flipxp(p):
  return tuple(map(flipx, p))

def flipyp(p):
  return tuple(map(flipy, p))

def remove_symmetry(seq):
  seen = set()
  for s in seq:
    q = tuple(s)
    if q not in seen:
      yield s
    seen.add(q)
    seen.add(flipxp(q))
    seen.add(flipyp(q))
    seen.add(flipyp(flipxp(q)))

def get_symmetry(n, i, middle):
  if n == 0 and i == middle:
    return remove_symmetry
  else:
    return id_generator

def collect_word(n, word, it, sharp, ngrams):
  if ngrams is None:
    ngramsize = 2
  else:
    wordsize, maxngram = map(int, ngrams)
    ngramsize = 2 if (len(word) > wordsize or n == 0) else maxngram
  start_positions = range(0, len(word) - 1, ngramsize - 1)
  middle_position = start_positions[len(start_positions) // 2]
  for i in start_positions:
    ngram = word[i:i + ngramsize]
    realsize = len(ngram)
    transform = get_symmetry(n, i, middle_position)
    for pos in transform(it.ngram_positions(realsize)):
      prefix = "#" if sharp else ""
      option = ["%sW%d_%d" % (prefix, n, i)]
      for j in range(realsize):
        option.append(item_letter(n, i + j, pos[j]))
        option.append("p%s:%s" % (readable_pos(pos[j]), word[i + j]))
        option.append("pw%d_%s:%s" % (n, encode_pos(pos[j]), encode(i + j)))
      for pa, pb in zip(pos, pos[1:]):
        if pa[0] != pb[0] and pa[1] != pb[1]:
          mj = min(pa[0], pb[0])
          mi = min(pa[1], pb[1])
          option.append("d%d_%d%d" % (n, mj, mi))
      yield " ".join(option)

def histogram(words):
  global_hist = Counter()
  for word in words:
    local_hist = Counter()
    for item in word:
      local_hist[item] += 1
    for item, count in local_hist.items():
      global_hist[item] = max(global_hist[item], count)
  return global_hist

def collect_letters(words):
  letters = histogram(words)
  for letter, count in letters.items():
    for c in range(count):
      for j in range(6):
        for i in range(3):
          p = (j, i)
          yield "L%s%d p%s:%s l%s%d:%s" % (
              letter, c, readable_pos(p), letter, letter, c, encode_pos(p))

def word_bigrams(words):
  for word in words:
    bigrams = [list(i) for i in zip(word, word[1:])]
    for bigram in bigrams:
      bigram.sort()
    yield ["".join(bigram) for bigram in bigrams]

def collect_bigrams(words, it):
  bigrams = histogram(word_bigrams(words))
  for bigram, count in bigrams.items():
    for edge in it.ngram_positions(2):
      yield "B%s%d p%s:%s p%s:%s b%s%d_0:%s b%s%d_1:%s" % (
          bigram, count,
          readable_pos(edge[0]), bigram[0],
          readable_pos(edge[1]), bigram[1],
          bigram, count, encode_pos(edge[0]),
          bigram, count, encode_pos(edge[1]))

def collect_empty():
  for j, i in itertools.product(range(6), range(3)):
    yield "P%d%d" % (j, i)
    yield "P%d%d p%s:-" % (j, i, pos((j, i)))

def extract_primary(option):
  for item in option.split():
    if ":" not in item and (item[0].isupper() or item[0] == "#"):
      yield item

def extract_secondary(option):
  for item in option.split():
    if ":" in item or item[0].islower():
      yield item.split(":")[0]

def main():
  parser = argparse.ArgumentParser(
      description="Generate a torto dlx file from a list of words")
  parser.add_argument("-s", "--sharp", action="store_true",
      help="Use the sharp heuristic")
  parser.add_argument("--ngram", nargs=2, metavar=("minsize", "ngramsize"),
      help="Use ngrams greater than bigrams")
  args = parser.parse_args()
  nwords = int(input())
  words = [input().strip() for _ in range(nwords)]
  words.sort(key=lambda w: len(w), reverse=True)
  options = []
  it = IterTorto()
  for n, word in enumerate(words):
    options.extend(collect_word(n, word, it, args.sharp, args.ngram))
  if args.sharp:
    options.extend(collect_letters(words))
    options.extend(collect_bigrams(words, it))
  primary = set()
  secondary = set()
  for option in options:
    for item in extract_primary(option):
      primary.add(item)
    for item in extract_secondary(option):
      secondary.add(item)
  print ("%s | %s" % (" ".join(primary), " ".join(secondary)))
  for option in options:
    print (option)

main()
