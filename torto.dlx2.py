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
      yield "w%d%s %s %s p%s:%s p%s:%s %s pw%d_%s:%s pw%d_%s:%s" % (
        n, w, item_letter(n, i, pa), item_letter(n, i + 1, pb),
        pos(pa), a, pos(pb), b, diag(n, name, pa, pb),
        n, encode_pos(pa), encode(i), n, encode_pos(pb), encode(i + 1))

def extract_primary(option):
  for item in option.split():
    if ":" not in item and item[0] in "w":
      yield item
      
def extract_secondary(option):
  for item in option.split():
    if ":" in item or item[0] not in "wLB|":
      yield item.split(":")[0]

def letter_primary(letters):
  rem_letters = 6 * 3 - sum(letters.itervalues())  
  for letter, count in letters.iteritems():
    for cost in xrange(count + rem_letters):
      yield "L%s%d" % (letter, cost)

def bigram_primary(bigrams):
  rem_bigrams = 2 * 6 + 3 * 5 + 2 * 2 * 5 - sum(bigrams.itervalues())
  for bigram, count in bigrams.iteritems():
    yield "B%s" % (bigram)

def collect_letters(words):
  all_letters = {}
  for word in words:
    letters = Counter()
    for letter in word:
      letters[letter] += 1
    for letter, count in letters.iteritems():
      all_letters[letter] = max(all_letters.get(letter, 0), count)
  return all_letters

def options_letters(letters):
  rem_letters = 6 * 3 - sum(letters.itervalues())  
  for j in xrange(6):
    for i in xrange(3):
      for letter, count in letters.iteritems():
        for cost in xrange(rem_letters + count):
          p = pos((j, i))
          yield "L%s%d p%s:%s |%d" % (letter, cost, p, letter, cost)

def unique_bigram(bigram):
  b = list(bigram)
  b.sort()
  return "".join(b)

def collect_bigrams(words):
  all_bigrams = {}
  for word in words:
    bigrams = Counter()
    for i in xrange(len(word) - 1):
      bigram = unique_bigram(word[i:i + 2])
      bigrams[bigram] += 1
    for bigram, count in bigrams.iteritems():
      all_bigrams[bigram] = max(all_bigrams.get(bigram, 0), count)
  return all_bigrams

def option_bigram(bigram, pa, a, pb, b):
  return "B%s p%s:%s p%s:%s |%d" % (bigram, pos(pa), a, pos(pb), b, random.randint(1,80))

def flip_bigram(bigrams):
  for bigram in bigrams:
    yield bigram
    if bigram[0] != bigram[1]:
      yield bigram[::-1]

def options_bigrams(bigrams):
  for bigram in flip_bigram(bigrams):
    for name, pa, pb in flip(iter_torto()):
      yield option_bigram(unique_bigram(bigram), pa, bigram[0], pb, bigram[1])

nwords = int(raw_input())
words = [raw_input().strip() for _ in xrange(nwords)]
words.sort(key=lambda w: len(w), reverse=True)
options = []
letters = collect_letters(words)
for n, word in enumerate(words):
  options.extend(collect_word(n, word))
bigrams = collect_bigrams(words)
#options.extend(options_bigrams(bigrams))
#options.extend(options_letters(letters))
primary = set()
secondary = set()
for option in options:
  for item in extract_primary(option):
    primary.add(item)
  #for item in letter_primary(letters):
  #  primary.add(item)
  #for item in bigram_primary(bigrams):
  #  primary.add(item)
  for item in extract_secondary(option):
    secondary.add(item)
print "%s | %s" % (" ".join(primary), " ".join(secondary))
for option in options:
  print option
