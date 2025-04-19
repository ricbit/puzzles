#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "easyscip/easyscip.h"
#include "printers/printers.h"

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
  bool silent = true;
  MIPSolver mip(silent);
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

  // Convert solution to grid format for GroupPrinter
  vector<vector<int>> solution(h, vector<int>(w));
  vector<vector<char>> groups(h, vector<char>(w));
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      // Store group information
      groups[j][i] = 'a' + group[j][i];
      // Find the number in this cell (1-based)
      for (int g = 0; g < group_size[group[j][i]]; g++) {
        if (sol.value(board[j][i][g]) > 0.5) {
          // Add 10 to given numbers to mark them differently
          solution[j][i] = given[j][i] >= 0 ? (g + 11) : (g + 1);
          break;
        }
      }
    }
  }

  // Set up cell symbols with colors for givens
  map<int, string> cell_symbols;
  const char* wide_numbers[] = {"１", "２", "３", "４", "５"};
  for (int i = 1; i <= 5; i++) {
    cell_symbols[i] = wide_numbers[i-1];  // Normal numbers
    cell_symbols[i + 10] = "\033[34m" + string(wide_numbers[i-1]) + "\033[0m";  // Blue for givens
  }

  // Create and use group printer
  GroupPrinter printer(h, w, groups, cell_symbols, 2);
  printer.print(solution);
}
