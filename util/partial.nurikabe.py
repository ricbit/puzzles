import sys
import re

def main():
  lines = sys.stdin.readlines()
  for line in lines:
    match = re.search(r"_W(\d+)", line)
    if match:
      h = int(match.group(1))
    match = re.search(r"_H(\d+)", line)
    if match:
      w = int(match.group(1))
  grid = [["."] * w for _ in range(h)]
  tree = [["."] * w for _ in range(h)]
  for line in lines:
    match = re.match(r"L\d+: (.*?) \(\d+ of (\d+)\)", line)
    if match:
      #if match.group(2) != "1":
      #  break
      for item in match.group(1).split():
        match = re.match(r"p(\d\d)(\d\d):(\d)", item)
        if match:
          i = int(match.group(1))
          j = int(match.group(2))
          v = int(match.group(3))
          if ord(grid[j][i]) < ord('A') or grid[j][i] == ".":
            grid[j][i] = str(v)
        match = re.match(r"^g(\d\d)(\d\d):(.)", item)
        if match:
          i = int(match.group(1))
          j = int(match.group(2))
          v = match.group(3)
          if ord(grid[j][i]) < ord('A') or grid[j][i] == ".":
            grid[j][i] = v
        match = re.match(r"^t(.)(\d\d)(\d\d):(.)", item)
        if match:
          g = match.group(1)
          i = int(match.group(2))
          j = int(match.group(3))
          v = match.group(4)
          if ord(tree[j][i]) < ord('A') or tree[j][i] == ".":
            tree[j][i] = v
      print(line)
      print("%s\n" % "\n".join("".join(str(i) for i in line) for line in grid))
      print("%s\n" % "\n".join("".join(str(i) for i in line) for line in tree))
  print("\nfinal\n")          
  print("%s\n" % "\n".join("".join(str(i) for i in line) for line in grid))
  print("%s\n" % "\n".join("".join(str(i) for i in line) for line in tree))

main()
