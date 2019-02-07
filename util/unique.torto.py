import sys

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

grids = [line.strip() for line in sys.stdin]
unique = set()
seen = set()
for grid in grids:
  if grid not in seen:
    if sys.argv[1] == "sorted":
      grids = [to_short(x) for x in flip(to_long(grid))]
      grids.sort()
      unique.add(grids[0])
    else:
      unique.add(grid)
  for flipped in flip(to_long(grid)):
    seen.add(to_short(flipped))
if sys.argv[1] in "unique":
  ans = unique
elif sys.argv[1] == "sorted":
  ans = sorted(unique)
else:
  ans = seen
for g in ans:
  print g
