import dlx
import itertools
import sys
import collections
import math

def index(seq):
  for i, _ in enumerate(seq):
    yield i


def build_matrix(h, w, default):
  return [[default] * w for _ in range(h)]

class NurikabeIterators:
  def __init__(self, h, w, groups):
    self.h = h
    self.w = w
    self.groups = groups
    self.seeds = {(gj, gi): n for n, (gj, gi, _) in enumerate(groups)}

  def iter_group(self, gj, gi, gsize):
    for j, i in itertools.product(range(1 - gsize, gsize), repeat=2):
      if 0 <= gj + j < self.h and 0 <= gi + i < self.w:
        dist = abs(j) + abs(i)
        if dist < gsize:
          yield gj + j, gi + i, dist

  def iter_neigh(self, pj, pi, group=None):
    disp = [x for x in itertools.product(range(-1, 2), repeat=2) if x.count(0) == 1]
    for jj, ii in disp:
      nj = pj + jj
      ni = pi + ii
      if 0 <= nj < self.h and 0 <= ni < self.w:
        if group:
          gj, gi, gsize = self.groups[group]
          if abs(nj - gj) + abs(ni - gi) < gsize:
            yield nj, ni
        else:
          yield nj, ni

  def inside(self, j, i):
    return 0 <= j < self.h and 0 <= i < self.w

  def forced_fill(self, j, i):
    return (j, i) in self.seeds


class NurikabeBuilder(NurikabeIterators):
  def __init__(self, h, w, groups):
    NurikabeIterators.__init__(self, h, w, groups)
    self.forbidden = [self.build_forbidden(g) for g in index(self.groups)]
    self.minmax = self.build_minmax()
    self.treesize = [self.build_treesize(i) for i in index(self.groups)]
    self.candidates = self.build_candidates()

  def build_forbidden(self, group):
    forbidden = build_matrix(self.h, self.w, False)
    for n, (gj, gi, gsize) in enumerate(self.groups):
      if n != group:
        forbidden[gj][gi] = True
        for nj, ni in self.iter_neigh(gj, gi, n):
          forbidden[nj][ni] = True
    return forbidden

  def build_min_distance_map(self, group):
    gj, gi, gsize = self.groups[group]
    value = build_matrix(self.h, self.w, -1)
    stack = [(gj, gi, 0)]
    value[gj][gi] = 0
    while stack:
      pj, pi, pv = stack.pop(0)
      for nj, ni in self.iter_neigh(pj, pi, group):
        if value[nj][ni] < 0 and not self.forbidden[group][nj][ni]:
          value[nj][ni] = pv + 1
          if pv + 1 < gsize - 1:
            stack.append((nj, ni, pv + 1))
    return value

  def build_max_distance_map(self, group):
    gj, gi, gsize = self.groups[group]
    value = build_matrix(self.h, self.w, gsize)
    prevstack = [(gj, gi, 0)]
    nextstack = []
    value[gj][gi] = 0
    while prevstack:
      for pj, pi, pv in prevstack:
        for nj, ni in self.iter_neigh(pj, pi, group):
          if nj == gj and ni == gi:
            continue
          if value[nj][ni] >= 2:
            value[nj][ni] = pv + 1
          if not self.forbidden[group][nj][ni] and pv + 1 < gsize - 1:
            nextstack.append((nj, ni, pv + 1))
      prevstack = nextstack
      nextstack = []
    return value

  def build_minmax(self):
    minmap = [self.build_min_distance_map(i) for i in index(self.groups)]
    maxmap = [self.build_max_distance_map(i) for i in index(self.groups)]
    minmax = [{} for _ in self.groups]
    for g, (gmin, gmax) in enumerate(zip(minmap, maxmap)):
      for j, i in dlx.iter_grid(self.h, self.w):
        if gmin[j][i] >= 0:
          minmax[g][(j, i)] = (gmin[j][i], gmax[j][i])
    return minmax

  def build_candidates(self):
    candidates = collections.defaultdict(lambda: set())
    for pos in dlx.iter_grid(self.h, self.w):
      if not self.forced_fill(*pos):
        candidates[pos].add(-1)
    for gn, minmax in enumerate(self.minmax):
      for pos in minmax:
        candidates[pos].add(gn)
    return candidates

  def build_treesize(self, gn):
    gj, gi, gsize = self.groups[gn]
    size = collections.Counter()
    used = gsize
    for j, i in dlx.iter_grid(self.h, self.w):
      if (j, i) in self.minmax[gn]:
        gmin, gmax = self.minmax[gn][(j, i)]
        for d in range(gmin, gmax + 1):
          size[d] += 1
    treesize = []
    for d in range(0, gsize):
      if used > 0:
        treesize.append((1, size[d]))
        used -= size[d]
      else:
        treesize.append((0, size[d]))
    return treesize


class Nurikabe(NurikabeIterators):
  def __init__(self, h, w, groups):
    NurikabeIterators.__init__(self, h, w, groups)
    self.empty_size = self.w * self.h - sum(size for _, _, size in self.groups)
    builder = NurikabeBuilder(h, w, groups)
    self.minmax = builder.minmax
    self.treesize = builder.treesize
    self.candidates = builder.candidates
    self.print_minmax()

  def print_minmax(self):
    for g, minmax in enumerate(self.minmax):
      print(g, self.encodegroup(g), self.groups[g], self.treesize[g], file=sys.stderr)
      print("%s\n\n" % self.encode_matrix(minmax, " ", lambda x:
        self.encodetree(x[1]) + self.encodetree(x[0])), file=sys.stderr)

  def forced_empty(self, j, i):
    return all((j, i) not in minmax for minmax in self.minmax)

  def encode_matrix(self, mat, spacer, encoder):
    ans = []
    for j in range(self.w):
      ans.append(spacer.join(encoder(mat.get((i, j), (-1,-1))) for i in range(self.h)))
    return "\n".join(ans)

  def encodepos(self, j, i):
    ej = "%02d" % j
    ei = "%02d" % i
    return "".join([ej, ei])

  def encodegroup(self, gn):
    if gn >= 0:
      return chr(ord('A') + gn)
    else:
      return "0"

  def encodetree(self, depth):
    return chr(ord('a') + depth)

  def decodegroup(self, gs):
    return ord(gs) - ord('A')

  def append_tree(self, option, gn, pj, pi, d):
    ep = self.encodepos(pj, pi)
    for g, (gj, gi, gsize) in enumerate(self.groups):
      if (pj, pi) in self.minmax[g]:
        tree = self.encodetree(d) if g == gn else "0"
        eg = self.encodegroup(g)
        option.append("t%s%s:%s" % (eg, ep, tree))
        if g != gn:
          for nj, ni in self.iter_neigh(pj, pi, g):
            if (nj, ni) in self.minmax[g]:
              option.append("t%s%s:0" % (eg, self.encodepos(nj, ni)))

  def collect_seed(self, baseoption, gn, pj, pi):
    eg = self.encodegroup(gn)
    option = baseoption.copy()
    option.append("T%s%d" % (eg, 0))
    self.append_tree(option, gn, pj, pi, 0)
    yield " ".join(option)

  def collect_tail(self, baseoption, gn, pj, pi):
    eg = self.encodegroup(gn)
    ep = self.encodepos(pj, pi)
    gj, gi, gsize = self.groups[gn]
    mindist, maxdist = self.minmax[gn][(pj, pi)]
    for nj, ni in self.iter_neigh(pj, pi, gn):
      if (nj, ni) not in self.minmax[gn]:
        continue
      nmin, nmax = self.minmax[gn][(nj, ni)]
      en = self.encodepos(nj, ni)
      for d in range(mindist, maxdist + 1):
        if nmin <= d - 1 <= nmax:
          option = baseoption.copy()
          option.append("T%s%d" % (eg, d))
          self.append_tree(option, gn, pj, pi, d)
          option.append("t%s%s:%s" % (eg, en, self.encodetree(d - 1)))
          for oj, oi in self.iter_neigh(pj, pi, gn):
            if (nj, ni) != (oj, oi) and (oj, oi) in self.minmax[gn]:
              omin, omax = self.minmax[gn][(oj, oi)]
              on = self.encodepos(oj, oi)
              for d2 in range(omin, omax + 1):
                if d2 < d - 1 or d2 > d + 1:
                  option.append("u%s%s%s:0" % (eg, on, self.encodetree(d2)))
          option.append("u%s%s" % (eg, ep))
          option.append("u%s%s%s:1" % (eg, ep, self.encodetree(d)))
          yield " ".join(option)

  def collect_groups(self):
    for gn, (gj, gi, gsize) in enumerate(self.groups):
      for pj, pi in self.minmax[gn]:
        eg = self.encodegroup(gn)
        ep = self.encodepos(pj, pi)
        baseoption = ["G%s" % eg]
        baseoption.append("P%s" % ep)
        baseoption.append("p%s:1" % ep)
        baseoption.append("g%s:%s" % (ep, eg))
        if pj == gj and pi == gi:
          yield from self.collect_seed(baseoption, gn, pj, pi)
        else:
          yield from self.collect_tail(baseoption, gn, pj, pi)

  def collect_empty(self):
    for j, i in itertools.product(range(self.h), range(self.w)):
      if not self.forced_fill(j, i):
        option = ["E"]
        pos = self.encodepos(j, i)
        option.append("g%s:0" % pos)
        option.append("p%s:0" % pos)
        option.append("P%s" % pos)
        for gn in index(self.groups):
          if (j, i) in self.minmax[gn]:
            option.append("t%s%s:0" % (self.encodegroup(gn), pos))
        yield " ".join(option)

  def collect_empty_pairs(self):
    for j, i in itertools.product(range(self.h), range(self.w)):
      if not self.forced_fill(j, i):
        pos = self.encodepos(j, i)
        yield "D%s p%s:1" % (pos, pos)
        for nj, ni in self.iter_neigh(j, i):
          if not self.forced_fill(nj, ni):
            option = ["D%s" % pos]
            option.append("p%s:0" % pos)
            option.append("p%s:0" % self.encodepos(nj, ni))
            yield " ".join(option)

  def collect_filled(self):
    for j, i in itertools.product(range(self.h), range(self.w)):
      if not self.forced_empty(j, i):
        if self.groups[self.seeds.get((j, i), 0)][2] != 1:
          pos = self.encodepos(j, i)
          yield "F%s p%s:0" % (pos, pos)
          for nj, ni in self.iter_neigh(j, i):
            if not self.forced_empty(nj, ni):
              option = ["F%s" % pos]
              option.append("p%s:1" % pos)
              option.append("p%s:1" % self.encodepos(nj, ni))
              yield " ".join(option)

  def collect_squares(self):
    for j, i in dlx.iter_grid(self.h - 1, self.w - 1):
      base = ["S%s" % self.encodepos(j, i)]
      for bits in range(1, 16):
        option = base.copy()
        impossible = False
        for n, (jj, ii) in enumerate(itertools.product([0, 1], repeat=2)):
          bit = int((bits & (1 << n)) > 0)
          option.append("p%s:%d" % (self.encodepos(j + jj, i + ii), bit))
          pos = j + jj, i + ii
          if self.forced_fill(*pos) and bit == 0:
            impossible = True
          if self.forced_empty(*pos) and bit == 1:
            impossible = True
        if not impossible:
          yield " ".join(option)

  def collect_direction(self, h, w, direction, move):
    for j, i in dlx.iter_grid(h, w):
      ep = self.encodepos(j, i)
      base = ["%s%s" % (direction, ep)]
      c = [self.candidates[move(j, i, k)] for k in range(2)]
      for g1, g2 in itertools.product(*c):
        if g1 == -1 or g2 == -1 or g1 == g2:
          option = base.copy()
          eg1 = self.encodegroup(g1)
          eg2 = self.encodegroup(g2)
          if g1 == g2 and g1 != -1:
            option.append("EDGE%s" % eg1)
          option.append("g%s:%s" % (ep, eg1))
          option.append("g%s:%s" % (self.encodepos(*move(j, i, 1)), eg2))
          option.append("p%s:%s" % (ep, int(g1 >= 0)))
          option.append("p%s:%s" % (self.encodepos(*move(j, i, 1)), int(g2 >= 0)))
          yield " ".join(option)

  def collect_pairs(self):
    yield from self.collect_direction(
      self.h, self.w - 1, "H", lambda j, i, k: (j, i + k))
    yield from self.collect_direction(
      self.h - 1, self.w, "V", lambda j, i, k: (j + k, i))

  def collect_twos(self):
    for gj, gi, gsize in self.groups:
      if gsize == 2:
        pos = self.encodepos(gj, gi)
        for jj, ii in itertools.product([-1, 1], repeat=2):
          pj, pi = gj + jj, gi + ii
          if self.inside(pj, pi):
            option = ["_2%s" % pos]
            option.append("p%s:0" % self.encodepos(pj, pi))
            nj, ni = gj, gi - ii
            if self.inside(nj, ni):
              option.append("p%s:0" % self.encodepos(nj, ni))
            nj, ni = gj - jj, gi
            if self.inside(nj, ni):
              option.append("p%s:0" % self.encodepos(nj, ni))
            yield " ".join(option)

  def collect_primary(self, options):
    items = {}
    empty = self.empty_size
    for option in options:
      for item in option.split(" "):
        if item.startswith("G"):
          gn = self.decodegroup(item[1:])
          items.setdefault("%d|%s" % (self.groups[gn][2], item), len(items))
        elif item.startswith("EDGE"):
          gn = self.decodegroup(item[4])
          _, _, gsize = self.groups[gn]
          # Max edges is given by A123663
          maxedges = 2 * gsize - int(math.ceil(2 * gsize ** 0.5))
          items.setdefault("%d:%d|%s" % (gsize - 1, maxedges, item), len(items))
        elif item.startswith("E"):
          items.setdefault("%d|%s" % (empty, item), len(items))
        elif item.startswith("T"):
          gn = self.decodegroup(item[1])
          size = self.treesize[gn][int(item[2:])]
          items.setdefault("%d:%d|%s" % (size[0], size[1], item), len(items))
        elif item[0].isupper() or item[0] == "_":
          items.setdefault(item, len(items))
    reverse = {v: k for k, v in items.items()}
    return [reverse[k] for k in sorted(reverse)]

def main():
  h, w = map(int, input().split())
  ngroups = int(input().strip())
  groups = [tuple(map(int, input().split())) for _ in range(ngroups)]
  solver = Nurikabe(h, w, groups)
  options = ["_W%d" % w, "_H%d" % h]
  options.extend(solver.collect_groups())
  options.extend(solver.collect_empty_pairs())
  options.extend(solver.collect_empty())
  options.extend(solver.collect_filled())
  options.extend(solver.collect_squares())
  options.extend(solver.collect_pairs())
  print("\n".join(dlx.build_dlx(options, primary=solver.collect_primary)))

main()
