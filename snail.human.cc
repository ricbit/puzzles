#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <optional>
#include <sstream>
#include <map>

using namespace std;

struct Maybe {
  int value, group;
  bool operator<(const Maybe &b) const {
    return make_pair(value, group) < make_pair(b.value, b.group);
  }
  bool operator==(const Maybe &b) const {
    return make_pair(value, group) == make_pair(b.value, b.group);
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
  optional<int> move(int j, int i, int dir) const {
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
  set<Maybe> maybe;
  optional<int> value;
  Cell(int n) {
    for (int i = 1; i <= 3; i++) {
      for (int g = 1; g <= n; g++) {
        maybe.insert(Maybe{i, g});
      }
    }
  }
  Cell(set<Maybe> maybe_, optional<int> value_)
      : maybe(maybe_), value(value_) {
  }
  bool operator==(const Cell& b) const {
    return value == b.value &&
        equal(maybe.begin(), maybe.end(), b.maybe.begin(), b.maybe.end());
  }
  bool operator!=(const Cell& b) const {
    return !operator==(b);
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
};

struct State {
  State(const Path &path_)
      : n(path_.n), pos_(n * n, Cell(n)), path(path_) {
  }
  Cell &pos(int i) {
    return pos_[i];
  }
  Cell &pos(int j, int i) {
    return pos_[path.line[j][i]];
  }
  int n;
  vector<Cell> pos_;
  const Path &path;
};

struct StatePrinter {
  StatePrinter(const Path &path_) : path(path_) {
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
  string print(State &state, const map<int, Cell> &changed) {
    ostringstream oss;
    oss << "<div><table>";
    for (int j = 0; j < path.n; j++) {
      oss << "<tr>";
      for (int i = 0; i < path.n; i++) {
        oss << "<td style=\"border-style: solid; border-color: black;";
        oss << border(j, i);
        if (changed.find(state.path.line[j][i]) != changed.end()) {
          oss << "background-color: yellow;";
        }
        oss << "\"><div class=\"content\">";
        oss << "<div class=\"maybe-values\">";
        oss << state.pos(j, i).maybe_values() << "</div>";
        oss << "<div class=\"value\">";
        oss << state.pos(j, i).print_value() << "</div>";
        oss << "<div class=\"maybe-groups\">";
        oss << state.pos(j, i).maybe_groups() << "</div>";
        oss << "</div></td>";
      }
      oss << "</tr>";
    }
    oss << "</table></div>\n";
    return oss.str();
  }
  const Path &path;
};

struct Strategy {
  virtual string name() = 0;
  virtual map<int, Cell> strategy(State &state) = 0;
};

struct Filter {
  Filter(State &state_) : state(state_) {
  }
  void put(int j, int i, Cell &cell) {
    if (state.pos(j, i) != cell) {
      ans.insert(make_pair(state.path.line[j][i], cell));
    }
  }
  void put(int i, Cell &cell) {
    if (state.pos(i) != cell) {
      ans.insert(make_pair(i, cell));
    }
  }
  map<int, Cell> flush() {
    return ans;
  }
  State &state;
  map<int, Cell> ans;
};

struct AddGivens : public Strategy {
  const vector<string> &grid;
  AddGivens(const vector<string> &grid_) : grid(grid_) {
  }
  virtual string name() {
    return "Add givens";
  }
  virtual map<int, Cell> strategy(State &state) {
    Filter filter(state);
    for (int j = 0; j < state.n; j++) {
      for (int i = 0; i < state.n; i++) {
        if (grid[j][i] != '.') {
          Cell cell{{}, grid[j][i] - '0'};
          for (auto &m : state.pos(j, i).maybe) {
            if (m.value == *cell.value) {
              cell.maybe.insert(m);
            }
          }
          filter.put(j, i, cell);
        }
      }
    }
    return filter.flush();
  }
};

struct RemoveCross : public Strategy {
  int bj, bi;
  RemoveCross(int bj_, int bi_) : bj(bj_), bi(bi_) {
  }
  virtual string name() {
    return "Remove from rows and columns";
  }
  virtual map<int, Cell> strategy(State &state) {
    Filter filter(state);
    if (!state.pos(bj, bi).value) {
      return {};
    }
    for (int j = 0; j < state.n; j++) {
      for (int i = 0; i < state.n; i++) {
        if ((j == bj && i == bi) || (j != bj && i != bi)) {
          continue;
        }
        Cell cell{{}, state.pos(j, i).value};
        for (auto &m : state.pos(j, i).maybe) {
          if (m.value != *state.pos(bj, bi).value) {
            cell.maybe.insert(m);
          }
        }
        filter.put(j, i, cell);
      }
    }
    return filter.flush();
  }
};

struct FixFirst : public Strategy {
  virtual string name() {
    return "Fix first cell";
  }
  virtual map<int, Cell> strategy(State &state) {
    Filter filter(state);
    Cell cell{set<Maybe>{Maybe{1, 1}}, state.pos(0).value};
    filter.put(0, cell);
    return filter.flush();
  }
};

struct FixLast : public Strategy {
  virtual string name() {
    return "Fix last cell";
  }
  virtual map<int, Cell> strategy(State &state) {
    Filter filter(state);
    int last = state.n * state.n - 1;
    Cell cell{
        set<Maybe>{Maybe{3, state.n}},
        state.pos(last).value};
    filter.put(last, cell);
    return filter.flush();
  }
};

struct Snail {
  Snail(int n_, const vector<string>& grid) 
      : n(n_), path(n), state(path), printer(path) {
    /*for (int j = 0; j < n; j++) {
      for (int i = 0; i < n; i++) {
        if (grid[j][i] != '.') {
          state.pos(j, i).value = grid[j][i] - '0';
          filter_given(state.pos(j, i));
        }
      }
    }
    state.pos(0).maybe = set<Maybe>{Maybe{1, 1}};
    state.pos(n * n - 1).maybe = set<Maybe>{Maybe{3, n}};
    forward_maybe();
    backward_maybe();
    //forward_maybe();
    */
    strategies.push_back(new AddGivens{grid});
    strategies.push_back(new FixFirst);
    strategies.push_back(new FixLast);
    for (int j = 0; j < n; j++) {
      for (int i = 0 ; i < n; i++) {
        strategies.push_back(new RemoveCross{j, i});
      }
    }
  }
  void solve() {
    for (auto &s : strategies) {
      auto ans = s->strategy(state);
      if (!ans.empty()) {
        cout << "<div class=\"strategy\"><div>" << s->name() << "</div>";
        cout << "<div class=\"compare\">";
        cout << printer.print(state, ans);
        for (auto &x : ans) {
          state.pos(x.first) = x.second;
        }
        cout << printer.print(state, ans);
        cout << "</div></div><hr>";
      }
    }
  }
  bool search_backwards(int start, const Maybe &m) {
    for (int j = start; j >= 0; j--) {
      auto &prev = state.pos(j).maybe;
      auto &prev_value = state.pos(j).value;
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
      auto &prev = state.pos(j).maybe;
      auto &prev_value = state.pos(j).value;
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
      auto &cur = state.pos(i).maybe;
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
      auto &cur = state.pos(i).maybe;
      set<Maybe> maybe_copy(cur.begin(), cur.end());
      for (auto &m : maybe_copy) {
        if (!search_forwards(i + 1, m)) {
          cur.erase(m);
        }
      }
    }
  }
  int n;
  Path path;
  State state;
  StatePrinter printer;
  vector<Strategy*> strategies;
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
  cout << "table { border-collapse: collapse; margin-bottom: 20px;";
  cout << "        margin-top: 20px; margin-right: 20px;}\n";
  cout << "td { width: 50px; height: 50px;";
  cout << "     margin: 0px; text-align: center}\n";
  cout << ".strategy { margin-top: 20px; ";
  cout << "    display: flex; flex-direction: column; font-size: 20px}\n";
  cout << ".compare {display: flex;}\n";
  cout << ".content {display: flex; flex-direction: column;";
  cout << "     justify-content: space-between;}\n";
  cout << ".maybe-values, .maybe-groups {";
  cout << "     font-size: 12px; font-family: sans-serif;}\n";
  cout << ".value {font-size: 30px; font-family: sans-serif;";
  cout << "        font-weigth: bold}\n";
  cout << ".maybe-values {color: green;}\n";
  cout << ".maybe-groups {color: brown;}\n";
  cout << "</style></head><body>\n";
  Snail snail(n, grid);
  snail.solve();
  cout << "</body></html>";
  return 0;
}
