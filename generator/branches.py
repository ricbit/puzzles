import sys
import random

w = int(sys.argv[1])
h = int(sys.argv[2])

grid = [[-1] * w for _ in xrange(h)]

def iter_empty_cells(grid):
  pos = 0
  for j in xrange(h):
    for i in xrange(w):
      if grid[j][i] == -1:
        yield (j, i, pos)
        pos += 1

def count(grid):
  ans = 0
  for j, i, pos in iter_empty_cells(grid):
    ans = pos + 1
  return ans

def find(grid, x):
  for j, i, pos in iter_empty_cells(grid):
    if x == pos:
      return (j, i)
  return None

alld = [(1,0), (0,1), (-1,0), (0,-1)]

def iter_direction(grid, j, i, d):
  di, dj = d
  cur = 0
  ii, jj = i + di, j + dj
  while ii >= 0 and jj >= 0 and ii < w and jj < h and grid[jj][ii] == -1:
    yield (jj, ii, cur)
    cur += 1
    ii += di
    jj += dj

def traverse(grid, j, i, d):
  ans = 0
  for jj, ii, cur in iter_direction(grid, j, i, d):
    ans = cur + 1
  return ans

def fill(grid, j, i, d, left):
  if left == 0:
    return 0
  tmax = min(left, random.randint(0, traverse(grid, j, i, d)))
  ans = 0
  for jj, ii, cur in iter_direction(grid, j, i, d):
    if cur <= tmax:
      grid[jj][ii] = -2
      ans = cur + 1
  return ans

g = 0
while True:
  m = count(grid)
  if m == 0:
    break
  x = random.randrange(m)
  j, i = find(grid, x)
  size = 0
  left = 8
  random.shuffle(alld)
  for d in alld:
    f = fill(grid, j, i, d, left)
    size += f
    left -= f
  grid[j][i] = size
  g += 1

print w, h, g
for line in grid:
  print ''.join('.' if x < 0 else str(x) for x in line)
  
