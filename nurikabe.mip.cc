#include <cstdio>
#include <vector>
#include <queue>
#include <set>
#include <cmath>
#include <tuple>
#include <iostream>
#include "easyscip/easyscip.h"
#include <map>
#include <string>
#include "printers/printers.h"

using namespace std;
using namespace easyscip;

// Forward declarations
typedef vector<bool> vb;
typedef vector<vb> vvb;

struct Group {
  int length;
  int row, col;
};

struct GroupPosition {
  int group_idx;
  vector<pair<int, int>> pos;
};

struct EmptyPosition {
  vector<pair<int, int>> empty, border;
};

template<typename T>
T abs(T x) {
  return x < 0 ? -x : x;
}

template<typename T>
T sqr(T x) {
  return x * x;
}

template<typename T>
void cell_iterator(int rows, int cols, T func) {
  for (int ic = 0; ic < rows; ic++) {
    for (int jc = 0; jc < cols; jc++) {
      func(ic, jc);
    }
  }
}

template<typename T>
void full_iterator(int rows, int cols, int groups, T func) {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      for (int k = 0; k < groups; k++) {
        func(i, j, k);
      }
    }
  }
}

template<typename T>
void group_iterator(int groups, T func) {
  for (int i = 0; i < groups; i++) {
    func(i);
  }
}

template<typename T>
void neighbour_iterator(int rows, int cols, int i, int j, T func) {
  static const int dx[] = {1, -1, 0, 0};
  static const int dy[] = {0, 0, 1, -1};
  for (int k = 0; k < 4; k++) {
    int ni = i + dx[k];
    int nj = j + dy[k];
    if (ni >= 0 && nj >=0 && ni < rows && nj < cols) {
      func(ni, nj);
    }
  }
}

char bigdigit(int n) {
  if (n < 10) {
    return '0' + n;
  } else {
    return 'A' + n - 10;
  }
}

struct NurikabeVariables {
  vector<vector<Variable>> used;
  vector<vector<vector<Variable>>> has_group, hasnt_group;
  vector<vector<vector<Variable>>> edge_h, edge_v;
  vector<vector<Variable>> empty_edge_h, empty_edge_v;
  vector<Variable> empty_group;
  NurikabeVariables(int rows, int cols) 
      : used(rows),
        has_group(rows, vector<vector<Variable>>(cols)),
        hasnt_group(rows, vector<vector<Variable>>(cols)),
        edge_h(rows, vector<vector<Variable>>(cols - 1)),
        edge_v(rows - 1, vector<vector<Variable>>(cols)),
        empty_edge_h(rows), empty_edge_v(rows - 1) {
  }
};

struct NurikabeSolution {
  vector<vector<int>> pos;
  NurikabeSolution(int rows, int cols, int groups, 
                   NurikabeVariables& var, Solution& sol) 
      : pos(rows, vector<int>(cols, -1)) {
    full_iterator(rows, cols, groups, [&](int i, int j, int k) {
      if (sol.value(var.has_group[i][j][k]) > 0.5) {
        pos[i][j] = k;
      }
    });
  }
};

struct NurikabeMIP {
  int rows, cols, groups;
  const vector<Group>& group;
  const vector<GroupPosition>& forbidden;
  const vector<EmptyPosition>& empty_forbidden;
  NurikabeVariables var;
  MIPSolver mip;
 public:
  NurikabeMIP(int rows_, int cols_, const vector<Group>& group_,
      const vector<GroupPosition>& forbidden_, 
      const vector<EmptyPosition>& empty_forbidden_)
      : rows(rows_), cols(cols_), groups(group_.size()), 
        group(group_), forbidden(forbidden_),
        empty_forbidden(empty_forbidden_), 
        var(rows, cols), mip(true) {
    setup_variables();
    setup_constraints();
    //printf("Variables loaded.\n");
  }

  NurikabeSolution solve() {
    Solution sol = mip.solve();
    return NurikabeSolution(rows, cols, groups, var, sol);
  }

 private:
  void setup_variables() {
    // One variable for each cell, used or not.
    cell_iterator(rows, cols, [&](int i, int j) {
      var.used[i].push_back(mip.binary_variable(1));
    });
    // Two variables for each cell, for each group.
    full_iterator(rows, cols, groups, [&](int i, int j, int k) {
      var.has_group[i][j].push_back(mip.binary_variable(1));
      var.hasnt_group[i][j].push_back(mip.binary_variable(0));
    });
    // Edge variables for each group.
    full_iterator(rows, cols - 1, groups, [&](int i, int j, int k) {
      var.edge_h[i][j].push_back(mip.binary_variable(1));
    });
    full_iterator(rows - 1, cols, groups, [&](int i, int j, int k) {
      var.edge_v[i][j].push_back(mip.binary_variable(1));
    });
    // Edge variables for empty cells.
    cell_iterator(rows, cols - 1, [&](int i, int j) {
      var.empty_edge_h[i].push_back(mip.binary_variable(1));
    });
    cell_iterator(rows - 1, cols, [&](int i, int j) {
      var.empty_edge_v[i].push_back(mip.binary_variable(1));
    });
  }

  void setup_constraints() {
    // Mark each group seed.
    group_iterator(groups, [&](int k) {
      Constraint used_cons = mip.constraint();
      used_cons.add_variable(var.used[group[k].row][group[k].col], 1);
      used_cons.commit(1, 1);

      Constraint group_cons = mip.constraint();
      group_cons.add_variable(var.has_group[group[k].row][group[k].col][k], 1);
      group_cons.commit(1, 1);
    });
    // Each cell is either empty or has exactly one group.
    cell_iterator(rows, cols, [&](int i, int j) {
      Constraint cons = mip.constraint();
      group_iterator(groups, [&](int k) {
        cons.add_variable(var.has_group[i][j][k], 1);
      });
      cons.add_variable(var.used[i][j], -1);
      cons.commit(0, 0);
    });
    // Set the length of the group.
    group_iterator(groups, [&](int k) {
      Constraint cons = mip.constraint();
      cell_iterator(rows, cols, [&](int i, int j) {
        cons.add_variable(var.has_group[i][j][k], 1);
      });
      cons.commit(group[k].length, group[k].length);
    });
    // No 2x2 block is empty.
    cell_iterator(rows - 1, cols - 1, [&](int i, int j) {
      Constraint cons = mip.constraint();
      cons.add_variable(var.used[i][j], 1);
      cons.add_variable(var.used[i + 1][j], 1);
      cons.add_variable(var.used[i][j + 1], 1);
      cons.add_variable(var.used[i + 1][j + 1], 1);
      cons.commit(1, 4);
    });
    // If a cell is used, either it has or hasn't a group.
    full_iterator(rows, cols, groups, [&](int i, int j, int k) {
      Constraint cons = mip.constraint();
      cons.add_variable(var.used[i][j], 1);
      cons.add_variable(var.has_group[i][j][k], -1);
      cons.add_variable(var.hasnt_group[i][j][k], -1);
      cons.commit(0, 0);
    });
    // Groups can't touch on horizontal.
    full_iterator(rows, cols - 1, groups, [&](int i, int j, int k) {
      Constraint cons = mip.constraint();
      cons.add_variable(var.has_group[i][j][k], 1);
      cons.add_variable(var.hasnt_group[i][j + 1][k], 1);
      cons.commit(0, 1);
    });
    // Groups can't touch on vertical.
    full_iterator(rows - 1, cols, groups, [&](int i, int j, int k) {
      Constraint cons = mip.constraint();
      cons.add_variable(var.has_group[i][j][k], 1);
      cons.add_variable(var.hasnt_group[i + 1][j][k], 1);
      cons.commit(0, 1);
    });
    // An h edge is present if both endpoints are from the same group.
    full_iterator(rows, cols - 1, groups, [&](int i, int j, int k) {
      Constraint cons = mip.constraint();
      cons.add_variable(var.has_group[i][j][k], 1);
      cons.add_variable(var.has_group[i][j + 1][k], 1);
      cons.add_variable(var.edge_h[i][j][k], -2);
      cons.commit(0, 1);
    });
    // A v edge is present if both endpoints are from the same group.
    full_iterator(rows - 1, cols, groups, [&](int i, int j, int k) {
      Constraint cons = mip.constraint();
      cons.add_variable(var.has_group[i][j][k], 1);
      cons.add_variable(var.has_group[i + 1][j][k], 1);
      cons.add_variable(var.edge_v[i][j][k], -2);
      cons.commit(0, 1);
    });
    // Every cell on a group must be on an edge, if group > 1.
    full_iterator(rows, cols, groups, [&](int i, int j, int k) {
      if (group[k].length > 1) {
        Constraint cons = mip.constraint();
        if (j > 0) cons.add_variable(var.edge_h[i][j - 1][k], -1);
        if (j < cols - 1) cons.add_variable(var.edge_h[i][j][k], -1);
        if (i > 0) cons.add_variable(var.edge_v[i - 1][j][k], -1);
        if (i < rows - 1) cons.add_variable(var.edge_v[i][j][k], -1);
        cons.add_variable(var.has_group[i][j][k], 1);
        cons.commit(-4, 0);
      }
    });
    // Each group of size n must have at least n-1 edges.
    group_iterator(groups, [&](int k) {
      Constraint cons = mip.constraint();
      cell_iterator(rows - 1, cols, [&](int i, int j) {
        cons.add_variable(var.edge_v[i][j][k], 1);
      });
      cell_iterator(rows, cols - 1, [&](int i, int j) {
        cons.add_variable(var.edge_h[i][j][k], 1);
      });
      cons.commit(group[k].length - 1, 2 * rows * cols);
    });
    // An empty h edge is present is both endpoints are empty.
    cell_iterator(rows, cols - 1, [&](int i, int j) {
      Constraint cons = mip.constraint();
      cons.add_variable(var.used[i][j], 1);
      cons.add_variable(var.used[i][j + 1], 1);
      cons.add_variable(var.empty_edge_h[i][j], 2);
      cons.commit(1, 2);
    });
    // An empty v edge is present is both endpoints are empty.
    cell_iterator(rows - 1, cols, [&](int i, int j) {
      Constraint cons = mip.constraint();
      cons.add_variable(var.used[i][j], 1);
      cons.add_variable(var.used[i + 1][j], 1);
      cons.add_variable(var.empty_edge_v[i][j], 2);
      cons.commit(1, 2);
    });
    // Every empty cell must have at least 1 empty edge,
    // if there are more than one empty cell.
    int empties = rows * cols;
    group_iterator(groups, [&](int k) {
      empties -= group[k].length;
    });
    if (empties > 1) {
      cell_iterator(rows, cols, [&](int i, int j) {
        Constraint cons = mip.constraint();
        if (j > 0) cons.add_variable(var.empty_edge_h[i][j - 1], 1);
        if (j < cols - 1) cons.add_variable(var.empty_edge_h[i][j], 1);
        if (i > 0) cons.add_variable(var.empty_edge_v[i - 1][j], 1);
        if (i < rows - 1) cons.add_variable(var.empty_edge_v[i][j], 1);
        cons.add_variable(var.used[i][j], 1);
        cons.commit(1, 5);
      });
    }
    // Set the minimum amount of empty edges:
    Constraint empty_cons = mip.constraint();
    cell_iterator(rows - 1, cols, [&](int i, int j) {
      empty_cons.add_variable(var.empty_edge_v[i][j], 1);
    });
    cell_iterator(rows, cols - 1, [&](int i, int j) {
      empty_cons.add_variable(var.empty_edge_h[i][j], 1);
    });
    empty_cons.commit(empties - 1, min(2 * rows * cols, 2 * empties));
    // Mark unreachable cells.
    /*full_iterator(rows, cols, groups, [&](int i, int j, int k) {
      if (manhattan(i, j, group[k].row, group[k].col) >= group[k].length) {
        Constraint cons = mip.constraint();
        cons.add_variable(var.hasnt_group[i][j][k], 1);
        cons.add_variable(var.used[i][j], -1);
        cons.commit(0, 0);
      }
    });*/
    // Pseudo-continuity based on relative unreachables.
    full_iterator(rows, cols, groups, [&](int i, int j, int k) {
      if (!(i == group[k].row && j == group[k].col)) {
        int unreachables = 0;
        Constraint reach_cons = mip.constraint();
        Constraint unreach_cons = mip.constraint();
        bool success = unreachable_iterator(
            rows, cols, i, j, k, [&](int ii, int jj, bool reachable) {
          if (reachable) {
            reach_cons.add_variable(var.has_group[ii][jj][k], 1);          
          } else {
            unreachables++;
            unreach_cons.add_variable(var.has_group[ii][jj][k], 1);          
          }
        });
        if (success) {
          unreach_cons.add_variable(var.has_group[i][j][k], unreachables);
          unreach_cons.commit(0, unreachables);
          reach_cons.add_variable(var.has_group[i][j][k], -(group[k].length - 2));
          reach_cons.commit(0, rows * cols);
        } else {
          Constraint cons = mip.constraint();
          cons.add_variable(var.hasnt_group[i][j][k], 1);
          cons.add_variable(var.used[i][j], -1);
          cons.commit(0, 0);
        }
      }
    });
    // Remove forbidden groups.
    for (auto &g : forbidden) {
      Constraint cons = mip.constraint();
      for (auto &pos : g.pos) {
        cons.add_variable(var.has_group[pos.first][pos.second][g.group_idx], 1);
      }
      cons.commit(0, group[g.group_idx].length - 1);
    }
    // Add a variable for each forbidden empty group.
    for (auto &g : empty_forbidden) {
      Variable empty_group_var = mip.binary_variable(0);
      var.empty_group.push_back(empty_group_var);
      Constraint cons = mip.constraint();
      cons.add_variable(empty_group_var, g.empty.size());
      for (auto &pos : g.empty) {
        cons.add_variable(var.used[pos.first][pos.second], 1);
      }
      cons.commit(1, g.empty.size());
    }
    // Empty group is only allowed if at least one neighbour is empty.
    group_iterator(empty_forbidden.size(), [&](int i) {
      Constraint cons = mip.constraint();
      cons.add_variable(var.empty_group[i], 1);
      for (auto &pos : empty_forbidden[i].border) {
        cons.add_variable(var.used[pos.first][pos.second], 1);
      }
      cons.commit(0, empty_forbidden[i].border.size());
    });
    // Add the dynamic constraint to make groups continuous.
    //mip.add_dynamic_constraint(dynamic_constraint);
  }

  double diff(int row, int col, int idx) {
    return sqrt(sqr(row - group[idx].row) + sqr(col - group[idx].col));
  }

  int manhattan(int i, int j, int ii, int jj) {
    return abs(i - ii) + abs(j - jj);
  }

  bool near(int i, int j, int k) {
    for (int kk = 0; kk < groups; kk++) {
      if (k == kk) continue;
      static const int dx[] = {0, 0, 0, 1, -1};
      static const int dy[] = {0, 1, -1, 0, 0};
      for (int n = 0; n < 5; n++) {
        if (i == group[kk].row + dx[n] && j == group[kk].col + dy[n]) {
          return true;
        }
      }      
    }
    return false;
  }

  template<typename T>
  bool unreachable_iterator(int rows, int cols, int i, int j, int k, T func) {
    if (manhattan(i, j, group[k].row, group[k].col) >= group[k].length) {
      return false;
    }
    vector<vector<int>> value(rows, vector<int>(cols, -1));
    priority_queue<tuple<int, int, int>> next;

    next.push(make_tuple(0, group[k].row, group[k].col));
    value[group[k].row][group[k].col] = 0;
    while (!next.empty()) {
      auto current = next.top();
      int pos = -get<0>(current);
      next.pop();
      int ii = get<1>(current);
      int jj = get<2>(current);
      if (i == ii && j == jj) break;
      neighbour_iterator(rows, cols, ii, jj, [&](int i3, int j3) {
        if (value[i3][j3] < 0 && !near(i3, j3, k)) {
          value[i3][j3] = pos + 1;
          next.push(make_tuple(-value[i3][j3], i3, j3));
        }
      });
    }

    int minlen = value[i][j];
    if (minlen < 0 || minlen >= group[k].length) return false;
    int marker = -rows * cols * 2;
    while (!next.empty()) {
      next.pop();
    }
    next.push(make_tuple(minlen, i, j));
    while (!next.empty()) {
      auto current = next.top();
      next.pop();
      int pos = get<0>(current);
      int ii = get<1>(current);
      int jj = get<2>(current);
      if (pos != value[ii][jj]) continue;
      neighbour_iterator(rows, cols, ii, jj, [&](int i3, int j3) {
        if (value[i3][j3] == value[ii][jj] - 1 && value[i3][j3] >= 0) {
          next.push(make_tuple(value[i3][j3], i3, j3));
        }
      });
      value[ii][jj] = marker;
    }

    cell_iterator(rows, cols, [&](int ii, int jj) {
      if (value[ii][jj] == marker) {
        value[ii][jj] = minlen + 1;
        next.push(make_tuple(-value[ii][jj], ii, jj));
      } else if (value[ii][jj] >= 0) {
        value[ii][jj] = -1;
      }
    });

    while (!next.empty()) {
      auto current = next.top();
      int pos = -get<0>(current);
      next.pop();
      if (pos >= group[k].length) break;
      neighbour_iterator(rows, cols, get<1>(current), get<2>(current), [&](int ii, int jj) {
        if (value[ii][jj] < 0 && !near(ii, jj, k)) {
          value[ii][jj] = pos + 1;
          next.push(make_tuple(-value[ii][jj], ii, jj));
        }
      });
    }

    /*printf("--- size %d\n", group[k].length);
    for (int j2 = 0; j2 < cols; j2++) {
      for (int i2 = 0; i2 < rows; i2++) {
        char c = '.';
        for (int kk = 0; kk < groups; kk++) {
          if (k != kk && i2 == group[kk].row && j2 == group[kk].col) {
            c = '*';
          }
        }
        if (c == '.') {
          if (i == i2 && j == j2) {
            c = 'X';
          } else if (i2 == group[k].row && j2 == group[k].col) {
            c = 'S';
          } else {
            int k = value[i2][j2];
            if (k >= 0) {
              c = bigdigit(k);
            }
          }
        }
        printf("%c", c);
      }
      printf("\n");
    }*/

    cell_iterator(rows, cols, [&](int ii, int jj) {
      if (!(i == ii && j == jj) ||
          !(ii == group[k].row && jj == group[k].col)) {
        func(ii, jj, value[ii][jj] >= 0);
      }
    });
    return true;
  }
};

void print_vector(const vector<pair<int, int>>& vec) {
  for (auto &pos : vec) {
    printf("%d-%d ", pos.first, pos.second);
  }
  printf("\n");
}

struct Nurikabe {
  int rows, cols;
  const vector<Group>& group;
  int groups;
  vector<vector<bool>> visited;
  vector<GroupPosition> forbidden;
  vector<EmptyPosition> empty_forbidden;

 public:
  Nurikabe(int rows_, int cols_, const vector<Group>& group_) 
      : rows(rows_), cols(cols_), group(group_), groups(group.size()),
        visited(rows, vector<bool>(cols)) {}

  void solve() {
    while (true) {
      clear_visited();
      vector<bool> visited_group(groups, false);
      NurikabeMIP mip(rows, cols, group, forbidden, empty_forbidden);
      NurikabeSolution sol = mip.solve();
      int failures = check_solution(sol, visited_group);
      if (!failures) {
        print(sol);
        break;
      }
    }
  }

 private:
  int check_solution(NurikabeSolution& sol, vector<bool>& visited_group) {
    int failures = 0;
    int empties = count_empties(sol);
    
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        int value = sol.pos[i][j];
        if (!visited[i][j] && value == -1) {
          int length = grow(sol, i, j, -1, true);
          if (length != empties) {
            add_empties(sol);
            failures++;
          }
        }
        if (!visited[i][j] && value >= 0 && !visited_group[value]) {
          visited_group[value] = true;
          int length = grow(sol, i, j, value);
          if (length != group[value].length) {
            add_group(sol, value);
            failures++;
          }
        }
      }
    }
    return failures;
  }

  void add_empties(NurikabeSolution& sol) {
    EmptyPosition position;
    set<pair<int, int>> border;
    
    cell_iterator(rows, cols, [&](int i, int j) {
      if (sol.pos[i][j] == -2) {
        position.empty.push_back(make_pair(i, j));
        neighbour_iterator(rows, cols, i, j, [&](int ni, int nj) {
          if (sol.pos[ni][nj] != -2) {
            border.insert(make_pair(ni, nj));
          }
        });
      }
    });
    
    position.border = vector<pair<int, int>>(border.begin(), border.end());
    empty_forbidden.push_back(position);
    
    cell_iterator(rows, cols, [&](int i, int j) {
      if (sol.pos[i][j] == -2) {
        sol.pos[i][j] = -3;
      }
    });
    
    /*
    printf("Empty group: ");
    print_vector(position.empty);
    printf("Border group: ");
    print_vector(position.border);
    */
  }

  int count_empties(const NurikabeSolution& sol) {
    int ans = 0;
    cell_iterator(rows, cols, [&](int i, int j) {
      if (sol.pos[i][j] < 0) {
        ans++;
      }
    });
    return ans;
  }

  int grow(NurikabeSolution& sol, int i, int j, int group_idx, bool mark = false) {
    if (sol.pos[i][j] != group_idx || visited[i][j]) {
      return 0;
    }
    if (mark) {
      sol.pos[i][j] = -2;
    }
    int ans = 1;
    visited[i][j] = true;
    neighbour_iterator(rows, cols, i, j, [&](int ni, int nj) {
      ans += grow(sol, ni, nj, group_idx, mark);
    });
    return ans;
  }

  void add_group(const NurikabeSolution& sol, int group_idx) {
    GroupPosition group_pos;
    group_pos.group_idx = group_idx;
    //printf("Group %d: ", group_idx);
    
    cell_iterator(rows, cols, [&](int i, int j) {
      if (sol.pos[i][j] == group_idx) {
        //printf("%d %d, ", i, j);
        group_pos.pos.push_back(make_pair(i, j));
      }
    });
    
    //printf("\n");
    forbidden.push_back(group_pos);
  }

  void clear_visited() {
    cell_iterator(rows, cols, [&](int i, int j) {
      visited[i][j] = false;
    });
  }

  void print(const NurikabeSolution& sol) {
    // Create group grid
    vector<vector<char>> group_grid(rows, vector<char>(cols));
    cell_iterator(rows, cols, [&](int i, int j) {
      int k = sol.pos[i][j];
      group_grid[i][j] = k >= 0 ? 'a' + k : '.';
    });

    // Set up cell symbols
    std::map<int, std::string> cell_symbols;
    cell_symbols[0] = "\033[34m・\033[0m";  // Blue centered wide dot for empty cells
    for (int i = 0; i < this->groups; i++) {
      string number;
      if (group[i].length < 10) {
        // Use wide digits for single digits
        const char* wide_numbers[] = {"０", "１", "２", "３", "４", "５", "６", "７", "８", "９"};
        number = wide_numbers[group[i].length];
      } else {
        // Use regular digits for numbers 10 and above
        char buf[16];  // Increased buffer size to handle larger numbers safely
        snprintf(buf, sizeof(buf), "%d", group[i].length);
        number = buf;
      }
      // Regular symbol for group numbers
      cell_symbols[i + 1] = number;
      // Yellow symbol for seed positions
      cell_symbols[-(i + 1)] = "\033[33m" + number + "\033[0m";
    }

    // Create and use group printer
    GroupPrinter printer(rows, cols, group_grid, cell_symbols, 2);
    vector<vector<int>> grid(rows, vector<int>(cols));
    cell_iterator(rows, cols, [&](int i, int j) {
      int value = sol.pos[i][j];
      if (value >= 0) {
        // Check if this is a seed position
        if (i == group[value].row && j == group[value].col) {
          grid[i][j] = -(value + 1);  // Use negative index for seed positions
        } else {
          grid[i][j] = value + 1;  // Regular group number
        }
      } else {
        grid[i][j] = 0;  // Water cell
      }
    });
    printer.print(grid);
  }
};

int main() {
  int rows, cols;
  cin >> rows >> cols;
  int groups;
  cin >> groups;
  vector<Group> group(groups);
  for (int i = 0; i < groups; i++) {
    cin >> group[i].row >> group[i].col >> group[i].length;
  }
  Nurikabe nurikabe(rows, cols, group);
  nurikabe.solve();
  return 0;
}
