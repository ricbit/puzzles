from collections import Counter
import itertools
import random

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
  limit = 10 + ord('z') - ord('a')
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
    seen.add(flipxp(q))

def get_symmetry(n, i):
  if n == 0 and i == 0:
    return remove_symmetry
  else:
    return id_generator

def collect_word(n, word):
  for i in xrange(len(word) - 1):
    a, b = w = word[i: i + 2]
    transform = get_symmetry(n, i)
    for name, pa, pb in transform(flip(iter_torto())):
      yield "W%d_%d %s %s p%s:%s p%s:%s %s pw%d_%s:%s pw%d_%s:%s" % (
        n, i, item_letter(n, i, pa), item_letter(n, i + 1, pb),
        pos(pa), a, pos(pb), b, diag(n, name, pa, pb),
        n, encode_pos(pa), encode(i), n, encode_pos(pb), encode(i + 1))

def extract_primary(option):
  for item in option.split():
    if ":" not in item and item[0].isupper():
      yield item
      
def extract_secondary(option):
  for item in option.split():
    if ":" in item or item[0].islower():
      yield item.split(":")[0]

nwords = int(raw_input())
words = [raw_input().strip() for _ in xrange(nwords)]
words.sort(key=lambda w: len(w), reverse=True)
options = []
for n, word in enumerate(words):
  options.extend(collect_word(n, word))
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
