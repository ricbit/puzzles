import sys
import unidecode

words = set()
for line in sys.stdin:
  lwords = [unidecode.unidecode(unicode(w, 'utf-8')) for w in line.strip().split(",")]
  words.update(lwords)
sorted_words = [w.strip() for w in sorted(words, key=lambda x:(len(x),x))]
print "\n".join(w for w in sorted_words if len(w) >= 3)
  


