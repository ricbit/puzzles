#include <cstdio>
#include <iostream>
#include <vector>
#include <map>
#include <string>

using namespace std;

#include "exactcover/exactcover.h"
#include "printers/printers.h"

typedef vector<bool> vb;
typedef vector<vb> vvb;

int mat[7][8];
int domino[7][7];

struct print_solution {
  void operator()(const vector<int>& solution) {
    // Convert solution to grid format
    vector<vector<int>> grid(7, vector<int>(8, 0));
    vector<vector<char>> groups(7, vector<char>(8, 'a'));
    char group_id = 'a';
    
    // Fill in the solution grid and group information
    for (int line : solution) {
      if (line < 49) {
        int i = line % 7, j = line / 7;
        grid[j][i] = mat[j][i] + 1;
        grid[j][i+1] = mat[j][i+1] + 1;
        groups[j][i] = group_id;
        groups[j][i+1] = group_id;
      } else {
        int i = (line - 49) % 8, j = (line - 49) / 8;
        grid[j][i] = mat[j][i] + 1;
        grid[j+1][i] = mat[j+1][i] + 1;
        groups[j][i] = group_id;
        groups[j+1][i] = group_id;
      }
      group_id++;
    }

    // Set up cell symbols with wide Unicode numbers
    map<int, string> cell_symbols;
    const char* wide_numbers[] = {"０", "１", "２", "３", "４", "５", "６"};
    for (int i = 0; i < 7; i++) {
      cell_symbols[i + 1] = wide_numbers[i];
    }

    // Create and use group printer
    GroupPrinter printer(7, 8, groups, cell_symbols, 2);
    printer.print(grid);
  }
};

int main(void) {
  int tot = 1;
  int c = 0;
  for (int j=0; j<7; j++)
    for (int i=j; i<7; i++)
      domino[i][j] = domino[j][i] = c++;

  while (tot--) {
    for (int j=0; j<7; j++)
      for (int i=0; i<8; i++)
        cin >> mat[j][i];
    vvb mmap(97, vb(84, false));
    // horiz
    for (int j=0; j<7; j++)
      for (int i=0; i<7; i++) {
        mmap[j*7+i][domino[mat[j][i]][mat[j][i+1]]] = true;
        mmap[j*7+i][28+j*8+i] = true;
        mmap[j*7+i][28+j*8+i+1] = true;
      }
    // vert
    for (int i=0; i<8; i++)
      for (int j=0; j<6; j++) {
        mmap[49+j*8+i][domino[mat[j][i]][mat[j+1][i]]] = true;
        mmap[49+j*8+i][28+j*8+i] = true;
        mmap[49+j*8+i][28+(j+1)*8+i] = true;
      }
    print_solution print;
    exactcover(mmap, print);
  }
  return 0;
}
