#include <iostream>
#include <array>
#include <vector>
#include <string>
#include <map>
#include "exactcover/exactcover.h"
#include "printers/printers.h"
#include "printers/branches.h" // Include the new header

using namespace std;

bool valid(int j, int i, int w, int h) {
  return i >= 0 && i < w && j >= 0 && j < h;
}

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
void iter_tile(const vector<string> &board, const Group &g, int w, int h, const Partition &p, T func) {
  for (int d = 0; d < 4; ++d) {
    for (int i = 1; i <= p[d]; i++) {
        func(g.j + dy[d] * i, g.i + dx[d] * i, d);
    }
  }
}

// Update the call in valid_tile (just add 'd' to lambda signature)
bool valid_tile(vector<string> &board, Group &g, int w, int h, Partition &p) {
  bool ans = true;
  iter_tile(board, g, w, h, p, [&](int j, int i, int d) {
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
        iter_tile(board, groups[g], w, h, p, [&](int j, int i, int d)  {
          // Check if index[j][i] is valid before accessing, this lambda does not use d
          if (valid(j, i, w, h) && index[j][i] != -1) {
             row[gs + index[j][i]] = true;
          }
        });
        mat.push_back(row);
        rows.push_back(Row{g, p});
      }
    });
  }
  // Solve and print.
  cout << "rows: " << mat.size() << "\n";

  exactcover(mat, [&](const vector<int>& solution_rows) {
    // Define special IDs for lines and empty cells
    const int EMPTY_CELL = -1;
    const int VERTICAL_LINE = -2;
    const int HORIZONTAL_LINE = -3;

    // Grid to store integer identifiers (group ID for seed, line IDs, or empty)
    vector<vector<int>> solution_grid(h, vector<int>(w, EMPTY_CELL));
    // Grid to store the character identifier for drawing group boundaries
    vector<vector<char>> groupmap(h, vector<char>(w, '.'));
    char current_group_char = 'a'; // Character label for group boundaries

    // Populate the grids based on the solution rows
    for (int row_index : solution_rows) {
        // Ensure row_index is valid for the rows vector
        if (row_index < 0 || row_index >= static_cast<int>(rows.size())) {
            cerr << "Error: Invalid row index " << row_index << " in solution." << endl;
            continue; // Skip this invalid row
        }
        const Row& row_data = rows[row_index];
        // Ensure group index is valid for the groups vector
        if (row_data.group < 0 || row_data.group >= groups.size()) {
             cerr << "Error: Invalid group index " << row_data.group << " in row data." << endl;
             continue; // Skip this row with invalid group
        }
        const Group& g = groups[row_data.group];
        const Partition& p = row_data.partition;
        char group_char_for_map = current_group_char++; // Assign a unique char for this group's boundary

        // Mark the seed cell (check bounds)
        if (valid(g.j, g.i, w, h)) {
            solution_grid[g.j][g.i] = row_data.group; // Use group index as identifier ONLY for seed
            groupmap[g.j][g.i] = group_char_for_map;
        } else {
             cerr << "Warning: Seed cell (" << g.j << "," << g.i << ") out of bounds." << endl;
        }

        // Mark the branch cells using line symbols
        iter_tile(board, g, w, h, p, [&](int jj, int ii, int d) {
            // Check bounds before assigning
            if (valid(jj, ii, w, h)) {
                // Assign vertical or horizontal line ID based on direction d
                solution_grid[jj][ii] = (d == 0 || d == 2) ? VERTICAL_LINE : HORIZONTAL_LINE;
                groupmap[jj][ii] = group_char_for_map;
            } else {
                cerr << "Warning: Branch cell (" << jj << "," << ii << ") out of bounds during iteration." << endl;
            }
        });
    }

    // Call the centralized printer function
    print_branches_grid(h, w, groups, solution_grid, groupmap);

  }); // End of exactcover lambda
}
