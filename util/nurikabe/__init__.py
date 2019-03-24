import collections
import re

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
  grid = [["."] * w for _ in range(h)]
  tree = [["."] * w for _ in range(h)]
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
    grid[p[1]][p[0]] = groups[g]
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
    if not any(item.startswith("E") for item in line.split()):
      continue
    for item in line.split(" "):
      match = re.match(r"^p(\d\d)(\d\d):0", item)
      if match:
        j = int(match.group(1))
        i = int(match.group(2))
        tree[i][j] = "+"
        grid[i][j] = "+"
  for line in solution:
    for item in line.split(" "):
      match = re.match(r"^p(\d\d)(\d\d):1", item)
      if match:
        j = int(match.group(1))
        i = int(match.group(2))
        if grid[i][j] == ".":
          tree[i][j] = "?"
          grid[i][j] = "?"
  return "\n".join(" %s" % " ".join(
    "%s%s" % (a,b) for a,b in zip(l1,l2)) for l1,l2 in zip(grid,tree))

