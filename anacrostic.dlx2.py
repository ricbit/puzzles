import itertools
import sys
import string
import collections
import dlx

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
      for j, srcword in enumerate(srcwords):
        if word_index(srcword, srcindex) == dsttuple:
          words.add(j)
      wordmap[sw] = words
      srccache[dsttuple] = words.copy()
  return wordmap

def build_graph(nsrc, dstpositions, srcwords, dstwords):
  srcmap = {i:{} for i in xrange(nsrc)}
  dstcandidates = {}
  dstmap = {}
  print >> sys.stderr, "Building dst graph"
  for j, dstposition in enumerate(dstpositions):
    positions = get_positions(dstposition)
    dstmap[j] = positions.keys()
    dstcandidates[j] = {}
    for i, dstword in enumerate(dstwords[len(dstposition)]):
      wordmap = get_wordmap(positions, srcwords, dstword)
      if all(wordmap[sw] for sw in positions.iterkeys()):
        dstcandidates[j][i] = wordmap
  return dstmap, dstcandidates

def print_len(name, graph):
  print >> sys.stderr, name, [len(v) for v in graph.itervalues()]

def count_len(graph):
  return sum([len(v) for v in graph.itervalues()])

def count_words(srcgraph, dstgraph):
  return count_len(srcgraph) + count_len(dstgraph)

def valid_word(word):
  return len(word) < 12 and all(w.islower() for w in word)

def build_src_graph(nsrc, dstgraph):
  keys = {}
  dstmap = {}
  print >> sys.stderr, "Building src graph"
  for j, dst in dstgraph.iteritems():
    for dstword, srcdetails in dst.iteritems():
      for sw, srcwords in srcdetails.iteritems():
        keys.setdefault(sw, set()).add(j)
        dstmap.setdefault(sw, {})
        for srcword in srcwords:
          dstmap[sw].setdefault(srcword, {}).setdefault(j, set()).add(dstword)
  for sw, indexes in keys.iteritems():
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
  print >> sys.stderr, "Started with %d words" % count_words(srcgraph, dstgraph)
  while True:
    if not clean_graph(dstgraph, srcgraph, dstpositions):
      break
    print >> sys.stderr, "Reduced to %d words" % count_words(srcgraph, dstgraph)
    if not clean_graph(srcgraph, dstgraph, srcpositions):
      break
    print >> sys.stderr, "Reduced to %d words" % count_words(srcgraph, dstgraph)

def collect_pool(dstgraph, dstpositions):
  pool = [set() for _ in xrange(len(dstgraph))]
  for i, word in enumerate(dstpositions):
    pool[len(word)].update(dstgraph[i].keys())
  return pool

def collect_src(srcgraph, srcwords, srcdegree):
  print >> sys.stderr, "Creating options for src words"
  for sw, words in srcgraph.iteritems():
    for srcword, items in words.iteritems():
      option = ["S%d" % sw, "su%d" % srcword, "f%d:%d" % (sw, ord(srcwords[srcword][0]))]
      option.append("s%d:%d" % (sw, srcword))
      yield " ".join(option)
      for pos, dstwords in items.iteritems():
        for dstword in dstwords:
          option = ["S%d_%d" % (sw, pos)]
          option.append("s%d:%d" % (sw, srcword))
          option.append("d%d:%d" % (pos, dstword))
          yield " ".join(option)

def collect_dst(dstgraph, dstdegree):
  print >> sys.stderr, "Creating options for dst words"
  for dw, words in dstgraph.iteritems():
    sharp = "#" if dstdegree[dw] == 1 and len(words) < 20 else ""
    for dstword, items in words.iteritems():
      option = ["%sD%d" % (sharp, dw)]
      option.append("d%d:%d" % (dw, dstword))
      yield " ".join(option)
      for pos, srcwords in items.iteritems():
        for srcword in srcwords:
          option = ["D%d_%d" % (dw, pos)]
          option.append("d%d:%d" % (dw, dstword))
          option.append("s%d:%d" % (pos, srcword))
          yield " ".join(option)

def collect_names(nsrc, nnames, allnames):
  print >> sys.stderr, "Creating options for names"
  minlen = min(allnames)
  maxlen = max(allnames)
  for i in xrange(nnames):
    print >> sys.stderr, "Creating options for name ", i
    start = minlen * i
    end = nsrc - minlen * (nnames - i - 1)
    for pos in xrange(start, end):
      for namelen, names in allnames.iteritems():
        if pos + namelen > end:
          continue
        if i == 0 and pos != 0:
          continue
        if i + 1 == nnames and pos + namelen != nsrc:
          continue
        for name in names:
          option = ["F%d" % i, "fb%d:%d" % (i, pos), "fb%d:%d" % (i + 1, pos + namelen)]
          for j, c in enumerate(name):
            option.append("f%d:%d" % (pos + j, ord(c)))
          yield " ".join(option)

def add_links(graph, name, links):
  for src, items in graph.iteritems():
    for word, dstmap in items.iteritems():
      for dst in dstmap:
        links.add(name % (src, dst))

def write_dot(srcgraph, dstgraph):
  f = open("anacrostic.dot", "wt")
  f.write("graph anacrostic {\n");
  links = set()
  add_links(srcgraph, "S%d -- D%d;\n", links)
  for link in links:
    f.write(link)
  f.write("}\n")
  f.close()

def find_degree(graph):
  edges = {}
  for src, items in graph.iteritems():
    for word, dstmap in items.iteritems():
      for dst in dstmap:
        edges.setdefault(src, set()).add(dst)
  return {k:len(v) for k,v in edges.iteritems()}

def print_degree(degrees):
  for k,v in sorted(degrees.iteritems(), key=lambda (k,v):v, reverse=True):
    print >> sys.stderr, k,v

def find_fanout(srcgraph, srcmap):
  fanout = {}
  for k, v in srcmap.iteritems():
    fanout[k] = set()
    for i in xrange(2, 1 + len(v)):
      fanout[k].update(tuple(x) for x in itertools.combinations(sorted(v), i))
  cycles = {}
  for a, b in itertools.combinations(srcmap.keys(), 2):
    inter = fanout[a].intersection(fanout[b])
    if inter:
      maxlen = max(len(x) for x in inter)
      cycles[(a,b)] = [x for x in inter if len(x) == maxlen]
  return cycles

def invert(srcfanout):
  for (a,b), fanouts in srcfanout.iteritems():
    yield (a,b), fanouts
    yield (b,a), fanouts

def discard_pairs(srcgraph, srcfanout):
  print "discarding"
  for (a, b), fanouts in invert(srcfanout):
    for fanout in fanouts:
      for worda, itemsa in srcgraph[a].iteritems():
        found = []
        count = 0
        for wordb, itemsb in srcgraph[b].iteritems():
          aset = [itemsa[k] for k in fanout]
          bset = [itemsb[k] for k in fanout]
          inter = [sa.intersection(sb) for sa,sb in zip(aset, bset)]
          if any(len(x) == 0 for x in inter):
            # print worda, wordb, inter
            count += 1
          else:
            found.append(wordb)
        print a, worda, count, len(found)

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
    words = open("words/words%d-from-OSPD4" % i).read().split()
    dstwords.append(words[:len(words) * 4 / 10])
  names = {}
  for line in open("words/names.txt"):
    name = line.strip()
    if len(name) > 9:
      continue
    if len(name) in [2,3,4,5]:
      continue
    names.setdefault(len(name), []).append(name)
  dstmap, dstgraph = build_graph(nsrc, dstpositions, srcwords, dstwords)
  srcmap, srcgraph = build_src_graph(nsrc, dstgraph)
  srcdegree = find_degree(srcgraph)
  dstdegree = find_degree(dstgraph)
  srcfanout = find_fanout(srcgraph, srcmap)
  dstfanout = find_fanout(dstgraph, dstmap)
  discard_pairs(srcgraph, srcfanout)
  return 
  #print_degree(srcdegree)
  #print_degree(dstdegree)
  iterate(dstmap, dstgraph, srcmap, srcgraph)
  pool = collect_pool(dstgraph, dstpositions)
  options = []
  options.extend(collect_src(srcgraph, srcwords, srcdegree))
  options.extend(collect_dst(dstgraph, dstdegree))
  #options.extend(collect_names(nsrc, 6, names))
  print >> sys.stderr, "Saving to disk"
  print "\n".join(dlx.build_dlx(options))


main()
