# Takes a list of words and build a torto model to be used with Knuth's dlx2 solver.
#
# If you want to use plain dlx2:
#   python torto.dlx2.py < words.txt > model.dlx
#
# If you want to use "Partial Cover":
#   python torto.dlx2.py --sharp < words.txt > model.dlx

import sys
import itertools
import argparse
from collections import Counter

def iter_torto():
  for j in xrange(6):
    for i in xrange(2):
      yield ("h%d%d" % (i, j), (j, i), (j, i + 1))
  for i in xrange(3):
    for j in xrange(5):
      yield ("v%d%d" % (j, i), (j, i), (j + 1, i))
  for i in xrange(2):
    for j in xrange(5):
      yield ("d%d%d" % (j, i), (j, i), (j + 1, i + 1))
      yield ("u%d%d" % (j, i + 1), (j, i + 1), (j + 1, i))

def flip(edges):
  for name, a, b in edges:
    yield (name, a, b)
    yield (name, b, a)

def pos(p):
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

def diag(n, name, pa, pb):
  if name[0] not in "ud":
    return ""
  mj = min(pa[0], pb[0])
  mi = min(pa[1], pb[1])
  return "d%d_%d%d " % (n, mj, mi)

def id_generator(seq):
  for s in seq:
    yield s

def flipx(p):
  return (p[0], 2 - p[1])

def flipy(p):
  return (5 - p[0], p[1])

def flipxp(p):
  return (flipx(p[0]), flipx(p[1]))

def flipyp(p):
  return (flipy(p[0]), flipy(p[1]))

def noflip(p):
  return (p[1], p[2])

def remove_symmetry(seq):
  seen = set()
  for s in seq:
    q = noflip(s)
    if q not in seen:
      yield s
    seen.add(q)
    seen.add(flipxp(q))
    seen.add(flipyp(q))
    seen.add(flipyp(flipxp(q)))

def get_symmetry(n, i, size):
  if n == 0 and i == size / 2:
    return remove_symmetry
  else:
    return id_generator

def collect_word(n, word, sharp):
  sharp = "#" if sharp else ""
  for i in xrange(len(word) - 1):
    a, b = w = word[i: i + 2]
    transform = get_symmetry(n, i, len(word))
    for name, pa, pb in transform(flip(iter_torto())):
      yield "%sW%d_%d %s %s p%s:%s p%s:%s %spw%d_%s:%s pw%d_%s:%s" % (
        sharp, n, i, item_letter(n, i, pa), item_letter(n, i + 1, pb),
        pos(pa), a, pos(pb), b, diag(n, name, pa, pb),
        n, encode_pos(pa), encode(i), n, encode_pos(pb), encode(i + 1))

def histogram(words):
  global_hist = Counter()
  for word in words:
    local_hist = Counter()
    for item in word:
      local_hist[item] += 1
    for item, count in local_hist.iteritems():
      global_hist[item] = max(global_hist[item], count)
  return global_hist

def collect_letters(words):
  letters = histogram(words)
  for letter, count in letters.iteritems():
    for c in xrange(count):
      for j in xrange(6):
        for i in xrange(3):
          p = (j, i)
          yield "L%s%d p%s:%s l%s%d:%s" % (
              letter, c, pos(p), letter, letter, c, encode_pos(p))

def word_bigrams(words):
  for word in words:
    bigrams = [list(i) for i in zip(word, word[1:])]
    for bigram in bigrams:
      bigram.sort()
    yield ["".join(bigram) for bigram in bigrams]

def collect_bigrams(words):
  bigrams = histogram(word_bigrams(words))
  edges = flip(iter_torto())
  for bigram, count in bigrams.iteritems():
    for edge in flip(iter_torto()):
      yield "B%s%d p%s:%s p%s:%s b%s%d_0:%s b%s%d_1:%s" % (
          bigram, count, pos(edge[1]), bigram[0], pos(edge[2]), bigram[1],
          bigram, count, encode_pos(edge[1]), bigram, count, encode_pos(edge[2]))

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
  parser.add_argument("-s", "--sharp", action="store_true", help="Use the sharp heuristic")
  args = parser.parse_args()
  nwords = int(raw_input())
  words = [raw_input().strip() for _ in xrange(nwords)]
  words.sort(key=lambda w: len(w), reverse=True)
  options = []
  for n, word in enumerate(words):
    options.extend(collect_word(n, word, args.sharp))
  if args.sharp:
    options.extend(collect_letters(words))
    options.extend(collect_bigrams(words))
  primary = set()
  secondary = set()
  for option in options:
    for item in extract_primary(option):
      primary.add(item)
    for item in extract_secondary(option):
      secondary.add(item)
  print "%s | %s" % (" ".join(primary), " ".join(secondary))
  for option in options:
    print option

main()
