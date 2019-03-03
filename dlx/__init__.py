
def encode(n):
  return chr(n + ord("a"))

def collect_primary(options):
  items = set()
  for option in options:
    for item in option.split():
      if item[0].isupper() or item[0] == "#":
        items.add(item)
  return items

def collect_secondary(options):
  items = set()
  for option in options:
    for item in option.split():
      if item[0].islower():
        items.add(item.split(":")[0])
  return items

def build_dlx(options):
  primary = collect_primary(options)
  secondary = collect_secondary(options)
  yield "%s | %s" % (" ".join(primary), " ".join(secondary))
  for option in options:
    yield option
