import sys
import re
import dlx
import nurikabe

def main():
  lines = sys.stdin.readlines()
  last = ""
  lastsolution = []
  for solution, branch in dlx.parse_partial_solutions(lines):
    grid = nurikabe.draw(solution)
    if grid != last:
      if len(solution) < len(lastsolution):
        print("Backtrack")
      print("%d %s : %s" % (len(solution), branch, solution[-1]))
      print(grid)
      last = grid
      lastsolution = solution
      print("\n%s\n\n" % ("-" * 10))

main()
