import dlx
import itertools
import sys

class Nurikabe:
  def __init__(self, h, w, groups):
    self.h = h
    self.w = w
    self.groups = groups

  def iter_group(self, gj, gi, gsize):
    for j, i in itertools.product(range(-gsize, gsize + 1), repeat=2):
      if 0 <= gj + j < self.h and 0 <= gi + i < self.w:
        dist = abs(j) + abs(i)
        if dist <= gsize:
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
        yield nj, ni

  def collect_groups(self):
    for gn, (gj, gi, gsize) in enumerate(self.groups):
      for pj, pi, dist in self.iter_group(gj, gi, gsize):
        eg = self.encodegroup(gn)
        ep = self.encodepos(pj, pi)
        baseoption = ["G%s" % eg]
        baseoption.append("p%s:1" % ep)
        baseoption.append("g%s:%s" % (ep, eg))
        if pj == gj and pi == gi:
          option = baseoption.copy()
          option.append("t%s%s:%s" % (eg, ep, self.encodetree(0)))
          yield " ".join(option)
        else:
          for nj, ni in self.iter_neigh(pj, pi, gj, gi, gsize):
            en = self.encodepos(nj, ni)
            for d in range(2, gsize):
              option = baseoption.copy()
              option.append("t%s%s:%s" % (eg, ep, self.encodetree(d)))
              option.append("t%s%s:%s" % (eg, en, self.encodetree(d - 1)))
              yield " ".join(option)

  def collect_empty(self):
    for j, i in itertools.product(range(self.h), range(self.w)):
      option = ["E"]
      option.append("g%s:0" % self.encodepos(j, i))
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
