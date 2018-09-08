#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <optional>
#include <sstream>
#include <map>
#include <numeric>
#include <algorithm>

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
  vector<int> column(int c) {
    vector<int> ans;
    for (int i = 0; i < n; i++) {
      ans.push_back(line[i][c]);
    }
    return ans;
  }
  vector<int> row(int r) const {
    vector<int> ans;
    for (int i = 0; i < n; i++) {
      ans.push_back(line[r][i]);
    }
    return ans;
  }
  void print() {
    for (int j = 0; j < n; j++) {
      for (int i = 0; i < n; i++) {
        cout << line[j][i] << " ";
      }
      cout << "\n";
    }
  }
  vector<int> forward() const {
    vector<int> ans;
    for (int i = 0; i < n * n; i++) {
      ans.push_back(i);
    }
    return ans;
  }
  vector<int> backward() const {
    vector<int> ans;
    for (int i = n * n - 1; i >= 0; i--) {
      ans.push_back(i);
    }
    return ans;
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
  bool has_maybe_value(int value) {
    for (auto &m : maybe) {
      if (m.value == value) {
        return true;
      }
    }
    return false;
  }
  template<typename T>
  Cell filter_maybe(T filter) const {
    Cell cell{{}, value};
    for (auto &m : maybe) {
      if (filter(m)) {
        cell.maybe.insert(m);
      }
    }
    return cell;
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
  bool has_value(const vector<int>& line, int value) {
    for (int i : line) {
      if (pos(i).value == value) {
        return true;
      }
    }
    return false;
  }
  int count_maybe(const vector<int>& line, Maybe &m) {
    int count = 0;
    for (int i : line) {
      if (pos(i).maybe.find(m) != pos(i).maybe.end()) {
        count++;
      }
    }
    return count;
  }
  auto first_non_empty(const vector<int>& order) {
    auto it = order.begin();
    while (pos(*it).maybe.empty()) {
      ++it;
    }
    return it;
  }
  bool done() {
    int count = 0;
    for (int i = 0; i < n * n; i++) {
      if (pos(i).value) {
        count++;
      }
    }
    return count == 3 * n;
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
        if (state.pos(j, i).value) {
          oss << "<div class=\"maybe-values\">";
          oss << state.pos(j, i).maybe_values() << "</div>";
          oss << "<div class=\"value\">";
          oss << state.pos(j, i).print_value() << "</div>";
          oss << "<div class=\"maybe-groups\">";
          oss << state.pos(j, i).maybe_groups() << "</div>";
          oss << "</div></td>";
        } else {
          oss << "<div class=\"maybe-values\">";          
          for (int d = 1; d <= 3; d++) {
            if (state.pos(j, i).has_maybe_value(d)) {
              oss << d << " : ";
              for (int g = 1; g <= path.n; g++) {
                if (state.pos(j, i).maybe.find(Maybe{d, g}) !=
                    state.pos(j, i).maybe.end()) {
                  oss << g << " ";
                }
              }
              oss << "<br>";
            }
          }
        }
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
  bool skip = false;
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
  bool empty() {
    return ans.empty();
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
          int value = grid[j][i] - '0';
          Cell cell = state.pos(j, i).filter_maybe([&](const Maybe &m) {
            return m.value == value;
          });
          cell.value = value;
          filter.put(j, i, cell);
        }
      }
    }
    skip = true;
    return filter.flush();
  }
};

struct RemoveCross : public Strategy {
  int bj, bi;
  RemoveCross(int bj_, int bi_) : bj(bj_), bi(bi_) {
  }
  virtual string name() {
    ostringstream oss;
    oss << "Remove from row " << bj << " and column "<< bi;
    return oss.str();
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
        Cell cell = state.pos(j, i).filter_maybe([&](const Maybe &m) {
          return m.value != *state.pos(bj, bi).value;
        });
        filter.put(j, i, cell);
      }
    }
    if (!filter.empty()) {
      skip = true;
    }
    return filter.flush();
  }
};

struct OnlyValue : public Strategy {
  int digit, group;
  const vector<int> line;
  string line_name;
  int line_pos;
  OnlyValue(int d, int g, const vector<int> line_,
            string line_name_, int pos_)
      : digit(d), group(g), line(line_),
        line_name(line_name_), line_pos(pos_) {
  }
  virtual string name() {
    ostringstream oss;
    oss << "Digit " << digit << " in group " << group;
    oss << " only appears on " << line_name << " " << line_pos;
    return oss.str();
  }
  virtual map<int, Cell> strategy(State &state) {
    Filter filter(state);
    if (state.has_value(line, digit)) {
      skip = true;
      return {};
    }
    Maybe m{digit, group};
    int line_count = state.count_maybe(line, m);
    int grid_count = state.count_maybe(state.path.forward(), m);
    if (line_count != grid_count) {
      return {};
    }
    for (int i : line) {
      Cell cell = state.pos(i).filter_maybe([&](const Maybe &m) {
        return m.value != digit || m.group == group;
      });
      filter.put(i, cell);
    }
    skip = true;
    return filter.flush();
  }
};

struct OnlyGroup : public Strategy {
  int digit, group;
  const vector<int> line;
  string line_name;
  int line_pos;
  OnlyGroup(int d, int g, const vector<int> line_,
            string line_name_, int pos_)
      : digit(d), group(g), line(line_),
        line_name(line_name_), line_pos(pos_) {
  }
  virtual string name() {
    ostringstream oss;
    oss << line_name << " " << line_pos << " only has ";
    oss << digit << " in group " << group;
    return oss.str();
  }
  virtual map<int, Cell> strategy(State &state) {
    Filter filter(state);
    if (state.has_value(line, digit)) {
      skip = true;
      return {};
    }
    Maybe m{digit, group};
    int line_count = state.count_maybe(line, m);
    int count = 0;
    for (int i : line) {
      for (auto &m : state.pos(i).maybe) {
        if (m.value == digit) {
          count++;
        }
      }
    }
    if (line_count != count) {
      return {};
    }
    set<int> skipline(line.begin(), line.end());
    for (int i : state.path.forward()) {
      if (skipline.find(i) == skipline.end()) {
        Cell cell = state.pos(i).filter_maybe([&](const Maybe &m) {
          return !(m.value == digit && m.group == group);
        });
        filter.put(i, cell);
      }
    }
    skip = true;
    return filter.flush();
  }
};

struct SingleLine : public Strategy {
  int digit;
  const vector<int> line;
  string line_name;
  int line_pos;
  SingleLine(int d, const vector<int> line_, string line_name_, int pos_)
      : digit(d), line(line_), line_name(line_name_), line_pos(pos_) {
  }
  virtual string name() {
    ostringstream oss;
    oss << "Single " << digit << " in " << line_name << " " << line_pos;
    return oss.str();
  }
  virtual map<int, Cell> strategy(State &state) {
    Filter filter(state);
    if (state.has_value(line, digit)) {
      skip = true;
      return {};
    }
    int count = 0;
    for (int j : line) {
      if (state.pos(j).has_maybe_value(digit)) {
        count++;
      }
    }
    if (count == 1) {
      for (int j : line) {
        bool has = false;
        set<Maybe> maybe;
        for (auto &m : state.pos(j).maybe) {
          if (m.value == digit) {
            has = true;
            maybe.insert(m);
          }
        }
        if (has) {
          Cell cell{maybe, digit};
          filter.put(j, cell);
          skip = true;
        }
      }
    }
    return filter.flush();
  }
};

template<typename T>
struct SequenceImplication : public Strategy {
  const vector<int> order, reverse;
  T action;
  string dir_name;
  SequenceImplication(const vector<int> order_, T action_, string name_)
    : order(order_), reverse(order.rbegin(), order.rend()),
      action(action_), dir_name(name_) {
  }
  virtual string name() {
    return dir_name + " implication";
  }
  virtual map<int, Cell> strategy(State &state) {
    Filter filter(state);
    auto it = state.first_non_empty(order) + 1;
    for (; it != order.end(); ++it) {
      if (state.pos(*it).value && state.pos(*it).maybe.size() == 1) {
        continue;
      }
      Cell cell = state.pos(*it).filter_maybe([&](const Maybe &m) {
        return search(state, *(it - 1), m);
      });
      filter.put(*it, cell);
    }
    return filter.flush();
  }
  bool search(State &state, int start, const Maybe &m) {
    auto it = find(reverse.begin(), reverse.end(), start);
    auto next = action(m);
    for (; it != reverse.end(); ++it) {
      auto &succ = state.pos(*it).maybe;
      auto &succ_value = state.pos(*it).value;
      if (succ.find(m) != succ.end() && succ_value) {
        return false;
      }
      if (succ.find(next) != succ.end() || succ.find(m) != succ.end()) {
        return true;
      }
      if (succ_value) {
        return false;
      }
    }
    return false;
  }
};

template<typename T>
SequenceImplication<T> *newSequenceImplication(
    const vector<int> order_, T action_, string name_){
  return new SequenceImplication<T>(order_, action_, name_);
}

struct FixEndpoint : public Strategy {
  const vector<int> order;
  const Maybe endpoint;
  string endpoint_name;
  FixEndpoint(const vector<int> order_,
              const Maybe endpoint_, string name_)
      : order(order_), endpoint(endpoint_),
        endpoint_name("Fix " + name_ + " cell") {
  }
  virtual string name() {
    return endpoint_name;
  }
  virtual map<int, Cell> strategy(State &state) {
    Filter filter(state);
    auto it = state.first_non_empty(order); 
    while (!state.pos(*it).has_maybe_value(endpoint.value)) {
      Cell cell{{}, state.pos(*it).value};
      filter.put(*it++, cell);
    }
    Cell cell{set<Maybe>{endpoint}, state.pos(*it).value};
    filter.put(*it, cell);
    return filter.flush();
  }
};

enum struct Status {
  SOLVED,
  CHANGED,
  UNCHANGED
};

struct Snail {
  Snail(int n_, const vector<string>& grid) 
      : n(n_), path(n), state(path), printer(path) {
    easy.push_back(new AddGivens{grid});
    for (int j = 0; j < n; j++) {
      for (int i = 0 ; i < n; i++) {
        easy.push_back(new RemoveCross{j, i});
      }
    }
    for (int d = 1; d <= 3; d++) {
      for (int j = 0; j < n; j++) {
        easy.push_back(new SingleLine{d, path.row(j), "row", j});
        easy.push_back(new SingleLine{d, path.column(j), "column", j});
      }
    }
    hard.push_back(new FixEndpoint(
        path.forward(), Maybe{1, 1}, "first"));
    hard.push_back(new FixEndpoint(
        path.backward(), Maybe{3, n}, "last"));
    hard.push_back(newSequenceImplication(
        path.forward(), [](const Maybe &m){ return m.prev(); },
        "Forward"));
    hard.push_back(newSequenceImplication(
        path.backward(), [](const Maybe &m){ return m.next(); },
        "Backward"));
    for (int d = 1; d <= 3; d++) {
      for (int g = 1; g <= n; g++) {
        for (int i = 0; i < n; i++) {
          hard.push_back(new OnlyValue(d, g, path.row(i), "row", i));
          hard.push_back(new OnlyValue(d, g, path.column(i), "column", i));
          hard.push_back(new OnlyGroup(d, g, path.row(i), "Row", i));
          hard.push_back(new OnlyGroup(d, g, path.column(i), "Column", i));
        }
      }
    }
  }
  void solve() {
    while (true) {
      auto easy_status = round(easy);
      if (easy_status == Status::SOLVED) {
        return;
      }
      if (easy_status == Status::CHANGED) {
        continue;
      }
      if (round(hard) != Status::CHANGED) {
        break;
      }
    }
  }
  Status round(const vector<Strategy*> &strategies) {
    bool changed = false;
    for (auto &s : strategies) {
      if (s->skip) {
        continue;
      }
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
        if (state.done()) {
          return Status::SOLVED;
        }
        changed = true;
      }
    }
    return changed ? Status::CHANGED : Status::UNCHANGED;
  }
  int n;
  Path path;
  State state;
  StatePrinter printer;
  vector<Strategy*> easy, hard;
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
  cout << "td { width: 70px; height: 70px;";
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
