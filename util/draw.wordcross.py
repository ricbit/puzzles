import sys
import re
import itertools

def draw_words_r(maxj, maxi, words, regexp):
  pos = []
  for word in words:
    for match in re.findall(regexp, word):
      pos.append(match)
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

def draw_words_u(maxj, maxi, words, regexp):
  pos = []
  for word in words:
    for match in re.findall(regexp, word):
      pos.append(match)
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

def draw_words_s(maxj, maxi, words, regexp):
  pos = []
  for word in words:
    for match in re.findall(regexp, word):
      pos.append(match)
  grid = [["."] * maxi for _ in xrange(maxj)]
  for j, i, c in pos:
    jj = decode(j)
    ii = decode(i)
    grid[jj][ii] = c
  for line in grid:
    yield "".join(line)

def find_dim(lines):
  pos = set()
  for line in lines:
    for match in re.findall(r"P(\d+)_(\d+)", line):
      pos.add(match)
  h = max(p[0] for p in pos)
  w = max(p[1] for p in pos)
  return 1 + int(h), 1 + int(w)

def draw_words(words):
  h, w = find_dim(words)
  grid = draw_words_r(h, w, words, r"p(\d+)_(\d+):(\w)")
  word = draw_words_u(h, w, words, r"u(\w)(\w)(\w):(\w)")
  used = draw_words_s(h, w, words, r"u(\w)(\w):(\w)")
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
