import itertools
import sys
import string
import collections

def word_index(word, index):
  return tuple(word[i] for i in index)

def get_positions(dstposition):
  positions = {}
  for i, p in enumerate(dstposition):
    dstword, index = divmod(p - 1, 5)
    positions.setdefault(dstword, []).append((i, index))
  return positions

def get_wordmap(positions, srcwords, dstword, srccache={}):
  wordmap = {}
  for sw, indexes in positions.iteritems():
    srcindex = [i[1] for i in indexes]
    dstindex = [i[0] for i in indexes]
    dsttuple = word_index(dstword, dstindex)
    if dsttuple in srccache:
      wordmap[sw] = srccache[dsttuple].copy()
    else:
      words = set()
      for srcword in srcwords:
        if word_index(srcword, srcindex) == dsttuple:
          words.add(srcword)
      wordmap[sw] = words
      srccache[dsttuple] = words.copy()
  return wordmap

def build_graph(nsrc, dstpositions, srcwords, dstwords):
  srcmap = {i:{} for i in xrange(nsrc)}
  dstcandidates = {}
  dstmap = {}
  for j, dstposition in enumerate(dstpositions):
    print >> sys.stderr, "Processing src word ", j
    positions = get_positions(dstposition)
    dstmap[j] = positions.keys()
    dstcandidates[j] = {}
    for dstword in dstwords[len(dstposition)]:
      wordmap = get_wordmap(positions, srcwords, dstword)
      if all(wordmap[sw] for sw in positions.iterkeys()):
        dstcandidates[j][dstword] = wordmap
  return dstmap, dstcandidates

def print_len(name, graph):
  print name, [len(v) for v in graph.itervalues()]

def valid_word(word):
  return len(word) < 12 and all(w.islower() for w in word)

def build_src_graph(nsrc, dstgraph):
  keys = {}
  dstmap = {}
  for j, dst in dstgraph.iteritems():
    for dstword, srcdetails in dst.iteritems():
      for sw, srcwords in srcdetails.iteritems():
        keys.setdefault(sw, set()).add(j)
        dstmap.setdefault(sw, {})
        for srcword in srcwords:
          dstmap[sw].setdefault(srcword, {}).setdefault(j, set()).add(dstword)
  for sw, indexes in keys.iteritems():
    print >> sys.stderr, "Processing dst word ", sw
    remove = set()
    for srcword, dst in dstmap[sw].iteritems():
      if any(i not in dstmap[sw][srcword] for i in indexes):
        remove.add((sw, srcword))
    for sw, srcword in remove:
        dstmap[sw].pop(srcword)
  return keys, dstmap

def clean_graph(dstgraph, srcgraph, dstpositions):
  remove = set()
  change = False
  for dw, v in dstgraph.iteritems():
    for dstword, vv in v.iteritems():
      for sw, vvv in vv.iteritems():
        for srcword in vvv:
          if srcword not in srcgraph[sw]:
            remove.add((dw, dstword, sw, srcword))
            change = True
  for dw, dstword, sw, srcword in remove:
    dstgraph[dw][dstword][sw].remove(srcword)
  remove = set()
  for dw, dstwords in dstgraph.iteritems():
    for dstword, indexes in dstwords.iteritems():
      if any(not indexes[i] for i in dstpositions[dw]):
        remove.add((dw, dstword))
  for dw, dstword in remove:
    dstgraph[dw].pop(dstword)
  return change

def iterate(dstpositions, dstgraph, srcpositions, srcgraph):
  print
  print dstpositions
  print srcpositions
  print_len("dst", dstgraph)
  print_len("src", srcgraph)
  while True:
    print 
    if not clean_graph(dstgraph, srcgraph, dstpositions):
      break
    print_len("dst", dstgraph)
    if not clean_graph(srcgraph, dstgraph, srcpositions):
      break
    print_len("src", srcgraph)

def main():
  nsrc, ndst = map(int, raw_input().split())
  dstpositions = [map(int, raw_input().split()) for _ in xrange(ndst)]
  alldst = set(itertools.chain.from_iterable(dstpositions))
  if len(alldst) != max(alldst):
    print >> sys.stderr, "Bad input"
    return
  srcwords = open("words/words5-from-sgb").read().split()[:1000]
  dstwords = [[], []]
  for i in xrange(2, 12):
    dstwords.append(open("words/words%d-from-OSPD4" % i).read().split())
  dstmap, dstgraph = build_graph(nsrc, dstpositions, srcwords, dstwords)
  print [len(v) for k,v in dstgraph.iteritems()]
  srcmap, srcgraph = build_src_graph(nsrc, dstgraph)
  print [len(v) for k,v in srcgraph.iteritems()]
  iterate(dstmap, dstgraph, srcmap, srcgraph)
  return
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
