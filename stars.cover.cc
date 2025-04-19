// Solution to Doppelstern using exact cover.
// Ricardo Bittencourt 2011

// Please see the link below for rules and examples.
// http://www.puzzlephil.com/index.php/en/puzzles/stars

#include <iostream>
#include <vector>
#include <set>
#include <map>

using namespace std;

#include "exactcover/exactcover.h"
#include "printers/printers.h"

struct print_solutions {
    int h, w;
    vector<string> groups;

    print_solutions(int h_, int w_, vector<string> groups_)
        : h(h_), w(w_), groups(groups_) {}

    void operator()(const vector<int>& solution) {
        // Convert flat list of star positions into 2D grid
        vector<vector<int>> grid(h, vector<int>(w, 0));
        for (int i = 0; i < int(solution.size()); i++) {
            if (solution[i] < h * w)
                grid[solution[i] / w][solution[i] % w] = 1;  // 1 represents a star
        }

        // Convert input strings to char grid for groups
        vector<vector<char>> group_grid(h, vector<char>(w));
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                group_grid[y][x] = groups[y][x];
            }
        }

        // Define cell symbols
        map<int, string> cell_symbols;
        cell_symbols[1] = "\033[33m★\033[0m";  // yellow star
        cell_symbols[0] = "·";  // empty

        // Create and use the group printer
        GroupPrinter printer(h, w, group_grid, cell_symbols, 2);
        printer.print(grid);
    }
};

int main() {
  int stars, x, y;
  cin >> stars >> y >> x;
  // stars must be 1 in this version.
  vector<string> groups(y);
  for (int i = 0; i < y; i++) {
    cin >> groups[i];
  }
  set<char> groups_set;
  for (int j = 0; j < y; j++) {
    for (int i = 0; i < x; i++) {
      groups_set.insert(groups[j][i]);
    }
  }
  int corners = (x - 1) * (y - 1);
  int lines = x * y + corners;
  int columns = y + x + groups_set.size() + corners;
  vector<vector<bool> > mat(lines, vector<bool>(columns, false));
  const int slines = 0;
  const int scolumns = y;
  const int sgroups = y + x;
  const int scorners = y + x + groups_set.size();
  for (int j = 0; j < y; j++) {
    for (int i = 0; i < x; i++) {
      int line = j * x + i;
      mat[line][slines + i] = true;
      mat[line][scolumns + j] = true;
      mat[line][sgroups + (groups[j][i] - 'a')] = true;
    }
  }
  for (int j = 0; j < y - 1; j++) {
    for (int i = 0; i < x - 1; i++) {
      int dummyline = x * y + (j * (x - 1) + i);
      int dummycolumn = scorners + j * (x - 1) + i;
      mat[dummyline][dummycolumn] = true;
      for (int jj = 0; jj < 2; jj++) {
        for (int ii = 0; ii < 2; ii++) {
          int line = (j+jj) * x + (i + ii);
          mat[line][scorners + j * (x - 1) + i] = true;
        }
      }
    }
  }
  print_solutions print(y, x, groups);
  exactcover(mat, print);
}
