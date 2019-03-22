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
  for line in solution:
    match = re.search(r"_W(\d+)", line)
    if match:
      h = int(match.group(1))
    match = re.search(r"_H(\d+)", line)
    if match:
      w = int(match.group(1))
  grid = [["."] * w for _ in range(h)]
  tree = [["."] * w for _ in range(h)]
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
      match = re.match(r"^t(.)(\d\d)(\d\d):(.)", item)
      if match:
        i = int(match.group(2))
        j = int(match.group(3))
        v = match.group(4)
        tree[j][i] = v
  for p, g in pos.items():
    grid[p[1]][p[0]] = encode(groups[g])
  print("%s\n" % "\n".join("".join(line) for line in grid))
  print("%s\n\n" % "\n".join("".join(line) for line in tree))

def main():
  for solution in dlx.parse_solutions(sys.stdin):
    draw_nurikabe(solution)

main()
