#include <iostream>
#include "easyscip/easyscip.h"

using namespace std;
using namespace easyscip;


bool valid(int i, int j, int w, int h) {
  return i >= 0 && i < w && j >= 0 && j < h;
}

int main() {
  int w,h;
  cin >> w >> h;
  vector<string> board(h);
  for (int i = 0; i < h; i++) {
    cin >> board[i];
  }
  bool silent = true;
  MIPSolver mip(silent);
  vector<vector<Variable>> lamp(h);
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      lamp[j].push_back(mip.binary_variable(1));
    }
  }
  // Enforce restriction around numbers.
  static int dx[] = {1, -1, 0, 0};
  static int dy[] = {0, 0, 1, -1};
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      if (board[j][i] >= '0' && board[j][i] <= '4') {
        auto cons = mip.constraint();
        for (int k = 0; k < 4; k++) {
          int jj = j + dy[k], ii = i + dx[k]; 
          if (valid(ii, jj, w, h) && board[jj][ii] == '.') {
            cons.add_variable(lamp[jj][ii], 1);
          }
        }
        int value = board[j][i] - '0';
        cons.commit(value, value);
      }
    }
  }
  // Every free position must be illuminated,
  // but no lamp can be illuminated by another lamp.
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      if (board[j][i] == '.') {
        auto cons = mip.constraint();
        for (int k = 0; k < 4; k++) {
          int jj = j + dy[k], ii = i + dx[k]; 
          while (valid(ii, jj, w, h) && board[jj][ii] == '.') {
            cons.add_variable(lamp[jj][ii], 1);
            ii += dx[k];
            jj += dy[k];
          }
        }
        cons.add_variable(lamp[j][i], w * h);
        cons.commit(1, w * h);
      }
    }
  }
  // Solve and print.
  auto sol = mip.solve();

  const string wall = "⬛";
  const string ulamp = "💡";
  const string lit  = "  ";
  const string floor = "⬜";
  const string digits[] = {"０", "１", "２", "３", "４"};

  // Top border
  cout << "┌";
  for (int i = 0; i < w - 1; ++i) cout << "──┬";
  cout << "──┐\n";

  for (int y = 0; y < h; ++y) {
      cout << "│";
      for (int x = 0; x < w; ++x) {
          char c = board[y][x];
          string cell;
          if (c == '.') {
              cell = sol.value(lamp[y][x]) > 0.5 ? ulamp : lit;
          } else if (c == 'X') {
              cell = wall;
          } else if (c >= '0' && c <= '4') {
              cell = digits[c - '0'];
          } else {
              cell = "？";
          }
          cout << cell << "│";
      }
      cout << '\n';

      // Row divider or bottom border
      if (y < h - 1) {
          cout << "├";
          for (int i = 0; i < w - 1; ++i) cout << "──┼";
          cout << "──┤\n";
      } else {
          cout << "└";
          for (int i = 0; i < w - 1; ++i) cout << "──┴";
          cout << "──┘\n";
      }
  }

}
