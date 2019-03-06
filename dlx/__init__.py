import string
import math
import sys

printable = "".join(x for x in string.printable if x not in ":|" and ord(x) > 32)
encodebase = len(printable)

def to_base(n, base, size):
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
    size = int(math.floor(math.log(limit, encodebase))) + 1
    basecache[limit] = size
  ans = "".join(printable[x] for x in to_base(n, encodebase, size))
  ncache[(n, limit)] = ans
  return ans

def collect_primary(options):
  items = set()
  for option in options:
    for item in option.split():
      if item[0].isupper() or item[0] == "#":
        items.add(item)
  return items

def collect_secondary(options):
  items = set()
  for option in options:
    for item in option.split():
      if item[0].islower():
        items.add(item.split(":")[0])
  return items

def build_dlx(options):
  primary = collect_primary(options)
  secondary = collect_secondary(options)
  yield "%s | %s" % (" ".join(primary), " ".join(secondary))
  for option in options:
    yield option
