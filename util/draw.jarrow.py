import sys
import re
import itertools

def decode(x):
  return ord(x) - ord('a')

def draw_words_u(words):
  pos = [['.'] * 10 for _ in xrange(10)]
  for word in words:
    match = re.search("(?<!\w)c(\w)(\w):(\w)", word)
    if match:
      pos[decode(match.group(1))][decode(match.group(2))] = decode(match.group(3))
  return pos

def draw_words(words):
  for line in draw_words_u(words):
    print line
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
