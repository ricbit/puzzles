#include <iostream>
#include <array>
#include "exactcover/exactcover.h"

using namespace std;

bool valid(int j, int i, int w, int h) {
  return i >= 0 && i < w && j >= 0 && j < h;
}

struct Group {
  int size, j, i;
};

typedef array<int, 4> Partition;

void print(Partition p) {
  for (int d = 0; d < 4; d++) {
    cout << int(p[d]) << " ";
  }
  cout << "\n";
}

struct Row {
  unsigned short group;
  Partition partition;
};

template<typename T>
void iter_partitions(int size, T func) {
  for (int pn = 0; pn <= size; pn++) {
    for (int pe = 0; pe <= size - pn; pe++) {
      for (int ps = 0; ps <= size - pn - pe; ps++) {
        int pw = size - pn - pe - ps;
        Partition p{pn, pe, ps, pw};
        func(p);
      }
    }
  }
}

int dx[] = {0, 1, 0, -1};
int dy[] = {-1, 0, 1, 0};

template<typename T>
void iter_tile(vector<string> &board, Group &g, int w, int h, Partition &p, T func) {
  for (int d = 0; d < 4; d++) {
    for (int i = 1; i <= p[d]; i++) {
      func(g.j + dy[d] * i, g.i + dx[d] * i, i == p[d]);
    }
  }
}

bool valid_tile(vector<string> &board, Group &g, int w, int h, Partition &p) {
  bool ans = true;
  iter_tile(board, g, w, h, p, [&](int j, int i, bool edge) {
    if (!valid(j, i, w, h) || board[j][i] != '.') {
      ans = false;
    }
  });
  return ans;
}

int main() {
  int w, h, dummy;
  cin >> w >> h >> dummy;
  vector<string> board(h);
  for (int i = 0; i < h; i++) {
    cin >> board[i];
  }
  vector<Group> groups;
  vector<vector<int>> index(h, vector<int>(w, -1));
  int is = 0;
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      if (board[j][i] != '.') {
        int g = board[j][i] <= '9' ?
            board[j][i] - '0' : board[j][i] - 'A' + 10;
        groups.push_back(Group{g, j, i});
      } else {
        index[j][i] = is++;
      }
    }
  }
  int gs = groups.size();
  vector<vector<bool>> mat;
  vector<Row> rows;
  for (unsigned short g = 0; g < gs; g++) {
    iter_partitions(groups[g].size, [&](Partition &p) {
      if (valid_tile(board, groups[g], w, h, p)) {
        vector<bool> row(gs + is, false);
        row[g] = true;
        iter_tile(board, groups[g], w, h, p, [&](int j, int i, bool edge) {
          row[gs + index[j][i]] = true;
        });
        mat.push_back(row);
        rows.push_back(Row{g, p});
      }
    });
  }
  // Solve and print.
  cout << "rows: " << mat.size() << "\n";
  exactcover(mat, [&](const vector<int>& solution) {
    vector<vector<char>> out(h, vector<char>(w, '.'));
    for (int row : solution) {
      Group &g = groups[rows[row].group];
      Partition &p = rows[row].partition;
      iter_tile(board, g, w, h, p, [&](int jj, int ii, bool edge) {
        if (!edge) {
          out[jj][ii] = ii == g.i ? '|' : '-';
        } else {
          out[jj][ii] = ii < g.i ? '<' : (
                        ii > g.i ? '>' : (
                        jj < g.j ? '^' : 'v'));
        }
      });
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
    cout << "\n";
  });
}
