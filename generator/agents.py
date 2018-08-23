# python agents.py num_agents slots

import math
import random
import sys

nagents = int(sys.argv[1])
slots = int(sys.argv[2])
R = 100.0

print nagents
for i in xrange(nagents):
  a = 2.0 * math.pi / nagents * i
  print R * math.cos(a), R * math.sin(a)

visits = slots * nagents
print visits
for t in xrange(slots):
  for _ in xrange(nagents):
    a = 2.0 * math.pi * random.random()
    r = R * math.sqrt(random.random())
    print t, r * math.cos(a), r * math.sin(a), random.randint(1, 4)
    
