import sys
import re
import itertools
import dlx
import collections

def encode(size):
  if size < 10:
    return str(size)
  else:
    return chr(ord('A') + size - 10)

def draw_nurikabe(solution):
  pos = {}
  groups = collections.Counter()
  for line in solution:
    if not any(item.startswith("G") for item in line.split()):
      continue
    for item in line.split():
      match = re.search(r"(?<!\w)g(\d\d)(\d\d):(\w)", item)
      if match and match.group(3)[0] != "0":
        j = int(match.group(1))
        i = int(match.group(2))
        g = ord(match.group(3)) - ord('a')
        groups[g] += 1
        pos[(j, i)] = g
  h = 1 + max(p[1] for p in pos)
  w = 1 + max(p[0] for p in pos)
  grid = [["."] * w for _ in range(h)]
  for p, g in pos.items():
    grid[p[1]][p[0]] = encode(groups[g])
  print("\n".join("".join(line) for line in grid))

def main():
  for solution in dlx.parse_solutions(sys.stdin):
    draw_nurikabe(solution)

main()
