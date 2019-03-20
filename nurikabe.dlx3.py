import dlx
import itertools
import sys
import collections

class Nurikabe:
  def __init__(self, h, w, groups):
    self.h = h
    self.w = w
    self.groups = groups
    self.candidates = collections.defaultdict(lambda: set([-1]))
    self.seeds = {(gj, gi): n for n, (gj, gi, _) in enumerate(groups)}
    self.empty_size = self.w * self.h - sum(size for _, _, size in self.groups)
    self.forbidden = [self.build_forbidden(g) for g in range(len(groups))]
    self.dmap = {i: self.build_min_distance_map(i) for i in range(len(groups))}

  def iter_group(self, gj, gi, gsize):
    for j, i in itertools.product(range(-gsize, gsize + 1), repeat=2):
      if 0 <= gj + j < self.h and 0 <= gi + i < self.w:
        dist = abs(j) + abs(i)
        if dist < gsize:
          yield gj + j, gi + i, dist 

  def iter_neigh(self, pj, pi, gj, gi, gsize):
    disp = [x for x in itertools.product(range(-1, 2), repeat=2) if x.count(0) == 1]
    for jj, ii in disp:
      nj = pj + jj
      ni = pi + ii
      if abs(nj - gj) + abs(ni - gi) <= gsize:
        if 0 <= nj < self.h and 0 <= ni < self.w:
          yield nj, ni

  def build_matrix(self, h, w, default):
    return [[default] * w for _ in range(h)]

  def build_forbidden(self, group):
    forbidden = self.build_matrix(self.h, self.w, False)
    for n, (gj, gi, gsize) in enumerate(self.groups):
      if n != group:
        forbidden[gj][gi] = True
        for nj, ni in self.iter_neigh(gj, gi, gj, gi, gsize):
          forbidden[nj][ni] = True
    return forbidden          

  def build_min_distance_map(self, group):
    gj, gi, gsize = self.groups[group]
    value = self.build_matrix(self.h, self.w, -1)
    stack = [(gj, gi, 0)]
    value[gj][gi] = 0
    while stack:
      pj, pi, pv = stack.pop(0)
      for nj, ni in self.iter_neigh(pj, pi, gj, gi, gsize):
        if value[nj][ni] < 0 and not self.forbidden[group][nj][ni]:
          value[nj][ni] = pv + 1
          if pv + 1 < gsize:
            stack.append((nj, ni, pv + 1))
    print(group, file=sys.stderr)
    print("\n".join("".join(self.encodegroup(c) for c in line) for line in value), file=sys.stderr)
    return value

  def encodepos(self, j, i):
    ej = "%02d" % j
    ei = "%02d" % i
    return "".join([ej, ei])

  def encodegroup(self, gn):
    if gn >= 0:
      return chr(ord('a') + gn)
    else:
      return "0"

  def encodetree(self, depth):
    return chr(ord('a') + depth)

  def decodegroup(self, gs):
    return ord(gs) - ord('a')

  def collect_groups(self):
    for gn, (gj, gi, gsize) in enumerate(self.groups):
      for pj, pi, dist in self.iter_group(gj, gi, gsize):
        if self.seeds.get((pj, pi), gn) != gn:
          continue
        eg = self.encodegroup(gn)
        ep = self.encodepos(pj, pi)
        self.candidates[(pj, pi)].add(gn)
        baseoption = ["G%s" % eg]
        baseoption.append("P%s" % ep)
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
          mindist = abs(pj - gj) + abs(pi - gi)
          for nj, ni in self.iter_neigh(pj, pi, gj, gi, gsize):
            en = self.encodepos(nj, ni)
            for d in range(mindist, gsize):
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
        option.append("p%s:0" % pos)
        option.append("P%s" % pos)
        for gn in range(len(self.groups)):
          option.append("t%s%s:0" % (self.encodegroup(gn), pos))
        yield " ".join(option)
 
  def collect_squares(self):
    for j, i in dlx.iter_grid(self.h - 1, self.w - 1):
      base = ["S%s" % self.encodepos(j, i)]
      for bits in range(1, 16):
        option = base.copy()
        for n, (jj, ii) in enumerate(itertools.product([0, 1], repeat=2)):
          bit = int((bits & (1 << n)) > 0)
          option.append("p%s:%d" % (self.encodepos(j + jj, i + ii), bit))
        yield " ".join(option)

  def collect_pairs(self):
    for j, i in dlx.iter_grid(self.h, self.w - 1):
      ep = self.encodepos(j, i)
      base = ["H%s" % ep]
      c = [self.candidates[(j, i + k)] for k in range(2)]
      for g1, g2 in itertools.product(*c):
        if g1 == -1 or g2 == -1 or g1 == g2:
          option = base.copy()
          option.append("g%s:%s" % (ep, self.encodegroup(g1)))
          option.append("g%s:%s" % (self.encodepos(j, i + 1), self.encodegroup(g2)))
          option.append("p%s:%s" % (ep, int(g1 >= 0)))
          option.append("p%s:%s" % (self.encodepos(j, i + 1), int(g2 >= 0)))
          yield " ".join(option)
    for j, i in dlx.iter_grid(self.h - 1, self.w):
      ep = self.encodepos(j, i)
      base = ["V%s" % ep]
      c = [self.candidates[(j + k, i)] for k in range(2)]
      for g1, g2 in itertools.product(*c):
        if g1 == -1 or g2 == -1 or g1 == g2:
          option = base.copy()
          option.append("g%s:%s" % (ep, self.encodegroup(g1)))
          option.append("g%s:%s" % (self.encodepos(j + 1, i), self.encodegroup(g2)))
          option.append("p%s:%s" % (ep, int(g1 >= 0)))
          option.append("p%s:%s" % (self.encodepos(j + 1, i), int(g2 >= 0)))
          yield " ".join(option)

  def collect_primary(self, options):
    items = set()
    empty = self.empty_size
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
  options.extend(solver.collect_squares())
  options.extend(solver.collect_pairs())
  print("\n".join(dlx.build_dlx(options, primary=solver.collect_primary)))

main()
