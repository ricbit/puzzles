#include <cstdio>
#include <iostream>
#include <vector>

using namespace std;

#include "exactcover/exactcover.h"

int mat[7][8];
int domino[7][7];

struct print_solution {
  void operator()(const vector<int>& solution) {
    int m[7][8], c = 1;
    for (int line : solution) {
      if (line < 49) {
        int i = line % 7, j = line / 7;
        m[j][i] = c;
        m[j][i+1] = c;
      } else {
        int i = (line - 49) % 8, j = (line - 49) / 8;
        m[j][i] = c;
        m[j+1][i] = c;
      }
      c++;
    }
    for (int i = 0; i < 7; i++) {
      for (int j = 0; j < 8; j++) {
        printf("%02d ", m[i][j]);
      }
      printf("\n");
    }
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
