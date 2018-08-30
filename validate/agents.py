# Usage: python validate/agents.py input.txt solution.txt output.dot
# To generate the plot: neato -n -T png output.dot -o output.png
# Format of the solution.txt, containing allocation of agents to visits:
#   0 1
#   1 3
#   2 5
# etc where the first number is the visit number, the second number is the agent number.

import sys

# Read input
f = open(sys.argv[1], "rt")
nagents = int(f.readline())
agents = []
for _ in xrange(nagents):
  x, y = map(float, f.readline().split())
  agents.append((x, y))
nvisits = int(f.readline())
visits = []
for _ in xrange(nvisits):
  slot, x, y, pool = [a(b) for a,b in zip([int, float, float, int], f.readline().split())]
  visits.append((slot, x, y, pool))
f.close()

# Read solution
f = open(sys.argv[2], "rt")
solution = {}
for line in f.readlines():
  splitline = line.split()
  if len(splitline) == 2:
    visit, agent = map(int, splitline)
    solution[visit] = agent
f.close()

# Validate 95% constraint
totalpool = sum(v[3] for v in visits)
allocpool = sum(v[3] for i,v in enumerate(visits) if i in solution)
if allocpool * 100 >= totalpool * 95:
  print "More than 95% allocated"
else:
  print "Constraint violation: less than 95% allocated"

# Calculate total distance
def distance(v1, v2):
  dx = v1[1] - v2[1]
  dy = v1[2] - v2[2]
  return (dx ** 2 + dy ** 2) ** 0.5

totaldist = 0.0
allnodes = []
for a, agent in enumerate(agents):
  allocvisit = [visits[v] for v in solution if solution[v] == a]
  agentslots = [v[0] for v in allocvisit]
  allocvisit.append((-1, agent[0], agent[1], 0))
  allocvisit.append((max(agentslots) + 1, agent[0], agent[1], 0))
  allocvisit.sort()
  agentslots = [v[0] for v in allocvisit]
  if len(agentslots) != len(set(agentslots)):
    print "Constraint violation: agent is in two places at the same time"
  dist = sum(distance(a, b) for a,b in zip(allocvisit, allocvisit[1:]))
  totaldist += dist
  allnodes.append(allocvisit[:-1])
print "Total distance = ", totaldist

# Write dot file
f = open(sys.argv[3], "wt")
f.write("digraph agents {\n")
SCALE = 10
for a, agentnodes in enumerate(allnodes):
  c = '"%f,1,0.7"' % (float(a) / nagents)
  for i, nodes in enumerate(agentnodes):
    f.write(('a%d%d [pos="%f,%f!", color=%s, label="", style=filled,' +
             'fixedsize=true, fillcolor=%s, %s];\n') %
        (a, i, nodes[1] * SCALE, nodes[2] * SCALE, c, c,
         ("width=1, height=1" if i == 0 else "width=0.3, height=0.3")))
  for i, nodes in enumerate(agentnodes[:-1]):
    f.write('a%d%d -> a%d%d [color=%s];\n' % (a, i, a, i + 1, c))
  f.write('a%d%d -> a%d%d [color=%s];\n' % (a, len(agentnodes) - 1, a, 0, c))
f.write("}")
f.close()

