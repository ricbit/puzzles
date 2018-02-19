#include <iostream>
#include <string>
#include <vector>
#include "easyscip/easyscip.h"

using namespace std;
using namespace easyscip;

bool valid(int i, int j, int w, int h) {
  return i >= 0 && i < w && j >= 0 && j < h;
}

int main() {
  int h, w, gs;
  cin >> h >> w >> gs;
  vector<vector<int>> given(h, vector<int>(w, -1));
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      string g;
      cin >> g;
      if (g != "-") {
        given[j][i] = stoi(g) - 1;
      }
    }
  }
  vector<vector<int>> group(h, vector<int>(w, 0));
  vector<int> group_size(gs, 0);
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      string g;
      cin >> g;
      group[j][i] = stoi(g) - 1;
      group_size[stoi(g) - 1]++;
    }
  }
  // Add variables.
  MIPSolver mip;
  vector<vector<vector<Variable>>> board(w, vector<vector<Variable>>(h));
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      for (int g = 0; g < group_size[group[j][i]]; g++) {
        board[j][i].push_back(mip.binary_variable(0));
      }
    }
  }
  // Add the givens.
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      if (given[j][i] >= 0) {
        auto cons = mip.constraint();
        cons.add_variable(board[j][i][given[j][i]], 1);
        cons.commit(1, 1);
      }
    }
  }
  // A position must be filled with only one number.
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      auto cons = mip.constraint();
      for (int g = 0; g < group_size[group[j][i]]; g++) {
        cons.add_variable(board[j][i][g], 1);
      }
      cons.commit(1, 1);
    }
  }
  // A number can't repeat on each group.
  for (int gn = 0; gn < gs; gn++) {
    for (int g = 0; g < group_size[gn]; g++) {
      auto cons = mip.constraint();
      for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
          if (group[j][i] == gn) {
            cons.add_variable(board[j][i][g], 1);
          }
        }
      }
      cons.commit(1, 1);
    }
  }
  // A number must not touch the same number.
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      for (int g = 0; g < group_size[group[j][i]]; g++) {
        auto cons = mip.constraint();
        int size = 0;
        for (int jj = -1; jj <= 1; jj++) {
          for (int ii = -1; ii <= 1; ii++) {
            if (ii == 0 && jj == 0) {
              continue;
            }
            if (valid(i + ii, j + jj, w, h) && 
                g < group_size[group[jj + j][ii + i]]) {
              cons.add_variable(board[j + jj][i + ii][g], 1);
              size++;
            }
          }
        }
        cons.add_variable(board[j][i][g], size);
        cons.commit(0, size);
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
