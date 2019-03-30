import collections
import re
import argparse
import itertools

def encodesize(size):
  if not size:
    return "-"
  if size < 10:
    return str(size)
  else:
    return chr(ord('A') + size - 10)

def matrix(h, w, default):
  return [[default] * w for _ in range(h)]

def iter_item(solution, start, regexp):
  for line in solution:
    items = line.split()
    if any(item.startswith(start) for item in items):
      for item in items:
        match = re.search(regexp, item)
        if match:
          groups = list(match.groups())
          for i in range(2):
            groups[i] = int(groups[i])
          yield groups

def get_size(solution):
  h, w = None, None
  for line in solution:
    match = re.search(r"_W(\d+)", line)
    if match:
      h = int(match.group(1))
    match = re.search(r"_H(\d+)", line)
    if match:
      w = int(match.group(1))
  return h, w

def draw(solution):
  h, w = get_size(solution)
  if not h or not w:
    return ""

  default = "."
  grid = matrix(h, w, default)
  tree = matrix(h, w, default)
  types = matrix(h, w, default)
  groups = collections.Counter()

  for j, i, group in iter_item(solution, "G", r"(?<!\w)g(\d\d)(\d\d):(.)"):
    groups[group] += 1
    grid[i][j] = group

  for j, i, g, v in iter_item(solution, "G", r"^t(\d\d)(\d\d)(.):(.)"):
    if grid[i][j] != default:
      tree[i][j] = v

  for j, i, v in iter_item(solution, "R", r"^r(\d\d)(\d\d):(.)"):
    tree[i][j] = v

  for j, i, v in iter_item(solution, "R", r"^y(\d\d)(\d\d):(.)"):
    types[i][j] = v

  for j, i in iter_item(solution, "E", r"^p(\d\d)(\d\d):0"):
    grid[i][j] = "-"

  for j, i in iter_item(solution, "", r"^p(\d\d)(\d\d):0"):
    if grid[i][j] == default:
      tree[i][j] = "?"
      grid[i][j] = "?"

  if not args.label:
    for j, i in itertools.product(range(h), range(w)):
      grid[i][j] = encodesize(groups[grid[i][j]])

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
  parser.add_argument("--label", action="store_true",
      help="Show group label")
  args = parser.parse_args()

