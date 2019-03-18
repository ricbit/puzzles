import dlx
import itertools
import sys

class Nurikabe:
  def __init__(self, h, w, groups):
    self.h = h
    self.w = w
    self.groups = groups
    self.seeds = {(gj, gi): n for n, (gj, gi, _) in enumerate(groups)}

  def iter_group(self, gj, gi, gsize):
    for j, i in itertools.product(range(-gsize, gsize + 1), repeat=2):
      if 0 <= gj + j < self.h and 0 <= gi + i < self.w:
        dist = abs(j) + abs(i)
        if dist < gsize:
          yield gj + j, gi + i, dist 

  def encodepos(self, j, i):
    ej = "%02d" % j
    ei = "%02d" % i
    return "".join([ej, ei])

  def encodegroup(self, gn):
    return chr(ord('a') + gn)

  def encodetree(self, depth):
    return chr(ord('a') + depth)

  def decodegroup(self, gs):
    return ord(gs) - ord('a')

  def iter_neigh(self, pj, pi, gj, gi, gsize):
    disp = [x for x in itertools.product(range(-1, 2), repeat=2) if x.count(0) == 1]
    for jj, ii in disp:
      nj = pj + jj
      ni = pi + ii
      if abs(nj - gj) + abs(ni - gi) <= gsize:
        if 0 <= nj < self.h and 0 <= ni < self.w:
          yield nj, ni

  def collect_groups(self):
    for gn, (gj, gi, gsize) in enumerate(self.groups):
      for pj, pi, dist in self.iter_group(gj, gi, gsize):
        if self.seeds.get((pj, pi), gn) != gn:
          continue
        eg = self.encodegroup(gn)
        ep = self.encodepos(pj, pi)
        baseoption = ["G%s" % eg]
        baseoption.append("p%s:1" % ep)
        baseoption.append("g%s:%s" % (ep, eg))
        if pj == gj and pi == gi:
          option = baseoption.copy()
          option.append("S%s" % eg)
          for g in range(len(self.groups)):
            tree = self.encodetree(0) if g == gn else "0"
            option.append("t%s%s:%s" % (self.encodegroup(g), ep, tree))
          yield " ".join(option)
        else:
          for nj, ni in self.iter_neigh(pj, pi, gj, gi, gsize):
            en = self.encodepos(nj, ni)
            for d in range(1, gsize):
              option = baseoption.copy()
              for g in range(len(self.groups)):
                tree = self.encodetree(d) if g == gn else "0"
                option.append("t%s%s:%s" % (self.encodegroup(g), ep, tree))
              option.append("t%s%s:%s" % (eg, en, self.encodetree(d - 1)))
              option.append("u%s%s" % (eg, ep))
              yield " ".join(option)

  def collect_empty(self):
    for j, i in itertools.product(range(self.h), range(self.w)):
      if (j, i) not in self.seeds:
        option = ["E"]
        pos = self.encodepos(j, i)
        option.append("g%s:0" % pos)
        for gn in range(len(self.groups)):
          option.append("t%s%s:0" % (self.encodegroup(gn), pos))
        yield " ".join(option)
  
  def empty_size(self):
    empty = self.w * self.h
    for gj, gi, gsize in self.groups:
      empty -= gsize
    return empty

  def collect_primary(self, options):
    items = set()
    empty = self.empty_size()
    for option in options:
      for item in option.split():
        if item.startswith("G"):
          gn = self.decodegroup(item[1:])
          items.add("%d|%s" % (self.groups[gn][2], item))
        elif item.startswith("E"):
          items.add("%d|%s" % (empty, item))
        elif item[0].isupper():
          items.add(item)
    return items

def main():
  h, w = map(int, input().split())
  ngroups = int(input().strip())
  groups = [tuple(map(int, input().split())) for _ in range(ngroups)]
  solver = Nurikabe(h, w, groups)
  options = []
  options.extend(solver.collect_groups())
  options.extend(solver.collect_empty())
  print("\n".join(dlx.build_dlx(options, primary=solver.collect_primary)))

main()
