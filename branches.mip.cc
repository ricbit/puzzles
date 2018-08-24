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

template<typename T>
bool valid_block(T &board, int jn, int jj, int in, int ii) {
  int count = 0;
  for (int j2 = min(jn, jj); j2 <= max(jn, jj); j2++) {
    for (int i2 = min(in, ii); i2 <= max(in, ii); i2++) {
      if (board[j2][i2] != '.') {
        count++;
      }
    }
  }
  return count == 1;
}

int distance(int j, int i, Group &g) {
  return abs(j - g.j) + abs(i - g.i);
}

template<typename T>
bool valid_tile(T &board, int j, int i, Group &g, int w, int h) {
  if ((i != g.i && j != g.j) ||
      (i == g.i && j == g.j) ||
      !valid(i, j, w, h) ||
      !valid_block(board, g.j, j, g.i, i)) {
    return false;
  }
  return distance(j, i, g) <= g.size;
}

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
        int g = board[j][i] <= '9' ?
            board[j][i] - '0' : board[j][i] - 'A' + 10;
        groups.push_back(Group{g, j, i});
      }
    }
  }
  int gs = groups.size();
  vector<vector<vector<Variable>>> grid(h, vector<vector<Variable>>(w));
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      for (int g = 0; g < gs; g++) {
        if (!valid_tile(board, j, i, groups[g], w, h)) {
          grid[j][i].push_back(NullVariable());
        } else {
          grid[j][i].push_back(mip.binary_variable(distance(j, i, groups[g])));
        }
      }
    }
  }
  // Every free cell must have exactly one branch.
  static int dx[] = {1, -1, 0, 0};
  static int dy[] = {0, 0, 1, -1};
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
            if (valid_tile(board, jj, ii, groups[g], w, h)) {
              int jn = groups[g].j + dy[d];
              int in = groups[g].i + dx[d];
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
        cons.add_variable(grid[j][i][g], distance(j, i, groups[g]));
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
    out[groups[g].j][groups[g].i] = groups[g].size > 9 ?
      'A' + groups[g].size - 10 : '0' + groups[g].size;
  }
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      cout << out[j][i];
    }
    cout << "\n";
  }
}
