#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <set>
#include <cstdio>
#include <limits>
#include <queue>
#include "constraint/constraint.h"

using namespace std;

struct Node {
  int x, y;
  int size;
  int id;
  set<int> links;
  Node(int x_, int y_, int size_, int id_)
      : x(x_), y(y_), size(size_), id(id_) {}
};

struct Link {
  int a, b;
  bool horizontal;
  int id;
  set<int> forbidden;
  Link(int a_, int b_, bool horizontal_, int id_)
      : a(a_), b(b_), horizontal(horizontal_), id(id_) {}
};

class SingleGroupConstraint : public ExternalConstraint {
  const vector<Node>& nodes;
  const vector<Link>& links;
 public:
  SingleGroupConstraint(const vector<Node>& nodes_, const vector<Link>& links_)
      : nodes(nodes_), links(links_) {}

  virtual bool operator()(const State* state) const {
    vector<bool> visited(nodes.size(), false);
    queue<int> next;
    next.push(0);
    visited[0] = true;
    while (!next.empty()) {
      int cur = next.front();
      next.pop();
      for (int ilink : nodes[cur].links) {
        const auto& link = links[ilink];
        int other = link.a == cur ? link.b : link.a;
        if (!visited[other] && state->read_lmax(ilink) > 0) {
          visited[other] = true;
          next.push(other);
        }
      }
    }
    for (bool v : visited) {
      if (!v) {
        return false;
      }
    }
    return true;
  }
};

class NoCrossConstraint : public ExternalConstraint {
  const vector<Link>& links;
 public:
  NoCrossConstraint(const vector<Link>& links_) 
      : links(links_) {}

  virtual bool operator()(const State* state) const {
    for (const auto& link : links) {
      for (int other : link.forbidden) {
        if (state->read_lmin(link.id) > 0 &&
            state->read_lmin(other) > 0) {
          return false;
        }
      }
    }
    return true;
  }
};

class HashiSolver {
 private:
  int width, height;
  const vector<string>& grid;
  vector<Node> nodes;
  vector<Link> links;
  ConstraintSolver solver;

 public:
  HashiSolver(int width_, int height_, const vector<string>& grid_)
      : width(width_), height(height_), grid(grid_), solver(true) {
  }

  template<typename T>
  void for_all_digits(T callback) {
    for (int j = 0; j < height; j++) {
      for (int i = 0; i < width; i++) {
        if (isdigit(grid[j][i])) {
          callback(j, i);
        }
      }
    }
  }

  void degeometrize() {
    vector<vector<int>> id(height, vector<int>(width, -1));

    for_all_digits([&](int j, int i) {
      id[j][i] = nodes.size();
      nodes.push_back(Node(i, j, grid[j][i] - '0', nodes.size()));
    });

    for_all_digits([&](int j, int i) {
      for (int ii = i + 1; ii < width; ii++) {
        if (isdigit(grid[j][ii])) {
          links.push_back(Link(id[j][i], id[j][ii], true, links.size()));
          break;
        }
      }
      for (int jj = j + 1; jj < height; jj++) {
        if (isdigit(grid[jj][i])) {
          links.push_back(Link(id[j][i], id[jj][i], false, links.size()));
          break;
        }
      }
    });

    for (auto& l1 : links) {
      if (l1.horizontal) {
        int y = nodes[l1.a].y;
        for (const auto& l2 : links) {
          if (!l2.horizontal) {
            int x = nodes[l2.a].x;
            if (y > nodes[l2.a].y && y < nodes[l2.b].y &&
                x > nodes[l1.a].x && x < nodes[l1.b].x) {
              l1.forbidden.insert(l2.id);
            }
          }
        }        
      }
    }

    for (const auto& link : links) {
      nodes[link.a].links.insert(link.id);
      nodes[link.b].links.insert(link.id);
    }
  }

  void solve() {
    for (auto& link : links) {
      link.id = solver.create_variable(0, 2);
    }
    vector<LinearConstraint*> linear;
    for (const auto& n : nodes) {
      auto cons = new LinearConstraint(n.size, n.size);
      for (auto link : n.links) {
        cons->add_variable(links[link].id);
      }
      linear.push_back(cons);
      solver.add_constraint(cons);
    }
    if (nodes.size() > 2) {
      for (const auto& link : links) {
        if (nodes[link.a].size == nodes[link.b].size &&
            nodes[link.a].size <= 2) {
          int size = nodes[link.a].size;
          auto cons = new LinearConstraint(0, size - 1);
          cons->add_variable(link.id);
          solver.add_constraint(cons);
        }
      }
    }
    NoCrossConstraint no_cross(links);
    solver.add_external_constraint(&no_cross);
    SingleGroupConstraint single_group(nodes, links);
    solver.add_external_constraint(&single_group);
    solver.solve();
  }

  void print() {
    FILE *f = fopen("hashi.dot", "wt");
    fprintf(f, "graph {\n");
    for (const auto& n : nodes) {
      fprintf(f, "n%d_%d [label=%d\npos=\"%d,%d!\"]\n",
              n.id, n.size, n.size, n.x, height - n.y - 1);
    }
    for (const auto& link : links) {
      for (int i = 1; i <= solver.value(link.id); i++) {
        fprintf(f, "n%d_%d -- n%d_%d;\n", 
                link.a, nodes[link.a].size,
                link.b, nodes[link.b].size);
      }
    }
    fprintf(f, "}\n");
    fclose(f);
  }

  void print_terminal() {
    const string circled_digits[10] = { "⓪", "①", "②", "③", "④", "⑤", "⑥", "⑦", "⑧", "⑨" };
    vector<vector<string>> canvas(2 * height - 1, vector<string>(2 * width - 1, "  "));

    // Place nodes
    for (const auto& n : nodes) {
        canvas[2 * n.y][2 * n.x] = circled_digits[n.size] + " ";
    }

    // Place bridges (fill every cell between nodes!)
    for (const auto& link : links) {
        int count = solver.value(link.id);
        if (count == 0) continue;

        int x1 = 2 * nodes[link.a].x;
        int y1 = 2 * nodes[link.a].y;
        int x2 = 2 * nodes[link.b].x;
        int y2 = 2 * nodes[link.b].y;

        if (x1 == x2) {
            // Vertical
            for (int y = min(y1, y2) + 1; y < max(y1, y2); ++y) {
                if (canvas[y][x1] == "  ")
                    canvas[y][x1] = (count == 1) ? " │" : " ║";
            }
        } else if (y1 == y2) {
            // Horizontal
            for (int x = min(x1, x2) + 1; x < max(x1, x2); ++x) {
                if (canvas[y1][x] == "  ")
                    canvas[y1][x] = (count == 1) ? "──" : "══";
            }
        }
    }

    // Print it
    for (const auto& row : canvas) {
        for (const auto& cell : row)
            cout << cell;
        cout << '\n';
    }
  }
};


int main() {
  int width, height;
  cin >> width;
  cin >> height;
  vector<string> grid(height);
  for (int i = 0; i < height; i++) {
    cin >> grid[i];
  }
  HashiSolver s(width, height, grid);
  s.degeometrize();
  s.solve();
  s.print_terminal();
  return 0;
}
