#include <iostream>
#include <vector>
#include <cctype>
#include <cstdio>
#include <queue>
#include "constraint/constraint.h"

using namespace std;

struct Node {
  int y, x;
  int id;
  vector<VariableId> links;
  Node(int y_, int x_, int id_) : y(y_), x(x_), id(id_) {}
};

struct Link {
  int a, b;
  int id;
  Link(int a_, int b_, int id_) : a(a_), b(b_), id(id_) {}
};

struct Cell {
  int size;
  vector<int> links;
};

class SingleLineConstraint : public ExternalConstraint {
  const vector<Node>& nodes;
  const vector<Link>& links;
 public:
  SingleLineConstraint(
      const vector<Node>& nodes_, const vector<Link>& links_)
      : nodes(nodes_), links(links_) {}
  virtual ~SingleLineConstraint() {}

  virtual bool operator()(const State* state) const {
    for (auto x : links) {
      if (!state->fixed(x.id)) {
        return true;
      }
    }
    int start = -1;
    for (const auto& link : links) {
      if (state->read_lmin(link.id) > 0) {
        start = link.id;
        break;
      }
    }
    if (start < 0) {
      return true;
    }
    vector<bool> visited(links.size(), false);
    visited[start] = true;
    queue<int> next;
    next.push(start);
    vector<int> dump;
    while (!next.empty()) {
      int cur = next.front();
      dump.push_back(cur);
      next.pop();
      for (const auto& link : nodes[links[cur].a].links) {
        if (!visited[link] && state->read_lmax(link) > 0) {
          visited[link] = true;
          next.push(link);
        }
      }
      for (const auto& link : nodes[links[cur].b].links) {
        if (!visited[link] && state->read_lmax(link) > 0) {
          visited[link] = true;
          next.push(link);
        }
      }
    }
    for (const auto& link : links) {
      if (state->read_lmin(link.id) > 0 && !visited[link.id]) {
        /*cout << "--\n";
        for (auto x : dump) {
          cout << x << " ";
        }
        cout << "\n\n";*/
        return false;
      }
    }
    return true;
  }
};

class PointConstraint : public TightenConstraint {
  const vector<VariableId>& links;
 public:
  PointConstraint(const vector<VariableId>& links_) : links(links_) {}
  virtual ~PointConstraint() {}

  virtual const std::vector<VariableId>& get_variables() const {
    return links;
  }

  virtual bool update_constraint(State* state, ConstraintQueue* cqueue) const {
    int fixed = 0;
    int fixedsum = 0;
    for (int link : links) {
      if (state->fixed(link)) {
        fixed++;
        fixedsum += state->read_lmin(link);
      }
    }
    if (fixed == int(links.size()) - 1) {
      if (fixedsum > 2) {
        return false;
      }
      int value = 0;
      if (fixedsum == 1) {
        value = 1;
      }
      for (int link : links) {
        if (!state->fixed(link)) {
          state->change_var(link, value, value);
          cqueue->push_variable(link);
          return true;
        }
      }
    }
    if (fixed < int(links.size())) {
      return true;
    }
    return fixedsum == 0 || fixedsum == 2;
  }
};

class SlitherLinkSolver {
  int width, height;
  const vector<string>& grid;
  ConstraintSolver solver;
  vector<Node> nodes;
  vector<Link> links;
  vector<Cell> cells;
 public:
  SlitherLinkSolver(int width_, int height_, const vector<string>& grid_)
      : width(width_), height(height_), grid(grid_), solver(true) {}

  int getid(int j, int i) {
    return j * (width + 1) + i;
  }

  void degeometrize() {
    for (int j = 0; j < height + 1; j++) {
      for (int i = 0; i < width + 1; i++) {
        nodes.push_back(Node(j, i, nodes.size()));
      }
    }
    vector<vector<vector<int>>> cellpos(height, vector<vector<int>>(width));
    for (int j = 0; j < height + 1; j++) {
      for (int i = 0; i < width; i++) {
        int id = links.size();
        links.push_back(Link(getid(j, i), getid(j, i + 1), id));
        nodes[getid(j, i)].links.push_back(id);
        nodes[getid(j, i + 1)].links.push_back(id);
        if (j > 0) {
          cellpos[j - 1][i].push_back(id);
        }
        if (j < height) {
          cellpos[j][i].push_back(id);
        }
      }
    }
    for (int j = 0; j < height; j++) {
      for (int i = 0; i < width + 1; i++) {
        int id = links.size();
        links.push_back(Link(getid(j, i), getid(j + 1, i), id));
        nodes[getid(j, i)].links.push_back(id);
        nodes[getid(j + 1, i)].links.push_back(id);
        if (i > 0) {
          cellpos[j][i - 1].push_back(id);
        }
        if (i < width) {
          cellpos[j][i].push_back(id);
        }
      }
    }
    for (int j = 0; j < height; j++) {
      for (int i = 0; i < width; i++) {
        if (isdigit(grid[j][i])) {
          Cell cell;
          cell.size = grid[j][i] - '0';
          cell.links = cellpos[j][i];
          cells.push_back(cell);
        }
      }
    }
  }
  
void print_terminal() {
    int H = 2 * height + 1;
    int W = 2 * width + 1;

    // Build canvas with strings (to support Unicode box chars)
    vector<vector<string>> canvas(H, vector<string>(W, " "));

        // Place numbers or dots in every cell
        for (int j = 0; j < height; ++j) {
            for (int i = 0; i < width; ++i) {
                int y = 2*j + 1, x = 2*i + 1;
                if (isdigit(grid[j][i])) {
                    canvas[y][x] = string(1, grid[j][i]);
                } else {
                    canvas[y][x] = "·";  // small dot for empty cell
                }
            }
        }

    // First pass: draw ─ and │ wherever a segment is present
    for (const auto& link : links) {
        if (solver.value(link.id) == 0) continue;

        int y1 = nodes[link.a].y * 2;
        int x1 = nodes[link.a].x * 2;
        int y2 = nodes[link.b].y * 2;
        int x2 = nodes[link.b].x * 2;

        if (y1 == y2) {
            for (int x = min(x1, x2) + 1; x < max(x1, x2); ++x) {
                canvas[y1][x] = "─";
                canvas[y1][x+1] = "─";
                canvas[y1][x-1] = "─";
             }
        } else if (x1 == x2) {
            for (int y = min(y1, y2) + 1; y < max(y1, y2); ++y)
                canvas[y][x1] = "│";
        }
    }

    // Second pass: replace + junctions with proper Unicode corners
    for (int y = 0; y < H; y += 2) {
        for (int x = 0; x < W; x += 2) {
            bool up    = y > 0     && canvas[y - 1][x] == "│";
            bool down  = y < H - 1 && canvas[y + 1][x] == "│";
            bool left  = x > 0     && canvas[y][x - 1] == "─";
            bool right = x < W - 1 && canvas[y][x + 1] == "─";

            int count = up + down + left + right;
            if (count == 0) continue;

            if (count == 1) {
                canvas[y][x] = (up || down) ? "│" : "─";
            } else if (count == 2) {
                if (up && down) canvas[y][x] = "│";
                else if (left && right) canvas[y][x] = "─";
                else if (up && right) canvas[y][x] = "└";
                else if (up && left)  canvas[y][x] = "┘";
                else if (down && right) canvas[y][x] = "┌";
                else if (down && left)  canvas[y][x] = "┐";
            } else if (count == 3) {
                if (!up) canvas[y][x] = "┬";
                else if (!down) canvas[y][x] = "┴";
                else if (!left) canvas[y][x] = "├";
                else if (!right) canvas[y][x] = "┤";
            } else if (count == 4) {
                canvas[y][x] = "┼";
            }
        }
    }

    // Print the final canvas
    for (const auto& row : canvas) {
        for (const auto& cell : row) cout << cell;
        cout << '\n';
    }
  }

  bool solve() {
    for (Link& link: links) {
      link.id = solver.create_variable(0, 1);
    }
    vector<TightenConstraint*> linear;
    for (const Cell& cell : cells) {
      auto cons = new LinearConstraint(cell.size, cell.size);
      for (int link : cell.links) {
        cons->add_variable(link);
      }
      linear.push_back(cons);
      solver.add_constraint(cons);
    }
    for (const Node& node : nodes) {
      auto cons = new LinearConstraint(0, 2);
      for (int link : node.links) {
        cons->add_variable(link);
      }
      linear.push_back(cons);
      solver.add_constraint(cons);
    }
    vector<PointConstraint*> external;
    for (const Node& node : nodes) {
      PointConstraint *pc = new PointConstraint(node.links);
      linear.push_back(pc);
      solver.add_constraint(pc);
    }
    SingleLineConstraint single_line(nodes, links);
    solver.add_external_constraint(&single_line);
    bool result = solver.solve();
    for (auto cons : external) {
      delete cons;
    }
    return result;
  }

  void print() {
    FILE *f = fopen("slither.dot", "wt");
    fprintf(f, "graph {\n");
    for (int j = 0; j < height + 1; j++) {
      for (int i = 0; i < width + 1; i++) {
        fprintf(f, "n%d_%d [label=\"\"\nshape=point\npos=\"%d,%d!\"]\n", 
                j, i, 2 * j, 2 * i);
      }
    }
    for (int j = 0; j < height; j++) {
      for (int i = 0; i < width; i++) {
        if (isdigit(grid[j][i])) {
          fprintf(f, "x%d_%d [label=%c\npos=\"%d,%d!\"]\n",
                  j, i, grid[j][i], 2 * j + 1, 2 * i + 1);
        }
      }
    }
    for (const Link& link : links) {
      if (solver.value(link.id) > 0) {
        fprintf(f, "n%d_%d -- n%d_%d;\n", 
                nodes[link.a].y, nodes[link.a].x,
                nodes[link.b].y, nodes[link.b].x);
      }
    }
   
    fprintf(f, "}\n");
    fclose(f);
  }
};

int main() {
  int width, height;
  cin >> width >> height;
  vector<string> grid(height);
  for (int i = 0; i < height; i++) {
    cin >> grid[i];
  }
  SlitherLinkSolver s(width, height, grid);  
  s.degeometrize();
  if (s.solve()) {
    s.print_terminal();
  }
  return 0;
}
