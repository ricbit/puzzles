import collections
import re
import argparse

def encodesize(size):
  if size < 10:
    return str(size)
  else:
    return chr(ord('A') + size - 10)

def matrix(h, w, default):
  return [[default] * w for _ in range(h)]

def draw(solution):
  h, w = None, None
  for line in solution:
    match = re.search(r"_W(\d+)", line)
    if match:
      h = int(match.group(1))
    match = re.search(r"_H(\d+)", line)
    if match:
      w = int(match.group(1))
  if not h or not w:
    return ""
  grid = matrix(h, w, ".")
  tree = matrix(h, w, ".")
  types = matrix(h, w, ".")
  pos = {}
  groups = collections.Counter()
  for line in solution:
    if not any(item.startswith("G") for item in line.split()):
      continue
    for item in line.split(" "):
      match = re.search(r"(?<!\w)g(\d\d)(\d\d):(.)", item)
      if match and match.group(3)[0] != "0":
        j = int(match.group(1))
        i = int(match.group(2))
        g = match.group(3)
        groups[g] += 1
        pos[(j, i)] = g
  for p, g in pos.items():
    grid[p[1]][p[0]] = encodesize(groups[g])
  for line in solution:
    if not any(item.startswith("G") for item in line.split()):
      continue
    for item in line.split(" "):
      match = re.match(r"^t(.)(\d\d)(\d\d):(.)", item)
      if match:
        g = match.group(1) #ord(match.group(1)) - ord('A')
        j = int(match.group(2))
        i = int(match.group(3))
        v = match.group(4)
        if g == pos.get((j, i), -1):
          tree[i][j] = v
  for line in solution:
    if not any(item[0] in ["R"] for item in line.split()):
      continue
    for item in line.split(" "):
      match = re.match(r"^r(\d\d)(\d\d):(.)", item)
      if match:
        j = int(match.group(1))
        i = int(match.group(2))
        v = match.group(3)
        tree[i][j] = v
      match = re.match(r"^t(\d\d)(\d\d):(.)", item)
      if match:
        j = int(match.group(1))
        i = int(match.group(2))
        v = match.group(3)
        types[i][j] = v
  for line in solution:
    if not any(item.startswith("E") for item in line.split()):
      continue
    for item in line.split(" "):
      match = re.match(r"^p(\d\d)(\d\d):0", item)
      if match:
        j = int(match.group(1))
        i = int(match.group(2))
        grid[i][j] = "-"
  for line in solution:
    for item in line.split(" "):
      match = re.match(r"^p(\d\d)(\d\d):1", item)
      if match:
        j = int(match.group(1))
        i = int(match.group(2))
        if grid[i][j] == ".":
          tree[i][j] = "?"
          grid[i][j] = "?"
  if args.empty:
    if args.type:
      return "\n".join(" %s" % " ".join(
        "%s%s%s" % (a,b,c) for a,b,c in zip(l1,l2,l3)) for l1,l2,l3 in zip(grid,tree,types))
    else:
      return "\n".join(" %s" % " ".join(
        "%s%s" % (a,b) for a,b in zip(l1,l2)) for l1,l2 in zip(grid,tree))
  else:
    return "\n".join("".join(line) for line in grid)

try:
  a = init
except:
  init = True
  parser = argparse.ArgumentParser(
      description="Configure nurikabe tools")
  parser.add_argument("--empty", action="store_true",
      help="Show empty tree")
  parser.add_argument("--type", action="store_true",
      help="Show empty type")
  args = parser.parse_args()

