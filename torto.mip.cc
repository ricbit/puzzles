#include <iostream>
#include <string>
#include <vector>
#include "easyscip/easyscip.h"

using namespace std;
using namespace easyscip;

bool valid(int i, int j, int w, int h) {
  return i >= 0 && i < w && j >= 0 && j < h;
}

int len(string& s) {
  return static_cast<int>(s.size());
}

int main() {
  int nwords;
  cin >> nwords;
  vector<string> words(nwords);
  for (int i = 0; i < nwords; i++) {
    cin >> words[i];
  }
  // Add variables.
  MIPSolver mip;

  int npos = 18;
  vector<vector<vector<Variable>>> board(
      npos, vector<vector<Variable>>(nwords));
  for (int p = 0; p < npos; p++) {
    for (int w = 0; w < nwords; w++) {
      for (int c = 0; c < len(words[w]); c++) {
        board[p][w].push_back(mip.binary_variable(0));
      }
    }
  }

  // Only one letter for grid position.
  for (int p = 0; p < npos; p++) {
    for (int w = 0; w < nwords; w++) {
      for (int c = 0; c < len(words[w]); c++) {
      }
    }
  }

  // Solve and print.
  auto sol = mip.solve();
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      cout << static_cast<char>('a' + group[j][i]);
      for (int g = 0; g < group_size[group[j][i]]; g++) {
        if (sol.value(board[j][i][g]) > 0.5) {
          cout << g + 1 << " ";
        }
      }
    }
    cout << "\n";
  }
  cout << "\n";
}
