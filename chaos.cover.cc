// Solver for the 6x6 Irregular Sudoku
// Ricardo Bittencourt 2008

#include <cstdio>
#include <vector>
#include <iostream>
#include <cstring>
#include <map>

using namespace std;

#include "exactcover/exactcover.h"
#include "printers/printers.h"

// callback to print the solutions found.
struct print_solution {
  char grid[6][7];  // Stores the group information (a-f)
  char given[6][7]; // Stores the given numbers

  print_solution(const char grid_[6][7], const char given_[6][7]) {
    memcpy(grid, grid_, sizeof(grid));
    memcpy(given, given_, sizeof(given));
  }

  void operator()(const vector<int>& solution) {
    // Convert solution to 2D grid
    vector<vector<int>> number_grid(6, vector<int>(6, 0));
    for (auto it : solution) {
      int row = it / 36;
      int col = (it / 6) % 6;
      int digit = it % 6;
      number_grid[row][col] = digit + 1;  // Store 1-6 instead of 0-5
    }

    // Convert group information to 2D char grid
    vector<vector<char>> groups(6, vector<char>(6));
    for (int i = 0; i < 6; i++) {
      for (int j = 0; j < 6; j++) {
        groups[i][j] = grid[i][j];
      }
    }

    // Set up cell symbols (1-6) using Unicode wide numbers
    // and color the givens in blue
    map<int, string> cell_symbols;
    const char* wide_numbers[] = {"１", "２", "３", "４", "５", "６"};
    for (int i = 1; i <= 6; i++) {
      cell_symbols[i] = wide_numbers[i-1];
    }

    // Add blue versions for givens
    for (int i = 0; i < 6; i++) {
      for (int j = 0; j < 6; j++) {
        if (given[i][j] != '-') {
          int num = given[i][j] - '1' + 1;
          number_grid[i][j] = num + 10;  // Offset by 10 for givens
          cell_symbols[num + 10] = "\033[34m" + string(wide_numbers[num-1]) + "\033[0m";
        }
      }
    }

    // Create and use group printer
    GroupPrinter printer(6, 6, groups, cell_symbols, 3);  // Use cell_width=3 for chaos
    printer.print(number_grid);
  }
};

int main(void) {
  // read the grid from stdin.
  char grid[6][7], given[6][7];
  for (int i = 0; i < 6; i++)
    cin >> grid[i];
  for (int i = 0; i < 6; i++)
    cin >> given[i];

  // build the exact cover matrix.
  vvb mat(6*6*6, vb(36*4, false));
  for (int digit = 0; digit < 6; digit++)
    for (int row = 0; row < 6; row++)
      for (int col = 0; col < 6; col++) {
        if (given[row][col] != '-' && given[row][col]-'1' != digit)
          continue;
        int i = digit + col*6 + row*6*6;
        mat[i][col + row*6] = true;
        mat[i][36 + row*6 + digit] = true;
        mat[i][36*2 + col*6 + digit] = true;
        int box = grid[row][col] - 'a';
        mat[i][36*3 + box*6 + digit]=true;
      }

  // print the solution.
  print_solution out(grid, given);
  exactcover(mat, out);

  return 0;
}
