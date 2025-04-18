// Solution to Doppelstern using exact cover.
// Ricardo Bittencourt 2011

// Please see the link below for rules and examples.
// http://www.puzzlephil.com/index.php/en/puzzles/stars

#include <iostream>
#include <vector>
#include <set>

using namespace std;

#include "exactcover/exactcover.h"

struct print_solutions {
    int h, w;
    vector<string> input;

    print_solutions(int h_, int w_, vector<string> input_)
        : h(h_), w(w_), input(input_) {}

    bool diff_right(int y, int x) {
        return x + 1 < w && input[y][x] != input[y][x + 1];
    }

    bool diff_down(int y, int x) {
        return y + 1 < h && input[y][x] != input[y + 1][x];
    }

    // Returns the correct Unicode corner character
    string get_corner(int y, int x) {
        char a = (y > 0 && x > 0)     ? input[y - 1][x - 1] : 0;
        char b = (y > 0 && x < w)     ? input[y - 1][x    ] : 0;
        char c = (y < h && x > 0)     ? input[y    ][x - 1] : 0;
        char d = (y < h && x < w)     ? input[y    ][x    ] : 0;

        bool up = a != b;
        bool down = c != d;
        bool left = a != c;
        bool right = b != d;

        int count = up + down + left + right;

        if (count == 0) return " ";
        if (count == 1) {
            if (up || down) return "│";
            return "─";
        }
        if (count == 2) {
            if (up && down) return "│";
            if (left && right) return "─";
            if (up && right) return "└";
            if (up && left) return "┘";
            if (down && right) return "┌";
            if (down && left) return "┐";
        }
        if (count == 3) {
            if (!up) return "┬";
            if (!down) return "┴";
            if (!left) return "├";
            if (!right) return "┤";
        }
        return "┼";
    }

    void operator()(const vector<int>& solution) {
        // Convert flat list of star positions into 2D grid
        vector<vector<bool>> stars(h, vector<bool>(w, false));
        for (int i = 0; i < int(solution.size()); i++) {
            if (solution[i] < h * w)
                stars[solution[i] / w][solution[i] % w] = true;
        }

        // Top border
        cout << "┌";
        for (int x = 0; x < w - 1; ++x)
            cout << "──" << (diff_right(0, x) ? "┬" : "─");
        cout << "──┐\n";

        // Rows
        for (int y = 0; y < h; ++y) {
            // Content row
            cout << "│";
            for (int x = 0; x < w; ++x) {
                if (stars[y][x])
                    cout << "\033[33m★ \033[0m";  // yellow star
                else
                    cout << "□ ";
                if (x < w - 1)
                    cout << (diff_right(y, x) ? "│" : " ");
            }
            cout << "│\n";

            // Border row (after each line, but skip last because bottom border is printed later)
            if (y < h - 1) {
                for (int x = 0; x <= w; ++x) {
                    cout << get_corner(y + 1, x);
                    if (x < w)
                        cout << (diff_down(y, x) ? "──" : "  ");
                }
                cout << '\n';
            }        
        }

        // Bottom border
        cout << "└";
        for (int x = 0; x < w - 1; ++x)
            cout << "──" << (diff_right(h - 1, x) ? "┴" : "─");
        cout << "──┘\n";
    }
};

int main() {
  int stars, x, y;
  cin >> stars >> y >> x;
  // stars must be 1 in this version.
  vector<string> input(y);
  for (int i = 0; i < y; i++) {
    cin >> input[i];
  }
  set<char> groups;
  for (int j = 0; j < y; j++) {
    for (int i = 0; i < x; i++) {
      groups.insert(input[j][i]);
    }
  }
  int corners = (x - 1) * (y - 1);
  int lines = x * y + corners;
  int columns = y + x + groups.size() + corners;
  vector<vector<bool> > mat(lines, vector<bool>(columns, false));
  const int slines = 0;
  const int scolumns = y;
  const int sgroups = y + x;
  const int scorners = y + x + groups.size();
  for (int j = 0; j < y; j++) {
    for (int i = 0; i < x; i++) {
      int line = j * x + i;
      mat[line][slines + i] = true;
      mat[line][scolumns + j] = true;
      mat[line][sgroups + (input[j][i] - 'a')] = true;
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
  print_solutions print(y, x, input);
  exactcover(mat, print);
}
