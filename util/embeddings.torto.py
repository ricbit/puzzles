# Usage: python embeddings.torto.py words.txt < torto.unique.txt

import sys

def iter_directions(j, i):
  for x in xrange(-1, 2):
    for y in xrange(-1, 2):
      if x != 0 or y != 0:
        yield (y + j, x + i)

def valid_movements(j, i):
  for (j, i) in iter_directions(j, i):
    if 0 <= j <= 5 and 0 <= i <= 2:
      yield (j, i)

def search_word(torto, word, pos, j, i, used, diags):
  if pos >= len(word):
    return 1
  count = 0
  for (jj, ii) in valid_movements(j, i):
    if torto[jj][ii] != word[pos]:
      continue
    if used[jj][ii]:
      continue
    diag = []
    if jj != j and ii != i:
      diag = (min(jj, j), min(ii, i))
      if diag in diags:
        continue
    used[jj][ii] = True
    count += search_word(torto, word, pos + 1, jj, ii, used, diags + [diag])
    used[jj][ii] = False
  return count

def count_word(torto, word):
  count = 0
  for j in xrange(6):
    for i in xrange(3):
      if torto[j][i] != word[0]:
        continue
      used = [[False] * 3 for _ in xrange(6)]
      used[j][i] = True
      diags = []
      count += search_word(torto, word, 1, j, i, used, diags)
  return count

def count_solutions(sol, words):
  torto = [sol[i*3:i*3+3] for i in xrange(6)]
  ans = 1
  for word in words:
    count = count_word(torto, word)
    if not count:
      print >> sys.stderr, "Word %s missing from solution %s" % (word, torto)
      sys.exit(1)
    ans *= count
  return ans

def main():
  if len(sys.argv) < 2:
    print >> sys.stderr, "Missing words file"
    sys.exit(1)
  f = open(sys.argv[1], "rt")
  nwords = int(f.readline())
  words = [f.readline().strip() for _ in xrange(nwords)]
  f.close()
  solutions = sys.stdin.readlines()
  count = 0
  for sol in solutions:
    count += count_solutions(sol, words)
  print count

main()
