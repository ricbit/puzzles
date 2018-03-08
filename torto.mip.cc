#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "easyscip/easyscip.h"

using namespace std;
using namespace easyscip;

bool valid(int i, int j, int w, int h) {
  return i >= 0 && i < w && j >= 0 && j < h;
}

int distance(int a, int b) {
  int ia = a % 3, ja = a / 3;
  int ib = b % 3, jb = b / 3;
  return abs(ia - ib) + abs(ja - jb);
}

bool neighbour(int a, int b) {
  int ia = a % 3, ja = a / 3;
  int ib = b % 3, jb = b / 3;
  return abs(ia - ib) <= 1 && abs(ja - jb) <= 1 && a != b;
}

int len(string& s) {
  return static_cast<int>(s.size());
}

template<typename T>
int len(vector<T> vec) {
  return static_cast<int>(vec.size());
}

struct Position {
  int word, start;
};

struct Bigram {
  Position a, b;
};

int main() {
  int nwords;
  cin >> nwords;
  vector<string> words(nwords);
  map<char, int> hist;
  for (int i = 0; i < nwords; i++) {
    cin >> words[i];
    for (int j = 0; j < len(words[i]); j++) {
      int total = 0;
      for (int k = 0; k < len(words[i]); k++) {
        if (words[i][k] == words[i][j]) {
          total++;
        }
      }
      hist[i] = max(hist[i], total);
    }
  }

  // Add variables.
  MIPSolver mip;

  int npos = 18;
  vector<vector<vector<Variable>>> grid(
      npos, vector<vector<Variable>>(nwords));
  for (int p = 0; p < npos; p++) {
    for (int w = 0; w < nwords; w++) {
      for (int c = 0; c < len(words[w]); c++) {
        grid[p][w].push_back(mip.binary_variable(0));
      }
    }
  }

  vector<Variable> bigramvar;
  vector<Bigram> bigrams;
  for (int w = 0; w < nwords; w++) {
    for (int c = 0; c < len(words[w]) - 1; c++) {
      for (int ww = 0; ww < nwords; ww++) {
        if (w == ww) continue;
        for (int cc = 0; cc < len(words[ww]) - 1; cc++) {
          if (c == cc) continue;
          if (words[w][c] == words[ww][cc] && 
              words[w][c + 1] == words[ww][cc + 1]) {
            bigramvar.push_back(mip.binary_variable(-1));
            bigrams.push_back(Bigram{Position{w, c}, Position{ww,cc}});
          }
        }
      }
    }
  }

  // Enforce bigrams.
  for (int i = 0; i < len(bigrams); i++) {
    for (int p = 0; p < npos; p++) {
      auto& b = bigrams[i];
      auto cons1 = mip.constraint();
      cons1.add_variable(bigramvar[i], 1);
      cons1.add_variable(grid[p][b.a.word][b.a.start], 1);
      for (int pp = 0; pp < npos; pp++) {
        if (p == pp) continue;
        cons1.add_variable(grid[pp][b.b.word][b.b.start], 1);
      }
      cons1.commit(0, 2);
      auto cons2 = mip.constraint();
      cons2.add_variable(bigramvar[i], 1);
      cons2.add_variable(grid[p][b.a.word][b.a.start + 1], 1);
      for (int pp = 0; pp < npos; pp++) {
        if (p == pp) continue;
        cons2.add_variable(grid[pp][b.b.word][b.b.start + 1], 1);
      }
      cons2.commit(0, 2);
    }
  }

  // Only one letter for grid position.
  for (int p = 0; p < npos; p++) {
    for (int w = 0; w < nwords; w++) {
      for (int c = 0; c < len(words[w]); c++) {
        auto cons = mip.constraint();
        int vars = 0;
        for (int ww = 0; ww < nwords; ww++) {
          for (int cc = 0; cc < len(words[ww]); cc++) {
            if (words[w][c] == words[ww][cc]) {
              continue;
            }
            cons.add_variable(grid[p][ww][cc], 1);
            vars++;
          }
        }
        cons.add_variable(grid[p][w][c], vars);
        cons.commit(0, vars);
      }
    }
  }

  // Each pos must have at least one letter.
  for (int p = 0; p < npos; p++) {
    auto cons = mip.constraint();
    int limit = 0;
    for (int w = 0; w < nwords; w++) {
      for (int c = 0; c < len(words[w]); c++) {
        cons.add_variable(grid[p][w][c], 1);
        limit++;
      }
    }
    cons.commit(1, limit);
  }

  // Each letter must appear in exactly one pos.
  for (int w = 0; w < nwords; w++) {
    for (int c = 0; c < len(words[w]); c++) {
      auto cons = mip.constraint();
      for (int p = 0; p < npos; p++) {
        cons.add_variable(grid[p][w][c], 1);
      }
      cons.commit(1, 1);
    }
  }

  // Letters must form a sequence.
  for (int w = 0; w < nwords; w++) {
    for (int c = 0; c < len(words[w]) - 1; c++) {
      for (int p = 0; p < npos; p++) {
        auto cons = mip.constraint();
        cons.add_variable(grid[p][w][c], 1);
        for (int pp = 0; pp < npos; pp++) {
          if (neighbour(p, pp)) {
            cons.add_variable(grid[pp][w][c + 1], -1);
          }
        }
        cons.commit(-1, 0);
      }
    }
  }

  // A word must not use the same letter of the grid twice.
  for (int w = 0; w < nwords; w++) {
    for (int p = 0; p < npos; p++) {
      auto cons = mip.constraint();
      for (int c = 0; c < len(words[w]); c++) {
        cons.add_variable(grid[p][w][c], 1);
      }
      cons.commit(0, 1);
    }
  }

  // Solve and print.
  auto sol = mip.solve();
  for (int p = 0; p < npos; p++) {
    bool has = false;
    for (int w = 0; w < nwords; w++) {
      for (int c = 0; c < len(words[w]); c++) {
        if (sol.value(grid[p][w][c]) > 0.5 && !has) {
          cout << words[w][c] << " ";
          has = true;
        }
      }
    }
    if (p % 3 == 2) {
      cout << "\n";
    }
  }
  cout << "\n";
}
