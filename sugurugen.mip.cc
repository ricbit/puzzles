#include <iostream>
#include <string>
#include <vector>
#include "easyscip/easyscip.h"

using namespace std;
using namespace easyscip;

bool valid(int i, int j, int w, int h) {
  return i >= 0 && i < w && j >= 0 && j < h;
}

int distance(int g, int p, int w, int h) {
    int ig = g % w, jg = g / w;
    int ip = p % w, jp = p / w;
    return abs(ig - ip) + abs(jg - jp);
}

template<typename T>
int iround(T x) {
  return static_cast<int>(x + 0.5);
}

int main() {
  int h, w, gmax;
  cin >> h >> w >> gmax;
  int ngroups = h * w;
  int npos = h * w;

  // Add variables.
  MIPSolver mip;
  vector<vector<Variable>> group_size(ngroups);
  for (int i = 0; i < ngroups; i++) {
    for (int g = 0; g < gmax; g++) {
      group_size[i].push_back(mip.binary_variable(0));
    }
  }

  vector<vector<vector<Variable>>> group(
      ngroups, vector<vector<Variable>>(npos));
  for (int g = 0; g < ngroups; g++) {
    for (int p = 0; p < npos; p++) {
      for (int d = 0; d < gmax; d++) {
        group[g][p].push_back(mip.binary_variable(0));
      }
    }
  }

  vector<vector<vector<Variable>>> 
      inflow(ngroups, vector<vector<Variable>>(npos)),
      outflow(ngroups, vector<vector<Variable>>(npos));
  for (int g = 0; g < ngroups; g++) {
    for (int p = 0; p < npos; p++) {
      for (int f = 0; f < 4; f++) {
        inflow[g][p].push_back(mip.integer_variable(0, gmax - 1, 0));
        outflow[g][p].push_back(mip.integer_variable(0, gmax - 1, 0));
      }
    }
  }

  // Mark static unreachables.
  for (int g = 0; g < ngroups; g++) {
    for (int p = 0; p < npos; p++) {
      if (distance(g, p, w, h) > gmax) {
        auto cons = mip.constraint();
        for (int d = 0; d < gmax; d++) {
          cons.add_variable(group[g][p][d], 1);
        }
        cons.commit(0, 0);
      }
    }
  }

  // Mark dynamic unreachables.
  for (int g = 0; g < ngroups; g++) {
    auto cons = mip.constraint();
    for (int p = 0; p < npos; p++) {
      for (int d = 0; d < gmax; d++) {
        cons.add_variable(group[g][p][d], -d);
      }
    }
    for (int p = 0; p < npos; p++) {
      for (int d = 0; d < gmax; d++) {
        cons.add_variable(group[g][p][d], distance(g, p, w, h));
      }
    }
    cons.commit(-npos * gmax * gmax, 0);
  }

  // Each group has only one size.
  for (int i = 0; i < ngroups; i++) {
    auto cons = mip.constraint();
    for (int g = 0; g < gmax; g++) {
      cons.add_variable(group_size[i][g], 1);
    }
    cons.commit(1, 1);
  }

  // A group may only have one or less number of each kind.
  for (int g = 0; g < ngroups; g++) {
    for (int d = 0; d < gmax; d++) {
      auto cons = mip.constraint();
      for (int p = 0; p < npos; p++) {
        cons.add_variable(group[g][p][d], 1);
      }
      cons.commit(0, 1);
    }
  }

  // Each pos must have only one digit.
  for (int p = 0; p < npos; p++) {
    auto cons = mip.constraint();
    for (int g = 0; g < ngroups; g++) {
      for (int d = 0; d < gmax; d++) {
        cons.add_variable(group[g][p][d], 1);
      }
    }
    cons.commit(1, 1);
  }

  // Numbers must not be neighbours.
  for (int d = 0; d < gmax; d++) {
    for (int p = 0; p < npos; p++) {
      int i = p % w, j = p / w;
      auto cons = mip.constraint();
      int k = 0;
      for (int g = 0; g < ngroups; g++) {
        for (int ii = -1; ii <= 1; ii++) {
          for (int jj = -1; jj <= 1; jj++) {
            if (!valid(i + ii, j + jj, w, h) || (ii == 0 && jj == 0)) {
              continue;
            }
            int pp = i + ii + (j + jj) * w;
            cons.add_variable(group[g][pp][d], 1);
            k++;
          }
        }
      }
      for (int g = 0; g < ngroups; g++) {
        cons.add_variable(group[g][p][d], k);
      }
      cons.commit(0, k);
    }
  }

  // Each group has a preferred position for the digit 1.
  for (int g = 0; g < ngroups; g++) {
    auto cons = mip.constraint();
    for (int p = 0; p < npos; p++) {
      if (g != p) {
        cons.add_variable(group[g][p][0], 1);
      }
    }
    cons.commit(0, 0);
  }

  // Each group only has digit x if all 1..x-1 are present.
  for (int g = 0; g < ngroups; g++) {
    for (int d = 1; d < gmax; d++) {
      auto cons = mip.constraint();
      for (int dd = 0; dd < d; dd++) {
        for (int p = 0; p < npos; p++) {
          cons.add_variable(group[g][p][dd], 1);
        }
      }
      for (int p = 0; p < npos; p++) {
        cons.add_variable(group[g][p][d], -d);
      }
      cons.commit(0, d);
    }
  }

  // Flow entering a digit 1 is always zero.
  for (int g = 0; g < ngroups; g++) {
    auto cons = mip.constraint();
    for (int i = 0; i < 4; i++) {
      cons.add_variable(inflow[g][g][i], 1);
    }
    cons.commit(0, 0);
  }

  // Flow exiting a digit 1 is equal to (max-1).
  for (int g = 0; g < ngroups; g++) {
    auto cons = mip.constraint();
    for (int p = 0; p < npos; p++) {
      if (g != p) {
        for (int d = 1; d < gmax; d++) {
          cons.add_variable(group[g][p][d], 1);
        }
      }
    }
    for (int i = 0; i < 4; i++) {
      cons.add_variable(outflow[g][g][i], -1);
    }
    cons.commit(0, 0);
  }

  // Kirchoff's law for flows.
  /*for (int g = 0; g < ngroups; g++) {
    for (int p = 0; p < npos; p++) {
      if (g == p) {
        continue;
      }
      auto cons = mip.constraint();
      static int di[4] = {1, -1, 0, 0};
      static int dj[4] = {0, 0, 1, -1};
      int pi = p % w, pj = p / w;
      for (int i = 0; i < 4; i++) {
        if (valid(pi + di[i], pj + dj[i], w, h)) {
        }
      }
    }
  }*/

  // Solve and print.
  auto sol = mip.solve();
  if (!sol.is_optimal()) {
    return 0;
  }
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      int p = j * w + i;
      for (int g = 0; g < ngroups; g++) {
        for (int d = 0; d < gmax; d++) {
          if (sol.value(group[g][p][d]) > 0.5) {
            cout << d + 1;
            cout << static_cast<char>('a' + g);
            cout << "(";
            for (int i = 0; i < 4; i++) {
              cout << iround(sol.value(inflow[g][p][i]));
            }
            cout << ";";
            for (int i = 0; i < 4; i++) {
              cout << iround(sol.value(outflow[g][p][i]));
            }
            cout << ") ";
          }
        }
      }
    }
    cout << "\n";
  }
  cout << "\n";
}
