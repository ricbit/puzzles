#include <iostream>
#include "easyscip/easyscip.h"

using namespace std;
using namespace easyscip;

bool valid(int i, int j, int w, int h) {
  return i >= 0 && i < w && j >= 0 && j < h;
}

struct Group {
  int size, j, i;
};

int main() {
  int w, h, dummy;
  cin >> w >> h >> dummy;
  vector<string> board(h);
  for (int i = 0; i < h; i++) {
    cin >> board[i];
  }
  MIPSolver mip;
  vector<Group> groups;
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      if (board[j][i] != '.') {
        groups.push_back(Group{board[j][i] - '0', j, i});
      }
    }
  }
  int gs = groups.size();
  vector<vector<vector<Variable>>> grid(h, vector<vector<Variable>>(w));
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      for (int g = 0; g < gs; g++) {
        if (i != groups[g].i && j != groups[g].j) {
          grid[j][i].push_back(NullVariable());
        } else {
          grid[j][i].push_back(mip.binary_variable(abs(j - groups[g].j) + abs(i - groups[g].i)));
        }
      }
    }
  }
  // If the cell has a number, it must be zero.
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      if (board[j][i] != '.') {
        for (int g = 0; g < gs; g++) {
          auto cons = mip.constraint();
          cons.add_variable(grid[j][i][g], 1);
          cons.commit(0, 0);
        }
      }
    }
  }
  // If not on the same row or column as the seed, then it must be zero.
  static int dx[] = {1, -1, 0, 0};
  static int dy[] = {0, 0, 1, -1};
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      for (int g = 0; g < gs; g++) {
        if (j != groups[g].j && i != groups[g].i) {
          auto cons = mip.constraint();
          cons.add_variable(grid[j][i][g], 1);
          cons.commit(0, 0);
        }
      }
    }
  }
  // Every free cell must have exactly one branch.
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      if (board[j][i] != '.') {
        continue;
      }
      auto cons = mip.constraint();
      for (int g = 0; g < gs; g++) {
        for (int p = 1; p <= groups[g].size; p++) {
          for (int d = 0; d < 4; d++) {
            int jj = groups[g].j + dy[d] * p;
            int ii = groups[g].i + dx[d] * p;
            if (valid(ii, jj, w, h)) {
              int jn = groups[g].j + dy[d];
              int in = groups[g].i + dx[d];
              int count = 0;
              for (int j2 = min(jn, jj); j2 <= max(jn, jj); j2++) {
                for (int i2 = min(in, ii); i2 <= max(in, ii); i2++) {
                  if (board[j2][i2] != '.') {
                    count++;
                  }
                }
              }
              if (count > 0) {
                continue;
              }
              if ((i == ii && j >= min(jn, jj) && j <= max(jn, jj)) ||
                  (j == jj && i >= min(in, ii) && i <= max(in, ii))) {
                cons.add_variable(grid[jj][ii][g], 1);
              }
            }
          }
        }
      }
      cons.commit(1, 1);
    }
  }
  // Number of branches for each group must be equal to seed number.
  for (int g = 0; g < gs; g++) {
    auto cons = mip.constraint();
    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        cons.add_variable(grid[j][i][g],
          abs(j - groups[g].j) + abs(i - groups[g].i));
      }
    }
    cons.commit(groups[g].size, groups[g].size);
  }
  // Solve and print.
  vector<vector<char>> out(h, vector<char>(w, '.'));
  auto sol = mip.solve();
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      for (int g = 0; g < gs; g++) {
        if (sol.value(grid[j][i][g]) < 0.5) {
          continue;
        }
        int ii = groups[g].i;
        int jj = groups[g].j;
        for (int xi = min(i, ii); xi <= max(i, ii); xi++) {
          for (int xj = min(j, jj); xj <= max(j, jj); xj++) {
            out[xj][xi] = xi == ii ? '|' : '-';
          }
        }
        out[j][i] = i < ii ? '<' : (
                    i > ii ? '>' : (
                    j < jj ? '^' : 'v'));
      }
    }
  }
  for (int g = 0; g < gs; g++) {
    out[groups[g].j][groups[g].i] = '0' + groups[g].size;
  }
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      cout << out[j][i];
    }
    cout << "\n";
  }
}
