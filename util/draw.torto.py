# Read output from dlx2 and draw solutions as Torto grids.
# Usage: python draw.torto.py --[short|long] --[ricbit|knuth] < torto.out.txt > torto.readable.txt

import sys
import re
import argparse

def write(torto, long_format):
  if all("".join(line) == "---" for line in torto):
    return
  if long_format:
    print "%s\n" % "\n".join("".join(line) for line in torto)
  else:
    print "".join("".join(line) for line in torto)

parser = argparse.ArgumentParser(
    description="Read output from dlx2 and draw solutions as Torto grids.")
encoding = parser.add_mutually_exclusive_group()
encoding.add_argument("--ricbit", action="store_false",
    help="Use Ricbit's encoding")
encoding.add_argument("--knuth", action="store_true",
    help="Use Knuth's encoding")
grid_format = parser.add_mutually_exclusive_group()
grid_format.add_argument("--short", action="store_false",
    help="Output solutions in short format")
grid_format.add_argument("--long", action="store_true",
    help="Output solutions in long format")
parser.add_argument("-p", "--progression", nargs="?", type=int, const=1000, default=None,
    help="Show progression status during processing")
args = parser.parse_args()

startsol = re.compile(r"^\d+:$")
findpos = re.compile(r"p(\d)(\d):(\w)") if args.ricbit else re.compile(r"([a-r]):(\w)")
torto = [["-"] * 3 for i in xrange(6)]
count = 0
for line in sys.stdin:
  if startsol.match(line):
    write(torto, args.long)
    torto = [["-"] * 3 for i in xrange(6)]
    count += 1
    if args.progression and count % args.progression == 0:
      print >> sys.stderr, "Processed %d solutions." % count
  for item in line.split():
    m = findpos.match(item)
    if m is not None:
      if args.ricbit:
        j, i, c = [m.group(i) for i in xrange(1, 4)]
        torto[int(j)][int(i)] = c
      else:
        pos, c = [m.group(i) for i in xrange(1, 3)]
        pos = ord(pos) - ord('a')
        torto[pos / 3][pos % 3] = c

write(torto, args.long)
