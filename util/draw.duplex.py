import sys
import re
import itertools

def parse_words(lines, config, source):
  ans = {0:[0, {}], 1:[0, {}]}
  maxword = 0
  for line in lines:
    match = re.search(r"W(\d+)_(\d+)", line)
    if match:
      a = int(match.group(1))
      p = int(match.group(2))
      pos = r"w%d%d%%d:(\d)" % (a, p)
      word = int("".join(re.search(pos % i, line).group(1) for i in range(4)))
      ans[a][1][p] = source[a][word]
      ans[a][0] = int(re.search(r"a\d_\d:(\d)", line).group(1))
      maxword = max(word, maxword)
  return (maxword, ans)

def parse_config():
  f = open(sys.argv[1])
  n = int(f.readline().strip())
  config = {"size": []}
  for _ in range(n):
    config["size"].append(map(int, f.readline().split()))
  config["author"] = f.readline().strip()
  config["quote"] = f.readline().strip()
  return config

def draw_coords(config, key, pos):
  out = []
  for n, q in enumerate(config["quote"]):
    if n in key:
      out.append(key[n][pos])
    else:
      out.append(" ")
  return "".join(out)

def draw_grid(config, solution):
  ans = []
  for a, (n, w) in enumerate(config["size"]):
    pos = solution[a][0]
    for i in range(n):
      word = list(solution[a][1][i])
      word[pos] = word[pos].upper()
      ans.append("".join(word))
  print("\n  %s" % "".join("%d" % (i + 1) for i in range(max(len(x) for x in ans))))
  letters = []
  for n, word in enumerate(ans):
    j = chr(ord('A') + n) 
    print("%s %s" % (j, word))
    letters.extend((w, j, i) for i, w in enumerate(word) if w.islower())
  print()
  return ans, letters

def draw_solution(config, maxword, solution):
  print(maxword)
  ans, letters = draw_grid(config, solution)
  letters.sort()
  quote = [(c, n) for n, c in enumerate(config["quote"])]
  quote.sort()
  quote = quote[config["quote"].count(" "):]
  key = {n: (j, str(i + 1)) for ((w, j, i), (c, n)) in zip(letters,quote)}
  print(draw_coords(config, key, 0))   
  print(draw_coords(config, key, 1))
  print(config["quote"])
  print()

def main():
  config = parse_config()
  source = []
  for n, size in config["size"]:
    source.append(open("words/words%d-from-OSPD4" % size).read().split())
  words = []
  solutions = []
  for line in sys.stdin:
    if re.match(r"^\d+:$", line):
      if words:
        solutions.append(parse_words(words, config, source))
      words = []
    if re.match(r"^\s.*of \d+\)$", line) is not None:
      words.append(line)
  if words:    
    solutions.append(parse_words(words, config, source))
  solutions.sort()
  for maxword, solution in solutions:    
    draw_solution(config, maxword, solution)

main()
