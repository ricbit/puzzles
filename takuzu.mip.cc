#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <limits>
#include "easyscip/easyscip.h"
#include "printers/printers.h"

using namespace std;
using namespace easyscip;

bool valid(int i, int j, int w, int h) {
  return i >= 0 && i < w && j >= 0 && j < h;
}

int main() {
  int w,h;
  cin >> w >> h;
  vector<string> input_board(h);
  for (int i = 0; i < h; i++) {
    cin >> input_board[i];
  }
  MIPSolver mip;

  vector<vector<Variable>> red(h);
  vector<vector<Variable>> blue(h);
  for (int j = 0; j < h; j++) {
    red[j].reserve(w);
    blue[j].reserve(w);
    for (int i = 0; i < w; i++) {
      // Use emplace_back to construct Variables in place
      red[j].emplace_back(mip.binary_variable(0));
      blue[j].emplace_back(mip.binary_variable(0));
    }
  }


  // Add the givens.
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      if (input_board[j][i] == 'r') {
        auto cons = mip.constraint();
        cons.add_variable(red[j][i], 1);
        cons.commit(1, 1);
        // Also constrain blue to 0 for givens
        auto cons_blue = mip.constraint();
        cons_blue.add_variable(blue[j][i], 1);
        cons_blue.commit(0, 0);
      }
      if (input_board[j][i] == 'b') {
        auto cons = mip.constraint();
        cons.add_variable(blue[j][i], 1);
        cons.commit(1, 1);
         // Also constrain red to 0 for givens
        auto cons_red = mip.constraint();
        cons_red.add_variable(red[j][i], 1);
        cons_red.commit(0, 0);
      }
    }
  }

  // Either a position is red or blue.
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      auto cons = mip.constraint();
      cons.add_variable(red[j][i], 1);
      cons.add_variable(blue[j][i], 1);
      cons.commit(1, 1);
    }
  }

  // Exactly w/2 reds per line.
  for (int j = 0; j < h; j++) {
    auto cons = mip.constraint();
    for (int i = 0; i < w; i++) {
      cons.add_variable(red[j][i], 1);
    }
    cons.commit(w / 2, w / 2);
  }

  // Exactly h/2 reds per column.
  for (int i = 0; i < w; i++) {
    auto cons = mip.constraint();
    for (int j = 0; j < h; j++) {
      cons.add_variable(red[j][i], 1);
    }
    cons.commit(h / 2, h / 2);
  }

  // No three h-adjacents should be equal.
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w - 2; i++) {
      // Constraint for red
      auto cons_red = mip.constraint();
      cons_red.add_variable(red[j][i], 1);
      cons_red.add_variable(red[j][i + 1], 1);
      cons_red.add_variable(red[j][i + 2], 1);
      cons_red.commit(1, 2); // Cannot be 0 or 3 reds
      // Constraint for blue
      auto cons_blue = mip.constraint();
      cons_blue.add_variable(blue[j][i], 1);
      cons_blue.add_variable(blue[j][i + 1], 1);
      cons_blue.add_variable(blue[j][i + 2], 1);
      cons_blue.commit(1, 2); // Cannot be 0 or 3 blues
    }
  }

  // No three v-adjacents should be equal.
  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h - 2; j++) {
       // Constraint for red
      auto cons_red = mip.constraint();
      cons_red.add_variable(red[j][i], 1);
      cons_red.add_variable(red[j + 1][i], 1);
      cons_red.add_variable(red[j + 2][i], 1);
      cons_red.commit(1, 2); // Cannot be 0 or 3 reds
      // Constraint for blue
      auto cons_blue = mip.constraint();
      cons_blue.add_variable(blue[j][i], 1);
      cons_blue.add_variable(blue[j + 1][i], 1);
      cons_blue.add_variable(blue[j + 2][i], 1);
      cons_blue.commit(1, 2); // Cannot be 0 or 3 blues
    }
  }

  // --- Constraints for unique rows/columns ---
  int M_col = 1 << (h + 1); // A sufficiently large number (max possible column difference + 1)

  // No two columns should be equal.
  for (int i = 0; i < w; i++) {
    for (int ii = i + 1; ii < w; ii++) {
      Variable diff = mip.integer_variable(-M_col, M_col, 0); // Difference variable
      Variable p = mip.binary_variable(0); // Positive difference indicator
      Variable n = mip.binary_variable(0); // Negative difference indicator

      // Constraint: diff = column[i] - column[ii]
      auto cons_diff = mip.constraint();
      for (int j = 0; j < h; j++) {
        cons_diff.add_variable(red[j][i], 1 << j);  // Add value of column i
        cons_diff.add_variable(red[j][ii], -(1 << j)); // Subtract value of column ii
      }
      cons_diff.add_variable(diff, -1); // diff - (col_i - col_ii) = 0
      cons_diff.commit(0, 0);

      // Constraint: diff >= 1 - M_col * (1 - p)  => diff + M_col*p >= 1
      auto cons_p = mip.constraint();
      cons_p.add_variable(diff, 1);
      cons_p.add_variable(p, M_col);
      cons_p.commit(1, M_col + M_col); // Upper bound is M_col + M_col (max diff + M_col)

      // Constraint: diff <= -1 + M_col * (1 - n) => diff - M_col*n <= -1
      auto cons_n = mip.constraint();
      cons_n.add_variable(diff, 1);
      cons_n.add_variable(n, -M_col);
      cons_n.commit(-M_col - M_col, -1); // Lower bound is -M_col - M_col (min diff - M_col)

      // Constraint: p + n = 1 (difference must be non-zero)
      auto cons_pn = mip.constraint();
      cons_pn.add_variable(p, 1);
      cons_pn.add_variable(n, 1);
      cons_pn.commit(1, 1);
    }
  }

  int M_row = 1 << (w + 1); // Update M for row comparison

  // No two rows should be equal.
  for (int j = 0; j < h; j++) {
    for (int jj = j + 1; jj < h; jj++) {
      Variable diff = mip.integer_variable(-M_row, M_row, 0); // Difference variable
      Variable p = mip.binary_variable(0); // Positive difference indicator
      Variable n = mip.binary_variable(0); // Negative difference indicator

      // Constraint: diff = row[j] - row[jj]
      auto cons_diff = mip.constraint();
      for (int i = 0; i < w; i++) {
        cons_diff.add_variable(red[j][i], 1 << i);  // Add value of row j
        cons_diff.add_variable(red[jj][i], -(1 << i)); // Subtract value of row jj
      }
      cons_diff.add_variable(diff, -1); // diff - (row_j - row_jj) = 0
      cons_diff.commit(0, 0);

      // Constraint: diff >= 1 - M_row * (1 - p) => diff + M_row*p >= 1
      auto cons_p = mip.constraint();
      cons_p.add_variable(diff, 1);
      cons_p.add_variable(p, M_row);
      cons_p.commit(1, M_row + M_row);

      // Constraint: diff <= -1 + M_row * (1 - n) => diff - M_row*n <= -1
      auto cons_n = mip.constraint();
      cons_n.add_variable(diff, 1);
      cons_n.add_variable(n, -M_row);
      cons_n.commit(-M_row - M_row, -1);

      // Constraint: p + n = 1
      auto cons_pn = mip.constraint();
      cons_pn.add_variable(p, 1);
      cons_pn.add_variable(n, 1);
      cons_pn.commit(1, 1);
    }
  }

  // Solve and print using GroupPrinter.
  cout << "Solving Takuzu...\n";
  auto sol = mip.solve();

  // Check if the objective value is valid (not SCIP_INVALID, which is usually >= 1e+20)
  // Use a threshold slightly lower than typical infinity values used in SCIP.
  const double SCIP_MAYBE_INVALID_THRESHOLD = 1e+19;
  bool solution_exists = (sol.objective() < SCIP_MAYBE_INVALID_THRESHOLD &&
                          sol.objective() > -SCIP_MAYBE_INVALID_THRESHOLD);

  if (solution_exists) {
      cout << "Solution found:\n";

      // Prepare data for GroupPrinter
      vector<vector<int>> solution_grid(h, vector<int>(w));
      vector<vector<char>> groups(h, vector<char>(w));
      for (int j = 0; j < h; ++j) {
          for (int i = 0; i < w; ++i) {
              // Assign a unique group character to each cell
              // Ensure group char stays within reasonable range if needed
              groups[j][i] = 'a' + ((j * w + i) % (26*26)); // Example modulo if many cells

              bool is_red = sol.value(red[j][i]) > 0.5;
              bool is_given = input_board[j][i] != '.'; // Assuming '.' means not given

              // Use codes: 1=solved red(1), 2=solved blue(0)
              // 11=given red(1), 12=given blue(0)
              if (is_red) {
                  solution_grid[j][i] = is_given ? 11 : 1;
              } else {
                  solution_grid[j][i] = is_given ? 12 : 2;
              }
          }
      }

      // Define cell symbols using wide digits and colors
      map<int, string> cell_symbols;
      const char* wide_zero = "０"; // Represents blue (0)
      const char* wide_one = "１";  // Represents red (1)

      // Solved cells (grey - ANSI code 90)
      cell_symbols[1] = "\033[90m" + string(wide_one) + "\033[0m";  // Solved '1' (red)
      cell_symbols[2] = "\033[90m" + string(wide_zero) + "\033[0m"; // Solved '0' (blue)

      // Given cells (white/default - ANSI code 37 or default)
      // Using default color which is usually white on dark terminals
      cell_symbols[11] = string(wide_one);  // Given '1' (red)
      cell_symbols[12] = string(wide_zero); // Given '0' (blue)

      // Create and use the group printer
      // Cell width 2 is usually good for wide characters
      GroupPrinter printer(h, w, groups, cell_symbols, 2);
      printer.print(solution_grid);

  } else {
      cout << "No solution found (problem might be infeasible or solver stopped early).\n";
  }

  cout << "\n";
  return 0;
}
