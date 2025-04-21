#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <limits>
#include "easyscip/easyscip.h"
#include "printers/printers.h"
#include "printers/branches.h" // Include the new header

using namespace std;
using namespace easyscip;

// Group struct is now defined in printers/branches.h
// Remove the struct Group definition from here if it exists

bool valid(int i, int j, int w, int h) {
  return i >= 0 && i < w && j >= 0 && j < h;
}

template<typename T>
bool valid_block(T &board, int jn, int jj, int in, int ii) {
  int count = 0;
  for (int j2 = min(jn, jj); j2 <= max(jn, jj); j2++) {
    for (int i2 = min(in, ii); i2 <= max(in, ii); i2++) {
      // Check bounds before accessing board
      if (valid(i2, j2, board[0].size(), board.size()) && board[j2][i2] != '.') {
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
            // Check if the potential endpoint (jj, ii) is valid according to original rules
            if (valid_tile(board, jj, ii, groups[g], w, h)) {
              int jn = groups[g].j + dy[d]; // First step from seed in direction d
              int in = groups[g].i + dx[d]; // First step from seed in direction d
              // Check if the current cell (j, i) lies on the path from seed to (jj, ii)
              if ((i == ii && j >= min(jn, jj) && j <= max(jn, jj)) ||
                  (j == jj && i >= min(in, ii) && i <= max(in, ii))) {
                 // If cell (j,i) is on the path, the variable for the endpoint (jj,ii) covers it
                 cons.add_variable(grid[jj][ii][g], 1);
              }
            }
          }
        }
      }
      cons.commit(1, 1); // Cell (j, i) must be covered by exactly one branch endpoint variable
    }
  }

  // Number of cells covered by each group must be equal to seed number.
  for (int g = 0; g < gs; g++) {
    auto cons = mip.constraint();
    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        cons.add_variable(grid[j][i][g], distance(j, i, groups[g]));
      }
    }
    cons.commit(groups[g].size, groups[g].size);
  }


  // Solve and print using GroupPrinter.
  cout << "Solving Branches...\n";
  auto sol = mip.solve();

  // Check if a solution exists (using the objective value check)
  const double SCIP_MAYBE_INVALID_THRESHOLD = 1e+19;
  bool solution_exists = (sol.objective() < SCIP_MAYBE_INVALID_THRESHOLD &&
                          sol.objective() > -SCIP_MAYBE_INVALID_THRESHOLD);

  if (solution_exists) {
      cout << "Solution found:\n";

      // Define special IDs for lines and empty cells
      const int EMPTY_CELL = -1;
      const int VERTICAL_LINE = -2;
      const int HORIZONTAL_LINE = -3;

      // Grid to store integer identifiers (group ID for seed, line IDs, or empty)
      vector<vector<int>> solution_grid(h, vector<int>(w, EMPTY_CELL));
      // Grid to store the character identifier for drawing group boundaries
      vector<vector<char>> groupmap(h, vector<char>(w, '.')); // Initialize with a default non-group char
      char current_group_char = 'a'; // Character label for group boundaries

      // Populate the grids based on the solution
      for (int g = 0; g < gs; ++g) {
          char group_char_for_map = current_group_char++; // Assign a unique char for this group's boundary

          // Mark the seed cell
          if (valid(groups[g].i, groups[g].j, w, h)) {
              solution_grid[groups[g].j][groups[g].i] = g; // Use group index as identifier ONLY for seed
              groupmap[groups[g].j][groups[g].i] = group_char_for_map;
          }

          // Iterate through potential branch endpoints and fill the path
          for (int j = 0; j < h; ++j) {
              for (int i = 0; i < w; ++i) {
                  // Check if variable is selected in the solution
                  if (sol.value(grid[j][i][g]) > 0.5) {
                      // This (j, i) is an endpoint for group g. Fill the path from seed to here.
                      int seed_j = groups[g].j;
                      int seed_i = groups[g].i;

                      if (j == seed_j) { // Horizontal branch
                          for (int cur_i = min(i, seed_i); cur_i <= max(i, seed_i); ++cur_i) {
                              if (cur_i != seed_i) { // Don't overwrite the seed cell's ID
                                  if (valid(cur_i, j, w, h)) { // Bounds check
                                      solution_grid[j][cur_i] = HORIZONTAL_LINE;
                                      groupmap[j][cur_i] = group_char_for_map;
                                  }
                              } else { // Ensure seed cell is in the group map
                                  if (valid(cur_i, j, w, h)) groupmap[j][cur_i] = group_char_for_map;
                              }
                          }
                      } else if (i == seed_i) { // Vertical branch
                          for (int cur_j = min(j, seed_j); cur_j <= max(j, seed_j); ++cur_j) {
                               if (cur_j != seed_j) { // Don't overwrite the seed cell's ID
                                  if (valid(i, cur_j, w, h)) { // Bounds check
                                      solution_grid[cur_j][i] = VERTICAL_LINE;
                                      groupmap[cur_j][i] = group_char_for_map;
                                  }
                              } else { // Ensure seed cell is in the group map
                                  if (valid(i, cur_j, w, h)) groupmap[cur_j][i] = group_char_for_map;
                              }
                          }
                      }
                  }
              }
          }
      }

      // Call the centralized printer function
      print_branches_grid(h, w, groups, solution_grid, groupmap);

  } else {
      cout << "No solution found (problem might be infeasible or solver stopped early).\n";
  }

  cout << "\n";
  return 0;
}
