import itertools
import functools

DIR = {
  "U": (-1, 0),
  "D": (1, 0),
  "L": (0, -1),
  "R": (0, 1)
}

def collect_heads(n, arrows, numbers, j, i):
  jj, ii = DIR[arrows[j][i]]
  pj = j + jj
  pi = i + ii
  cell = []
  seen = set()
  while 0 <= pi < n and 0 <= pj < n:
    cell.append((pj, pi))
    if numbers[pj][pi] != '.':
      seen.add(int(numbers[pj][pi]))
    pj += jj
    pi += ii
  minsize = max(1, len(seen))
  maxsize = sum(1 for jj, ii in cell if numbers[jj][ii] == ".") + minsize
  if numbers[j][i] != '.':
    minsize = maxsize = int(numbers[j][i])
  return (seen, cell, minsize, maxsize)

def create_grid(n, func, *args):
  f = functools.partial(func, *args)
  targets = []
  for j in xrange(n):
    line = []
    for i in xrange(n):
      line.append(f(j, i))
    targets.append(line)
  return targets

def iter_grid(n):
  for j in xrange(n):
    for i in xrange(n):
      yield (j, i)

def collect_targets(n, heads, j, i):
  cell = []
  for jj, ii in iter_grid(n):
    for tj, ti in heads[jj][ii][1]:
      if tj == j and ti == i:
        cell.append((jj, ii))
  return cell

def encode(x):
  return chr(ord('a') + x)

def encode_pos(j, i, *more):
  return (encode(j), encode(i)) + tuple(more)

def collect_cells(n, heads, targets, numbers):
  for j, i in iter_grid(n):
    minsize = heads[j][i][2]
    maxsize = heads[j][i][3]
    for size in xrange(minsize, maxsize + 1):
      option = ["#C%s%s" % encode_pos(j, i)]
      option.append("c%s%s:%s" % encode_pos(j, i, encode(size)))
      for x in xrange(1, 10):
        option.append("i%s%s%d:%d" % encode_pos(j, i, x, int(x == size)))
      for target in targets[j][i]:
        option.append("h%s%s%d:1" % encode_pos(target[0], target[1], size))
      yield " ".join(option)
      choices = set(range(1, 10)) - set(heads[j][i][0])
      for comb in itertools.combinations(choices, size - len(heads[j][i][0])):
        all_choices = set(comb).union(set(heads[j][i][0]))
        option = ["#H%s%s" % encode_pos(j, i)]
        option.append("c%s%s:%s" % encode_pos(j, i, encode(size)))
        for k in xrange(1, 10):
          option.append("h%s%s%d:%d" % encode_pos(j, i, k, int(k in all_choices)))
        yield " ".join(option)

def collect_greater(n, heads, arrows):
  for j, i in iter_grid(n):
    for tj, ti in heads[j][i][1]:
      if arrows[tj][ti] == arrows[j][i]:
        minsize = heads[j][i][2]
        maxsize = heads[j][i][3]
        for s1 in xrange(minsize, maxsize + 1):
          for s2 in xrange(heads[tj][ti][2], s1 + 1):
            option = ["#G%s%s%s%s" % (encode_pos(j, i) + encode_pos(tj, ti))]
            option.append("c%s%s:%s" % encode_pos(j, i, encode(s1)))
            option.append("c%s%s:%s" % encode_pos(tj, ti, encode(s2)))
            yield " ".join(option)

def collect_empty(n, heads):
  for j, i in iter_grid(n):
    for k in xrange(1, 10):
      option = ["E%s%s%d" % encode_pos(j, i, k)]
      option.append("h%s%s%d:0" % encode_pos(j, i, k))
      for jj, ii in heads[j][i][1]:
        option.append("i%s%s%d:0" % encode_pos(jj, ii, k))
      yield " ".join(option)
      for jj, ii in heads[j][i][1]:
        option = ["E%s%s%d" % encode_pos(j, i, k)]
        option.append("h%s%s%d:1" % encode_pos(j, i, k))
        option.append("i%s%s%d:1" % encode_pos(jj, ii, k))
        yield " ".join(option)

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

def main():
  n = int(raw_input())
  arrows = [raw_input().strip() for _ in xrange(n)]
  numbers = [raw_input().strip() for _ in xrange(n)]
  heads = create_grid(n, collect_heads, n, arrows, numbers)
  targets = create_grid(n, collect_targets, n, heads)
  options = []
  options.extend(collect_cells(n, heads, targets, numbers))
  options.extend(collect_greater(n, heads, arrows))
  options.extend(collect_empty(n, heads))
  primary = collect_primary(options)
  secondary = collect_secondary(options)
  print "%s | %s" % (" ".join(primary), " ".join(secondary))
  for option in options:
    print option


main()