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

def iter_word(word, j, i, jj, ii):
  for k, letter in enumerate(word):
    pj = j + k * jj
    pi = i + k * ii
    yield (k, letter, pj, pi)

def iter_word_ident(nw, j, i, jj, ii):
  if jj == 1:
    yield "wvj%d:%s" % (nw, encode(j))
    yield "wvi%d:%s" % (nw, encode(i))
    yield "whj%d:-" % nw
    yield "whi%d:-" % nw
  else:
    yield "whj%d:%s" % (nw, encode(j))
    yield "whi%d:%s" % (nw, encode(i))
    yield "wvj%d:-" % nw
    yield "wvi%d:-" % nw

def option_word(nw, word, j, i, jj, ii):
  option = ["#W%d" % nw]
  option.extend(iter_word_ident(nw, j, i, jj, ii))
  pos = set()
  for k, letter in enumerate(word):
    pj = j + k * jj
    pi = i + k * ii
    option.append("p%d_%d:%s" % (pj, pi, letter))
    option.append(encode_used_pos(pj, pi, 1))
    option.append("%s%d_%d:%s" % (("v" if jj == 1 else "h"), pj, pi, encode(nw)))
    pos.add((pj, pi))
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

def gen_word_letters(words):
  word_letters = {}
  for nw, word in enumerate(words):
    for j, i, jj, ii in iter_vector(word):
      if sizeh != sizew or nw > 0 or ii > 0:
        for k, letter, pj, pi in iter_word(word, j, i, jj, ii):
          word_letters.setdefault((pj, pi, letter), set()).add(
            (nw, j, i, jj, ii))
  return word_letters

def iter_word_crossings(word_letters):
  for (pj, pi, letter), words in word_letters.iteritems():
    for word1, word2 in itertools.product(words, repeat=2):
      nw1, j1, i1, jj1, ii1 = word1
      nw2, j2, i2, jj2, ii2 = word2
      if nw1 == nw2 or ii1 == ii2:
        continue
      yield (word1, word2)

def collect_words(words):
  word_letters = gen_word_letters(words)
  possible_words = set()
  for word1, word2 in iter_word_crossings(word_letters):
    possible_words.add(word1)
    possible_words.add(word2)
  for nw, j, i, jj, ii in possible_words:
    yield option_word(nw, words[nw], j, i, jj, ii)
  for word1, word2 in iter_word_crossings(word_letters):
    nw1, j1, i1, jj1, ii1 = word1
    nw2, j2, i2, jj2, ii2 = word2
    option = ["#WM%d" % nw1]
    option.extend(iter_word_ident(nw1, j1, i1, jj1, ii1))
    option.extend(iter_word_ident(nw2, j2, i2, jj2, ii2))
    yield " ".join(option)

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
  words.sort(key=lambda x: len(x), reverse=False)
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
