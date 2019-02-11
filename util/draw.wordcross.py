import sys
import re
import itertools

def draw_words_r(words, regexp):
  pos = []
  for word in words:
    for match in re.findall(regexp, word):
      pos.append(match)
  maxj = 1 + max(int(p[0]) for p in pos)
  maxi = 1 + max(int(p[1]) for p in pos)
  grid = [["."] * maxi for _ in xrange(maxj)]
  for j, i, c in pos:
    grid[int(j)][int(i)] = c
  for line in grid:
    yield "".join(line)

def decode(x):
  return ord(x) - ord('a')

def recode(x):
  n = decode(x)
  if n < 10:
    return str(n)
  else:
    return chr(n - 10 + ord("A"))

def draw_words_u(words, regexp):
  pos = []
  for word in words:
    for match in re.findall(regexp, word):
      pos.append(match)
  maxj = 1 + max(decode(p[1]) for p in pos)
  maxi = 1 + max(decode(p[2]) for p in pos)
  grid = [["."] * maxi for _ in xrange(maxj)]
  for nw, j, i, c in pos:
    if c == '0':
      continue
    jj = decode(j)
    ii = decode(i)
    nc = recode(nw)
    if grid[jj][ii] not in ['.', nc]:
      grid[jj][ii] = 'X'
    else:
      grid[jj][ii] = recode(nw)
  for line in grid:
    yield "".join(line)

def draw_words_s(words, regexp):
  pos = []
  for word in words:
    for match in re.findall(regexp, word):
      pos.append(match)
  maxj = 1 + max(decode(p[0]) for p in pos)
  maxi = 1 + max(decode(p[1]) for p in pos)
  grid = [["."] * maxi for _ in xrange(maxj)]
  for j, i, c in pos:
    jj = decode(j)
    ii = decode(i)
    grid[jj][ii] = c
  for line in grid:
    yield "".join(line)

def draw_words(words):
  grid = draw_words_r(words, r"p(\d+)_(\d+):(\w)")
  word = draw_words_u(words, r"u(\w)(\w)(\w):(\w)")
  used = draw_words_s(words, r"u(\w)(\w):(\w)")
  for lines in itertools.izip_longest(grid, word, used):
    mw = max(len(x) for x in lines if x)
    print " ".join((line if line else "." * mw) for line in lines)
  print

words = []
for line in sys.stdin:
  if re.match(r"^\d+:$", line):
    if words:
      draw_words(words)
    words = []
  if re.match(r"^\s.*of \d+\)$", line) is not None:
    words.append(line)
if words:    
  draw_words(words)
