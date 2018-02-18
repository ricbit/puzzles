import itertools
import requests

base_url = "https://www.janko.at/Raetsel/Suguru/%03d.a.htm"

for i in xrange(1, 51):
  html = requests.get(base_url % i).text.splitlines()
  problem = html[html.index('problem') + 1 : html.index('areas')]
  areas = html[html.index('areas') + 1 : html.index('solution')]
  print i
  f = open('suguru.%d.txt' % i, 'wt')
  f.write('%d %d %d\n' % (
    len(problem),
    len(problem[0].split()), 
    max(int(x) for x in ' '.join(areas).split())))
  f.write('\n'.join(problem))
  f.write('\n')
  f.write('\n'.join(areas))
  f.write('\n')
  f.close()
