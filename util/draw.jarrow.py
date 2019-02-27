import sys
import re
import itertools

def decode(x):
  return ord(x) - ord('a')

def get_size(lines):
  size = 0
  for line in lines:
    for match in re.findall(r'E(\w)\w\d', line):
      size = max(size, decode(match))
  return size + 1

def draw_words_u(words):
  size = get_size(words)
  pos = [['.'] * size for _ in xrange(size)]
  for word in words:
    match = re.search("(?<!\w)c(\w)(\w):(\w)", word)
    if match:
      pos[decode(match.group(1))][decode(match.group(2))] = str(decode(match.group(3)))
  return pos

def draw_words(words):
  for line in draw_words_u(words):
    print "".join(line)
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
