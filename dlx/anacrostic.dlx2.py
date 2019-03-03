import itertools
import sys
import string
import collections

def encode(n):
  return chr(n + ord("a"))

def get_triple(index, word):
  return tuple(word[k] for k in index)

def get_triples(index, words):
  ans = set()
  for word in words:
    ans.add(get_triple(index, word))
  return ans

def find_triples(triple, index, words):
  for word in words:
    if get_triple(index, word) == triple:
      yield word

def collect_src(n, words, factors):
  for i in xrange(n):
    triple = None
    for dindex, dstindex, sindex, srcindex, goal in factors:
      if i == sindex:
        triple = goal
        index = srcindex
    for nword, word in enumerate(words):
      if triple and get_triple(index, word) not in triple:
        continue
      option = ["S%d" % i, "nw%d" % nword]
      for j, w in enumerate(word):
        option.append("w%d:%s" % (i * 5 + j + 1, w))
      yield " ".join(option)
  
def collect_dst(dst, words, factors):
  for i, dstword in enumerate(dst):
    triple = None
    for dindex, dstindex, sindex, srcindex, goal in factors:
      if i == dindex:
        triple = goal
        index = dstindex
    for j, word in enumerate(words[len(dstword)]):
      if triple and get_triple(index, word) not in triple:
        continue
      option = ["D%d" % i]
      for j, w in zip(dstword, word):
        option.append("w%d:%s" % (j, w))
      yield " ".join(option)

def collect_trigrams(dst, words):
  for i, dstword in enumerate(dst):
    if len(dstword) < 4:
      continue
    dwords = words[len(dstword)]
    for j in xrange(len(dstword) / 3):
      trigrams = set()
      for word in dwords:
        trigrams.add(word[j*3:j*3+3])
      for trigram in trigrams:
        option = ["T%d%02d" % (j, i)]
        for k, w in enumerate(trigram):
          option.append("w%d:%s" % (dstword[j * 3 + k], w))
        yield " ".join(option)

def collect_letters(dst, words):
  for i, dstword in enumerate(dst):
    dwords = words[len(dstword)]
    for j in xrange(len(dstword)):
      letters = set()
      for word in dwords:
        letters.add(word[j])
      for letter in letters:
        option = ["L%s%02d" % (encode(j), i), "w%d:%s" % (dstword[j], letter)]
        yield " ".join(option)

def collect_initials(nsrc, words1000, words):
  for n in xrange(nsrc):
    if n >= 1:
      yield "F%d f%d:0" % (n, n)
    for size in xrange(2, len(words)):
      if n + size >= nsrc:
        continue      
      for word in words[size]:
        option = ["F%d" % n]
        for i, w in enumerate(word):
          option.append("f%d:%d" % (n + i, int(i == 0)))
          option.append("w%d:%s" % ((n + i) * 5 + 1, w))
        yield " ".join(option)

def collect_bigrams(dst, bigrams):
  for n in xrange(len(dst) - 1):
    k = (len(dst[n]), len(dst[n + 1]))
    for freq, w1, w2 in bigrams[k]:
      option = ["B%d" % n]
      for i, w in itertools.chain(zip(dst[n], w1), zip(dst[n + 1], w2)):
        option.append("w%d:%s" % (i, w))
      #yield "%s |%d" % (" ".join(option), freq)
      yield " ".join(option)
    yield "B%d" % n
  
def collect_primary(options):
  items = set()
  for option in options:
    for item in option.split():
      if item[0].isupper() or item[0] == "#":
        items.add(item)
  return items

def collect_secondary(options):
  items = set()
  for option in options:
    for item in option.split():
      if item[0].islower():
        items.add(item.split(":")[0])
  return items

def valid_word(word):
  return len(word) < 12 and all(w.islower() for w in word)

def print_factors(word, dstindex, srcindex, words, words1000):
  print "-- ", word
  totald, totals = 0, 0
  for g in goal:
    dwords = list(find_triples(g, dstindex, words[len(word)]))
    swords = list(find_triples(g, srcindex, words1000))
    totald += len(dwords)
    totals += len(swords)
  print len(words[len(word)]), totald, 1000, totals

def find_factors(dst, words1000, words):
  for dindex, word in enumerate(dst):
    hist = collections.Counter()
    for w in word:
      hist[(w - 1) / 5] += 1
    factors = sum(int(v >= 3) for v in hist.itervalues())
    if not factors:
      continue
    if factors > 1:
      print >> sys.stderr, "More than one factor per word is not supported yet"
      sys.exit(1)
    sindex = [k for k,v in hist.iteritems() if v == 3][0]
    dstindex = [i for i, v in enumerate(word) if (v - 1) / 5 == sindex]
    srcindex = [(word[s] - 1) % 5 for s in dstindex]
    dsttriples = get_triples(dstindex, words[len(word)])
    srctriples = get_triples(srcindex, words1000)
    goal = srctriples.intersection(dsttriples)
    yield (dindex, dstindex, sindex, srcindex, goal)

def eval_bigrams(wordset):
  bigrams = {}
  bigram_words = [set() for _ in xrange(12)]
  for line in open("words/bigrams/coca-bigrams.txt"):
    freq, w1, w2 = line.split()
    if valid_word(w1) and valid_word(w2):
      bigrams.setdefault((len(w1), len(w2)), []).append((int(freq), w1, w2))
      if w1 in wordset[len(w1)]:
        bigram_words[len(w1)].add(w1)
      if w2 in wordset[len(w2)]:
        bigram_words[len(w2)].add(w2)      

def main():
  nsrc, ndst = map(int, raw_input().split())
  dst = [map(int, raw_input().split()) for _ in xrange(ndst)]
  alldst = set(itertools.chain.from_iterable(dst))
  if len(alldst) != max(alldst):
    print >> sys.stderr, "Bad input"
    return
  words1000 = open("words/words5-from-sgb").read().split()[:1000]
  words = [[], []]
  limits = [85, 562, 1863, 3199, 4650, 5631, 5417, 4724, 3657, 2448]
  for i, limit in zip(range(2, 12), limits):
    words.append(open("words/words%d-from-OSPD4" % i).read().split()[:limit])
  wordset = [set(w) for w in words]
  factors = list(find_factors(dst, words1000, words))
  options = []
  options.extend(collect_src(nsrc, words1000, factors))
  options.extend(collect_dst(dst, words, factors))
  options.extend(collect_trigrams(dst, words))
  options.extend(collect_initials(nsrc, words1000, words))
  options.extend(collect_letters(dst, words))
  #options.extend(collect_bigrams(dst, bigrams))
  primary = collect_primary(options)
  secondary = collect_secondary(options)
  print "%s | %s" % (" ".join(primary), " ".join(secondary))
  for option in options:
    print option


main()
