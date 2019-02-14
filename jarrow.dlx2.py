import itertools

DIR = {
  "U": (-1, 0),
  "D": (1, 0),
  "L": (0, -1),
  "R": (0, 1)
}  

def collect_heads(n, arrows, numbers):
  heads = []
  for j in xrange(n):
    line = []
    for i in xrange(n):
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
      line.append((seen, cell, minsize, maxsize))
    heads.append(line)
  return heads

def collect_targets(n, heads):
  targets = []
  for j in xrange(n):
    line = []
    for i in xrange(n):
      cell = []
      for jj in xrange(n):
        for ii in xrange(n):
          for (tj, ti) in heads[jj][ii][1]:
            if tj == j and ti == i:
              cell.append((jj, ii))
      line.append(cell)
    targets.append(line)
  return targets
  
def encode(x):
  return chr(ord('a') + x)

def collect_cells(n, heads, targets, numbers):
  for j in xrange(n):
    for i in xrange(n):
      minsize = heads[j][i][2]
      maxsize = heads[j][i][3]
      for size in xrange(minsize, maxsize + 1):
        option = ["C%s%s" % (encode(j), encode(i))]
        option.append("c%s%s:%d" % (encode(j), encode(i), size))
        for target in targets[j][i]:
          option.append("h%s%s%d:1" % (encode(target[0]), encode(target[1]), size))
        yield " ".join(option)
        choices = set(range(1, 10)) - set(heads[j][i][0])
        for comb in itertools.combinations(choices, size - len(heads[j][i][0])):
          all_choices = set(comb).union(set(heads[j][i][0]))
          option = ["H%s%s" % (encode(j), encode(i))]
          option.append("c%s%s:%d" % (encode(j), encode(i), size))
          for k in xrange(1, 10):
            option.append("h%s%s%d:%d" % (encode(j), encode(i), k, int(k in all_choices)))
          yield " ".join(option)

def collect_greater(n, heads, arrows):
  for j in xrange(n):
    for i in xrange(n):
      for tj, ti in heads[j][i][1]:
        if arrows[tj][ti] == arrows[j][i]:
          minsize = heads[j][i][2]
          maxsize = heads[j][i][3]
          for s1 in xrange(minsize, maxsize + 1):
            for s2 in xrange(heads[tj][ti][2], s1 + 1):
              option = ["G%s%s%s%s" % (encode(j), encode(i), encode(tj), encode(ti))]
              option.append("c%s%s:%d" % (encode(j), encode(i), s1))
              option.append("c%s%s:%d" % (encode(tj), encode(ti), s2))
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
  heads = collect_heads(n, arrows, numbers)
  targets = collect_targets(n, heads)
  options = []
  options.extend(collect_cells(n, heads, targets, numbers))
  options.extend(collect_greater(n, heads, arrows))
  primary = collect_primary(options)
  secondary = collect_secondary(options)
  print "%s | %s" % (" ".join(primary), " ".join(secondary))
  for option in options:
    print option


main()
