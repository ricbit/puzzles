# Takes a description of a Japanese Arrows puzzle and 
# build a model to run with Knuth's dlx2 solver.
#
# Usage: python jarrow.dlx2.py < jarrow.txt > model.dlx

import itertools
import functools

DIR = {
  "^": (-1, 0),
  "v": (1, 0),
  "<": (0, -1),
  ">": (0, 1)
}

def collect_targets(n, directions, numbers, j, i):
  jj, ii = DIR[directions[j][i]]
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
  minsize = max(1, len(seen)) if len(cell) else 0
  maxsize = sum(1 for jj, ii in cell if numbers[jj][ii] == ".") + len(seen)
  if numbers[j][i] != '.':
    minsize = maxsize = int(numbers[j][i])
  return (seen, cell, minsize, maxsize)

def create_grid(n, func, *args):
  f = functools.partial(func, *args)
  directions = []
  for j in xrange(n):
    line = []
    for i in xrange(n):
      line.append(f(j, i))
    directions.append(line)
  return directions

def iter_grid(n):
  for j in xrange(n):
    for i in xrange(n):
      yield (j, i)

def collect_arrows(n, targets, j, i):
  cell = []
  for jj, ii in iter_grid(n):
    for tj, ti in targets[jj][ii][1]:
      if tj == j and ti == i:
        cell.append((jj, ii))
  return cell

def encode(x):
  return chr(ord('a') + x)

def encode_pos(j, i, *more):
  return (encode(j), encode(i)) + tuple(more)

def collect_cells(n, targets, arrows, numbers):
  for j, i in iter_grid(n):
    minsize = targets[j][i][2]
    maxsize = targets[j][i][3]
    for size in xrange(minsize, maxsize + 1):
      option = ["#C%s%s" % encode_pos(j, i)]
      option.append("c%s%s:%s" % encode_pos(j, i, encode(size)))
      for x in xrange(0, 10):
        option.append("i%s%s%d:%d" % encode_pos(j, i, x, int(x == size)))
      for arrow in arrows[j][i]:
        option.append("h%s%s%d:1" % encode_pos(arrow[0], arrow[1], size))
      yield " ".join(option)
      if len(targets[j][i][1]):
        choices = set(range(0, 10)) - set(targets[j][i][0])
        combs = itertools.combinations(choices, size - len(targets[j][i][0]))
      else:
        combs = [[0]]
      for comb in combs:
        all_choices = set(comb).union(set(targets[j][i][0]))
        option = ["#H%s%s" % encode_pos(j, i)]
        option.append("c%s%s:%s" % encode_pos(j, i, encode(size)))
        for k in xrange(0, 10):
          option.append("h%s%s%d:%d" % encode_pos(j, i, k, int(k in all_choices)))
        yield " ".join(option)

def collect_greater(n, targets, directions):
  for j, i in iter_grid(n):
    for tj, ti in targets[j][i][1]:
      if directions[tj][ti] == directions[j][i]:
        minsize = targets[j][i][2]
        maxsize = targets[j][i][3]
        for s1 in xrange(minsize, maxsize + 1):
          for s2 in xrange(targets[tj][ti][2], s1 + 1):
            option = ["G%s%s%s%s" % (encode_pos(j, i) + encode_pos(tj, ti))]
            option.append("c%s%s:%s" % encode_pos(j, i, encode(s1)))
            option.append("c%s%s:%s" % encode_pos(tj, ti, encode(s2)))
            yield " ".join(option)

def collect_empty(n, targets):
  for j, i in iter_grid(n):
    for k in xrange(0, 10):
      if targets[j][i][1] and k > 0:
        option = ["E%s%s%d" % encode_pos(j, i, k)]
        option.append("h%s%s%d:0" % encode_pos(j, i, k))
        for jj, ii in targets[j][i][1]:
          option.append("i%s%s%d:0" % encode_pos(jj, ii, k))
        yield " ".join(option)
        for jj, ii in targets[j][i][1]:
          option = ["E%s%s%d" % encode_pos(j, i, k)]
          option.append("h%s%s%d:1" % encode_pos(j, i, k))
          option.append("i%s%s%d:1" % encode_pos(jj, ii, k))
          yield " ".join(option)
      elif k == 0 and not targets[j][i][1]:
        option = ["E%s%s0" % encode_pos(j, i)]
        option.append("h%s%s0:1" % encode_pos(j, i))
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

def print_targets(n, targets, arrows):
  for j in xrange(n):
    for i in xrange(n):
      print j, i, "target", targets[j][i]
      print j, i, "arrow", arrows[j][i]

def main():
  n = int(raw_input())
  directions = [raw_input().strip() for _ in xrange(n)]
  numbers = [raw_input().strip() for _ in xrange(n)]
  targets = create_grid(n, collect_targets, n, directions, numbers)
  arrows = create_grid(n, collect_arrows, n, targets)
  options = []
  options.extend(collect_cells(n, targets, arrows, numbers))
  options.extend(collect_greater(n, targets, directions))
  options.extend(collect_empty(n, targets))
  primary = collect_primary(options)
  secondary = collect_secondary(options)
  print "%s | %s" % (" ".join(primary), " ".join(secondary))
  for option in options:
    print option


main()
