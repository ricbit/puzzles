import dlx
import sys

for n, solution in enumerate(dlx.parse_solutions(sys.stdin)):
  f = open("%s.%d.txt" % (sys.argv[1], n), "w")
  f.write("".join(sorted(solution)))
  f.close()

