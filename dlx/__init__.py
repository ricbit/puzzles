import string
import math
import re
import itertools
import sys

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
    n //= base
  return ans

def encode(n, limit, basecache={}, ncache={}):
  if (n, limit) in ncache:
    return ncache[(n, limit)]
  if limit in basecache:
    size = basecache[limit]
  else:
    # OB1 here
    size = int(math.floor(math.log(limit, ENCODEBASE))) + 1
    basecache[limit] = size
  ans = "".join(PRINTABLE[x] for x in _to_base(n, ENCODEBASE, size))
  ncache[(n, limit)] = ans
  return ans

def decode(single):
  return self.PRINTABLE.index(single)

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

def _id(items):
  for item in items:
    yield item

def build_dlx(options,
    primary=_collect_primary, secondary=_collect_secondary,
    sorted_items=False, sorted_options=False):
  item_sorter = sorted if sorted_options else _id
  new_options = []
  for option in options:
    new_options.append(" ".join(item_sorter(option.split(" "))))
  if sorted_options:
    new_options.sort()
  poptions = primary(new_options)
  soptions = secondary(new_options)
  if soptions:
    yield "%s | %s" % (" ".join(poptions), " ".join(soptions))
  else:
    yield "%s" % (" ".join(poptions))
  for option in new_options:
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
  last_branch = ""
  for line in lines:
    match = re.match(r"^Level.*branching on (.*)$", line)
    if match:
      last_branch = match.group(1)
      continue
    match = re.match(r"^L(\d+):\s+(.*)\s+\(\d+ of \d+\)", line)
    if match:
      n = int(match.group(1))
      option = match.group(2)
      if n >= len(solution):
        solution.append(option)
      else:
        solution = solution[:n] + [option]
      yield solution, last_branch

def iter_grid(height, width):
  for j, i in itertools.product(range(height), range(width)):
    yield (j, i)
