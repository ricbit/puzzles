from collections import Counter

def collect_words(nb, words, author, quote):
  for nnw, word in enumerate(words):
    for na, la in enumerate(author):
      for nw, lw in enumerate(word): 
        if la != lw:
          continue
        hist = Counter(word)
        hist[lw] -= 1
        if any(v != 1 and k != lw for k, v in hist.iteritems()):
          continue
        if hist[lw] > 1:
          continue
        option = ["W%d_%d" % (nb, na)]
        for nw1, lw1 in enumerate(word):
          if nw == nw1:
            option.append("a%d_%d:%d" % (nb, na, nw))
          elif quote[lw1] > 0:
            option.append("L%s" % lw1)
        size = 4
        prefix = "%%0%dd" % size
        for i in xrange(size):
          option.append("w%d%d%d:%s" % (nb, na, i, (prefix % nnw)[i]))
        yield " ".join(option)

def collect_histogram(quote):
  hist = Counter()
  for q in quote:
    hist[q] += 1
  return hist

def collect_primary(options, quote):
  items = set()
  for option in options:
    for item in option.split():
      if item[0].isupper() or item[0] == "#":
        items.add(item)
  for item in items:
    if item[0] == "L" and quote[item[1]] > 1:
      yield "%d|%s" % (quote[item[1]], item)
    else:
      yield item

def collect_secondary(options):
  items = set()
  for option in options:
    for item in option.split():
      if item[0].islower():
        items.add(item.split(":")[0])
  for item in items:
    yield item

def collect_author(wordsizes, authors):
  for na, ((n, size), author) in enumerate(zip(wordsizes, authors)):
    for s in xrange(size):
      option = ["A%d" % na]
      for nna, la in enumerate(author):
        option.append("a%d_%d:%d" % (na, nna, s))
      yield " ".join(option)

def main():
  nwordsets = int(raw_input())
  wordsizes = [map(int, raw_input().split()) for _ in xrange(nwordsets)]
  authors = raw_input().split()
  quote = collect_histogram("".join(raw_input().split()))
  limit = 2000
  options = []
  for nb, ((n, size), author) in enumerate(zip(wordsizes, authors)):
    words = open("words/words%d-from-OSPD4" % size).read().split()[:limit]
    options.extend(collect_words(nb, words, author, quote))
  options.extend(collect_author(wordsizes, authors))
  primary = collect_primary(options, quote)
  secondary = collect_secondary(options)
  print "%s | %s" % (" ".join(primary), " ".join(secondary))
  for option in options:
    print option

main()
