import sys

def main():
  lines = [line.strip() for line in sys.stdin.readlines()]
  h = lines.index("")
  w = len(lines[0].split()[0])
  start = 0
  grids = set()
  while start < len(lines):
    grid = tuple(x.split()[0] for x in lines[start:start + h])
    if len(grid) == h:
      grids.add(grid)
    start += h + 1
  print >> sys.stderr, len(grids)
  for grid in grids:
    for line in grid:
      print line
    print

main()
