import sys
import re
import dlx
import nurikabe

def main():
  lines = sys.stdin.readlines()
  last = ""
  for solution in dlx.parse_partial_solutions(lines):
    grid = nurikabe.draw(solution)
    if grid != last:
      print(solution[-1])
      print(grid)
      last = grid
      print("\n%s\n\n" % ("-" * 10))

main()
