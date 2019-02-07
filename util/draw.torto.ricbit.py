# Read output from dlx2 and draw solutions as Torto grids.
# Usage: python draw.torto.py (short|long) < torto.out.txt > torto.readable.txt

import sys
import re

def write(torto):
  if all("".join(line) == "---" for line in torto):
    return
  if sys.argv[1] == "long":
    print "%s\n" % "\n".join("".join(line) for line in torto)
  else:
    print "".join("".join(line) for line in torto)

startsol = re.compile(r"^\d+:$")
findpos = re.compile(r"p(\d)(\d):(\w)")
torto = [["-"] * 3 for i in xrange(6)]
count = 0
if len(sys.argv) < 2:
  print >> sys.stderr, "Missing short|long"
  sys.exit(1)
for line in sys.stdin:
  if startsol.match(line):
    write(torto)
    torto = [["-"] * 3 for i in xrange(6)]
    count += 1
    if count % 1000 == 0:
      print >> sys.stderr, count
  for item in line.split():
    m = findpos.match(item)
    if m is not None:
      j, i, c = [m.group(i) for i in xrange(1, 4)]
      torto[int(j)][int(i)] = c
write(torto)      
