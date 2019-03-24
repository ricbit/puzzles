import sys
import dlx
import nurikabe

def main():
  for solution in dlx.parse_solutions(sys.stdin):
    print(nurikabe.draw(solution))

main()
