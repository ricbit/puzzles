import sys
import itertools

if len(sys.argv) < 3:
  print >> sys.stderr, "Usage: python wordcross.py height width < words.txt > words.dlx"
  sys.exit(1)

sizeh = int(sys.argv[1])
sizew = int(sys.argv[2])

def iter_directions():
  yield (0, 1)
  yield (1, 0)

def iter_positions():
  for j, i in itertools.product(range(sizeh), range(sizew)):
    yield j, i

def iter_vector(word):
  for jj, ii in iter_directions():
    for j, i in iter_positions():
      nj = j + jj * (len(word) - 1)
      ni = i + ii * (len(word) - 1)
      if 0 <= nj < sizeh and 0 <= ni < sizew:
        yield (j, i, jj, ii)

def encode(j):
  return chr(ord('a') + j)

def encode_used_pos(pj, pi, value):
  return "u%s%s:%d" % (encode(pj), encode(pi), value)

def encode_word_pos(nw, pj, pi, value):
  return "u%s%s%s:%d" % (encode(nw), encode(pj), encode(pi), value)

def option_word(nw, word, j, i, jj, ii, word_letters):
  option = ["#W%d" % nw]
  pos = set()
  for k, letter in enumerate(word):
    pj = j + k * jj
    pi = i + k * ii
    option.append("p%d_%d:%s" % (pj, pi, letter))
    option.append("wj%d_%d:%s" % (nw, k, encode(pj)))
    option.append("wi%d_%d:%s" % (nw, k, encode(pi)))
    option.append(encode_used_pos(pj, pi, 1))
    pos.add((pj, pi))
    word_letters[nw][k].add((pj, pi))
  for j, i in iter_positions():
    option.append(encode_word_pos(nw, j, i, int((j, i) in pos)))
  return " ".join(option)  

def encode_bigram(*vec):
  return "B%s" % "".join(map(encode, vec))

def option_bigram(aj, ai, bj, bi, words):
  for ea, eb in [(0, 0), (0, 1), (1, 0)]:
    yield "%s %s %s" % (
      encode_bigram(aj, ai, bj, bi),
      encode_used_pos(aj, ai, ea),
      encode_used_pos(bj, bi, eb))
  for nw, word in enumerate(words):
    option = [encode_bigram(aj, ai, bj, bi)]
    option.append(encode_word_pos(nw, aj, ai, 1))
    option.append(encode_word_pos(nw, bj, bi, 1))
    yield " ".join(option)

def collect_words(words):
  word_letters = {}
  for nw, word in enumerate(words):
    word_letters[nw] = {i : set() for i in xrange(len(word))}
  for nw, word in enumerate(words):
    for j, i, jj, ii in iter_vector(word):
      if sizeh != sizew or nw > 0 or ii > 0:
        yield option_word(nw, word, j, i, jj, ii, word_letters)
  for nw, word in enumerate(words):
    for nw2, word2 in enumerate(words):
      if nw != nw2:
        for k1, k2 in itertools.product(range(len(word)), range(len(word2))):
          if word[k1] != word2[k2]:
            continue
          for p1, p2 in itertools.product(word_letters[nw][k1], word_letters[nw2][k2]):
            if p1 != p2:
              continue
            j, i = p1  
            yield "WM%d wj%d_%d:%s wi%d_%d:%s wj%d_%d:%s wi%d_%d:%s " % (
              nw, nw, k1, encode(j), nw, k1, encode(i),
              nw2, k2, encode(j), nw2, k2, encode(i))

def collect_bigrams(words):
  for j, i in iter_positions():
    if i > 0:
      for bigram in option_bigram(j, i - 1, j, i, words):
        yield bigram
    if j > 0:
      for bigram in option_bigram(j - 1, i, j, i, words):
        yield bigram
    for nw, word in enumerate(words):
      yield "P%d_%d %s %s" % (j, i, encode_used_pos(j, i, 1), encode_word_pos(nw, j, i, 1))
    option = ["P%d_%d" % (j, i), encode_used_pos(j, i, 0)]
    for nw, word in enumerate(words):
      option.append(encode_word_pos(nw, j, i, 0))
    yield " ".join(option)

def extract_primary(option):
  for item in option.split():
    if ":" not in item and (item[0].isupper() or item[0] == "#"):
      yield item

def extract_secondary(option):
  for item in option.split():
    if ":" in item or item[0].islower():
      yield item.split(":")[0]

def main():
  nwords = int(raw_input())
  words = [raw_input().strip() for _ in xrange(nwords)]
  words.sort(key=lambda x: len(x), reverse=True)
  if any(len(word) > max(sizeh, sizew) for word in words):
    print >> sys.stderr, "Invalid grid"
    return
  options = list(collect_words(words))
  options.extend(collect_bigrams(words))
  primary = set()
  secondary = set()
  for option in options:
    for item in extract_primary(option):
      primary.add(item)
    for item in extract_secondary(option):
      secondary.add(item)
  print "%s | %s" % (" ".join(primary), " ".join(secondary))
  for option in options:
    print option

main()
