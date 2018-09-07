#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <optional>
#include <sstream>

using namespace std;

struct Maybe {
  int value, group;
  bool operator<(const Maybe& b) const {
    return make_pair(value, group) < make_pair(b.value, b.group);
  }
  Maybe next() const {
    if (value < 3) {
      return Maybe{value + 1, group};
    } else {
      return Maybe{1, group + 1};
    }
  }
  Maybe prev() const {
    if (value > 1) {
      return Maybe{value - 1, group};
    } else {
      return Maybe{3, group - 1};
    }
  }
};

struct Coord {
  int y, x;
};

struct Path {
  Path(int n_): n(n_), grid(n * n), line(n, vector<int>(n)) {
    int miny = -1, maxy = n;
    int minx = -1, maxx = n;
    for (int i = 0; i < n * n; i++) {
      grid[i] = Coord{cury, curx};
      line[cury][curx] = i;
      curx += dx[dir];
      cury += dy[dir];
      if (curx >= maxx) {
        change();
        miny++;
      } else if (curx <= minx) {
        change();
        maxy--;
      } else if (cury >= maxy) {
        change();
        maxx--;
      } else if (cury <= miny) {
        change();
        minx++;
      }
    }
  }
  void change() {
    curx -= dx[dir];
    cury -= dy[dir];
    dir = (dir + 1) % 4;
    curx += dx[dir];
    cury += dy[dir];
  }
  optional<int> move(int j, int i, int dir) {
    int nj = j + dy[dir];
    int ni = i + dx[dir];
    if (ni < 0 || ni >= n || nj < 0 || nj >= n) {
      return {};
    } else {
      return line[nj][ni];
    }
  }
  void print() {
    for (int j = 0; j < n; j++) {
      for (int i = 0; i < n; i++) {
        cout << line[j][i] << " ";
      }
      cout << "\n";
    }
  }
  int n;
  vector<Coord> grid;
  vector<vector<int>> line;
 private:
  int cury = 0, curx = 0;
  int dir = 0;
  constexpr static int dx[4]{1, 0, -1, 0};
  constexpr static int dy[4]{0, 1, 0, -1};
};

struct Cell {
  Cell(int n) {
    for (int i = 1; i <= 3; i++) {
      for (int g = 1; g <= n; g++) {
        maybe.insert(Maybe{i, g});
      }
    }
  }
  string print_value() {
    if (value) {
      if (*value > 0) {
        return string(1, '0' + *value);
      } else {
        return ".";
      }
    } else {
      return "&nbsp;";
    }
  }
  string maybe_values() {
    return print_maybe([](const Maybe &m) {
      return m.value;
    });
  }
  string maybe_groups() {
    return print_maybe([](const Maybe &m) {
      return m.group;
    });
  }
  template<typename T>
  string print_maybe(T action) {
    ostringstream oss;
    set<int> values;
    for (auto &m: maybe) {
      values.insert(action(m));
    }
    for (auto &v: values) {
      oss << v << " ";
    }
    return oss.str();
  }
  set<Maybe> maybe;
  optional<int> value;
};

struct Snail {
  Snail(int n_, const vector<string>& grid) 
      : n(n_), pos_(n * n, Cell(n)), path(n) {
    for (int j = 0; j < n; j++) {
      for (int i = 0; i < n; i++) {
        if (grid[j][i] != '.') {
          pos(j, i).value = grid[j][i] - '0';
          filter_given(pos(j, i));
        }
      }
    }
    pos(0).maybe = set<Maybe>{Maybe{1, 1}};
    pos(n * n - 1).maybe = set<Maybe>{Maybe{3, n}};
    forward_maybe();
    backward_maybe();
    //forward_maybe();
    print_current();
  }
  Cell &pos(int i) {
    return pos_[i];
  }
  Cell &pos(int j, int i) {
    return pos_[path.line[j][i]];
  }
  void filter_given(Cell &cell) {
    set<Maybe> maybe_copy(cell.maybe.begin(), cell.maybe.end());
    for (auto &m : maybe_copy) {
      if (m.value != cell.value) {
        cell.maybe.erase(m);
      }
    }
  }
  bool search_backwards(int start, const Maybe &m) {
    for (int j = start; j >= 0; j--) {
      auto &prev = pos(j).maybe;
      auto &prev_value = pos(j).value;
      if (prev.find(m) != prev.end() && prev_value) {
        return false;
      }
      if (prev.find(m.prev()) != prev.end() || prev.find(m) != prev.end()) {
        return true;
      }
      if (!prev_value) {
        return false;
      }
    }
    return false;
  }
  bool search_forwards(int start, const Maybe &m) {
    for (int j = start; j < n * n; j++) {
      auto &prev = pos(j).maybe;
      auto &prev_value = pos(j).value;
      if (prev.find(m) != prev.end() && prev_value) {
        return false;
      }
      if (prev.find(m.next()) != prev.end() || prev.find(m) != prev.end()) {
        return true;
      }
      if (!prev_value) {
        return false;
      }
    }
    return false;
  }
  void forward_maybe() {
    for (int i = 1; i < n * n; i++) {
      auto &cur = pos(i).maybe;
      set<Maybe> maybe_copy(cur.begin(), cur.end());
      for (auto &m : maybe_copy) {
        if (!search_backwards(i - 1, m)) {
          cur.erase(m);
        }
      }
    }
  }
  void backward_maybe() {
    for (int i = n * n - 2; i >= 0; i--) {
      auto &cur = pos(i).maybe;
      set<Maybe> maybe_copy(cur.begin(), cur.end());
      for (auto &m : maybe_copy) {
        if (!search_forwards(i + 1, m)) {
          cur.erase(m);
        }
      }
    }
  }
  string border(int j, int i) {
    ostringstream oss;
    const static string name[4]{"right", "bottom", "left", "top"};
    for (int d = 1; d <= 4; d++) {
      auto coord = path.move(j, i, d % 4);
      oss << "border-" << name[d % 4] << "-width: ";
      oss << (coord && (abs(*coord - path.line[j][i]) == 1) ? 1 : 3);
      oss << "px;";
    }
    return oss.str();
  }
  void print_current() {
    cout << "<table>";
    for (int j = 0; j < n; j++) {
      cout << "<tr>";
      for (int i = 0; i < n; i++) {
        cout << "<td style=\"border-style: solid; border-color: black;";
        cout << border(j, i) << "\"><div class=\"content\">";
        cout << "<div class=\"maybe-values\">";
        cout << pos(j, i).maybe_values() << "</div>";
        cout << "<div class=\"value\">";
        cout << pos(j, i).print_value() << "</div>";
        cout << "<div class=\"maybe-groups\">";
        cout << pos(j, i).maybe_groups() << "</div>";
        cout << "</div></td>";
      }
      cout << "</tr>";
    }
    cout << "</table>";
  }
  int n;
  vector<Cell> pos_;
  Path path;
};

int main() {
  int n;
  cin >> n;
  vector<string> grid(n);
  for (int i = 0; i < n; i++) {
    cin >> grid[i];
  }
  cout << "<html>";
  cout << "<head><style>";
  cout << "table { border-collapse: collapse; }\n";
  cout << "td { width: 50px; height: 50px;";
  cout << "     margin: 0px; text-align: center}\n";
  cout << ".content {display: flex; flex-direction: column;";
  cout << "     justify-content: space-between;}\n";
  cout << ".maybe-values, .maybe-groups {";
  cout << "     font-size: 12px; font-family: sans-serif;}\n";
  cout << ".value {font-size: 30px; font-family: sans-serif;";
  cout << "        font-weigth: bold}\n";
  cout << ".maybe-values {color: green;}\n";
  cout << ".maybe-groups {color: brown;}\n";
  cout << "</style></head><body>";
  Snail snail(n, grid);
  cout << "</body></html>";
  return 0;
}
