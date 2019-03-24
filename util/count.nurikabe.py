import sys
import re
import itertools
import dlx
import collections
import nurikabe

def main():
  solutions = set()
  for solution in dlx.parse_solutions(sys.stdin):
    solutions.add(nurikabe.draw(solution))
  print(len(solutions))

main()
