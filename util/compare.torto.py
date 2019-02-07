import sys
import itertools

def flipx(g):
  return [gg[::-1] for gg in g]

def flipy(g):
  return g[::-1]

def flip(grid):
  yield grid
  yield flipx(grid)
  yield flipy(grid)
  yield flipx(flipy(grid))

def to_long(short_grid):
  return [short_grid[3*n:3*n+3] for n in xrange(6)]

def to_short(long_grid):
  return "".join(gg for gg in long_grid)

def canonical(short_grid):
  grids = [to_short(g) for g in flip(to_long(short_grid))]
  grids.sort()
  return grids[0]

def process(gridmap, grids, slot):
  for grid in grids:
    key = canonical(grid)
    gridmap.setdefault(key, ([], []))[slot].append(grid)

all_grids = [[g.strip() for g in open(sys.argv[n]).readlines()] for n in xrange(1, 3)]

gridmap = {}
for n, grids in enumerate(all_grids):
  process(gridmap, grids, n)

for k, gs in gridmap.iteritems():
  grouped = [[(k, len(list(v))) for k,v in itertools.groupby(sorted(g))] for g in gs]
  grouped = [list(sorted(g)) for g in grouped]
  print "Canonical: %s (first: %d, second: %d)\n" % (canonical(k), len(grouped[0]), len(grouped[1]))
  for (k1, v1), (k2, v2) in itertools.izip_longest(*grouped, fillvalue=(" "*18, 0)):
    vs = [("(x%2d)" % v if v else " " * 5) for v in [v1, v2]]
    print "%s %s | %s %s" % (k1, vs[0], k2, vs[1])
  print "\n%s\n" % ("." * 79)
