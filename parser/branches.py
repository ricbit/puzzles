import itertools
import requests

base_url = "https://www.janko.at/Raetsel/Vier-Winde/%03d.a.htm"

def convert(x):
  if x == "-":
    return "."
  x = int(x)
  if x < 10:
    return str(x)
  else:
    return chr(x - 10 + ord('A'))

for i in xrange(1, 161):
  html = requests.get(base_url % i).text.splitlines()
  problem = html[html.index('problem') + 1 : html.index('solution')]
  print i
  f = open('data/branches.janko.%d.txt' % i, 'wt')
  n = len(problem)
  g = 0
  out = []
  for line in problem:
    s = line.strip().split()
    g += sum(1 for x in s if x != "-")
    out.append("".join(convert(x) for x in s))
  f.write('%d %d %d\n' % (n, n, g))
  f.write('\n'.join(out))
  f.close()
