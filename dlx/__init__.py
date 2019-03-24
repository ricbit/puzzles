import string
import math
import re
import itertools

try:
  range = xrange
except NameError:
  pass

PRINTABLE = "".join(x for x in string.printable if x not in ":|" and ord(x) > 32)
ENCODEBASE = len(PRINTABLE)

def _to_base(n, base, size):
  ans = [0] * size
  pos = size - 1
  while n:
    ans[pos] = n % base
    pos -= 1
    n /= base
  return ans

def encode(n, limit, basecache={}, ncache={}):
  if (n, limit) in ncache:
    return ncache[(n, limit)]
  if limit in basecache:
    size = basecache[limit]
  else:
    size = int(math.floor(math.log(limit, ENCODEBASE))) + 1
    basecache[limit] = size
  ans = "".join(PRINTABLE[x] for x in _to_base(n, ENCODEBASE, size))
  ncache[(n, limit)] = ans
  return ans

def _collect_primary(options):
  items = set()
  for option in options:
    for item in option.split():
      if item[0].isupper() or item[0] == "#":
        items.add(item)
  return items

def _collect_secondary(options):
  items = set()
  for option in options:
    for item in option.split():
      if item[0].islower():
        items.add(item.split(":")[0])
  return items

def build_dlx(options, primary=_collect_primary, secondary=_collect_secondary):
  sorted_options = []
  for option in options:
    sorted_options.append(" ".join(sorted(option.split(" "))))
  poptions = primary(sorted_options)
  soptions = secondary(sorted_options)
  if soptions:
    yield "%s | %s" % (" ".join(poptions), " ".join(soptions))
  else:
    yield "%s" % (" ".join(poptions))
  for option in sorted_options:
    yield option

def parse_solutions(lines):
  solution = []
  for line in lines:
    if re.match(r"^\d+:$", line):
      if solution:
        yield solution
        solution = []
    if re.match(r"^\s.*of \d+\)$", line) is not None:
      solution.append(line)
  if solution:
    yield solution

def parse_partial_solutions(lines):
  solution = []
  for line in lines:
    match = re.match(r"^L(\d+):\s+(.*)\s+\(\d+ of \d+\)", line)
    if match:
      n = int(match.group(1))
      option = match.group(2)
      if n >= len(solution):
        solution.append(option)
      else:
        solution = solution[:n] + [option]
      yield solution

def iter_grid(height, width):
  for j, i in itertools.product(range(height), range(width)):
    yield (j, i)
