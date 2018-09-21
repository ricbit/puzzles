#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <optional>
#include <sstream>
#include <map>
#include <numeric>
#include <algorithm>
#include <iterator>
#include <utility>
#include <memory>

using namespace std;
using namespace rel_ops;

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
  const int n;
  const vector<Coord> grid;
  const vector<vector<int>> line;
  const vector<vector<int>> row, column;
  const vector<int> forward, backward;
  int modinc(int a) const {
    return 1 + ((a - 1) + 1) % 3;
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
  constexpr static int dx[4]{1, 0, -1, 0};
  constexpr static int dy[4]{0, 1, 0, -1};
};

struct PathBuilder {
  PathBuilder(int n): n(n), grid(n * n), line(n, vector<int>(n)),
      row(n), column(n) {
  }
  Path build() {
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
    for (int i = 0; i < n; i++) {
      row[i] = build_row(i);
      column[i] = build_column(i);
    }
    return Path{n, grid, line, row, column, build_forward(), build_backward()};
  }
 private:
  void change() {
    curx -= dx[dir];
    cury -= dy[dir];
    dir = (dir + 1) % 4;
    curx += dx[dir];
    cury += dy[dir];
  }
  void print() const {
    for (int j = 0; j < n; j++) {
      for (int i = 0; i < n; i++) {
        cout << line[j][i] << " ";
      }
      cout << "\n";
    }
  }
  const vector<int> build_forward() const {
    vector<int> ans;
    for (int i = 0; i < n * n; i++) {
      ans.push_back(i);
    }
    return ans;
  }
  const vector<int> build_backward() const {
    vector<int> ans;
    for (int i = n * n - 1; i >= 0; i--) {
      ans.push_back(i);
    }
    return ans;
  }
  vector<int> build_column(int c) const {
    vector<int> ans;
    for (int i = 0; i < n; i++) {
      ans.push_back(line[i][c]);
    }
    return ans;
  }
  vector<int> build_row(int r) const {
    vector<int> ans;
    for (int i = 0; i < n; i++) {
      ans.push_back(line[r][i]);
    }
    return ans;
  }
  const int n;
  vector<Coord> grid;
  vector<vector<int>> line;
  vector<vector<int>> row, column;
  int cury = 0, curx = 0;
  int dir = 0;
  constexpr static int dx[4]{1, 0, -1, 0};
  constexpr static int dy[4]{0, 1, 0, -1};
};

struct MaybeSet {
  set<Maybe> maybe;
  set<Maybe> &operator()() {
    return maybe;
  }
  const set<Maybe> &operator()() const {
    return maybe;
  }
  bool operator==(const MaybeSet &b) const {
    return equal(maybe.begin(), maybe.end(), b.maybe.begin(), b.maybe.end());
  }
  bool has_value(int v) const {
    return maybe.end() != find_if(maybe.begin(), maybe.end(), [&](auto &m) {
      return m.value == v;
    });
  }
  bool has_maybe(const Maybe &m) const {
    return maybe.find(m) != maybe.end();
  }
  bool empty() const {
    return maybe.empty();
  }
  template<typename T>
  MaybeSet filter(T action) const {
    set<Maybe> ans;
    copy_if(begin(maybe), end(maybe), inserter(ans, begin(ans)), action);
    return MaybeSet{ans};
  }
  void fill(int d, int n) {
    for (int i = 1; i <= d; i++) {
      for (int g = 1; g <= n; g++) {
        maybe.insert(Maybe{i, g});
      }
    }
  }
  auto size() const {
    return maybe.size();
  }
  template<typename T>
  auto count(T action) const {
    return count_if(begin(maybe), end(maybe), action);
  }
};

struct Cell {
  MaybeSet maybe;
  optional<int> value;
  MaybeSet head, tail;
  Cell() = default;
  Cell(int n) {
    maybe.fill(3, n);
  }
  Cell(MaybeSet newmaybe, optional<int> value)
      : maybe(move(newmaybe)), value(value) {
    if (value.has_value()) {
      head = maybe;
      tail = maybe;
    }
  }
  bool operator==(const Cell& b) const {
    return value == b.value && maybe == b.maybe;
  }
  bool empty() const {
    return maybe.empty();
  }
  template<typename T>
  Cell filter_maybe(optional<int> new_value, T filter) const {
    return Cell{maybe.filter(filter), new_value};
  }
  template<typename T>
  Cell filter_maybe(T filter) const {
    return filter_maybe(value, filter);
  }
  template<typename T>
  set<int> get_maybe(T action) const {
    set<int> seq;
    transform(maybe().begin(), maybe().end(), inserter(seq, seq.begin()), action);
    return seq;
  }
  bool found() const {
    return value && maybe.size() == 1;
  }
};

struct CellPrinter {
  string maybe_values(const Cell &cell) const {
    return print_values(cell.get_maybe([](auto &m) {
      return m.value;
    }));
  }
  string maybe_groups(const Cell &cell) const {
    return format_sequence(cell.get_maybe([](auto &m) {
      return m.group;
    }));
  }
  string print_value(const Cell& cell) const {
    if (cell.value) {
      return string(1, '0' + *cell.value);
    } else {
      return "&nbsp;";
    }
  }
  template<class T>
  string print_values(const T &values) const {
    ostringstream oss;
    for (auto &v : values) {
      oss << v << " ";
    }
    return oss.str();
  }
  string groups_from_value(const MaybeSet &maybes, int v) const {
    vector<int> groups;
    for (auto &m: maybes.maybe) {
      if (m.value == v) {
        groups.push_back(m.group);
      }
    }
    return format_sequence(groups);
  }
  template<class T>
  string format_sequence(const T& seq) const {
    ostringstream oss;
    int start = -1, current = -1;
    for (int i : seq) {
      if (start == -1) {
        start = i;
        current = i;
      } else if (i == current + 1) {
        current = i;
      } else {
        dump_sequence(start, current, oss);
        start = i;
        current = i;
      }
    }
    dump_sequence(start, current, oss);
    return oss.str();
  }
  void dump_sequence(int start, int current, ostringstream &oss) const {
    if (current > start) {
      oss << start << "-" << current << " ";
    } else {
      oss << start << " ";
    }
  }
};

struct State {
  State(const Path &path)
      : n(path.n), pos_(n * n, Cell(n)), path(path) {
  }
  const Cell &pos(int i) const {
    return pos_[i];
  }
  const Cell &pos(int j, int i) const {
    return pos_[path.line[j][i]];
  }
  Cell &pos(int i) {
    return pos_[i];
  }
  Cell &pos(int j, int i) {
    return pos_[path.line[j][i]];
  }
  bool has_value(const vector<int>& line, int value) const {
    return line.end() != find_if(line.begin(), line.end(), [&](int i) {
      return pos(i).value == value;
    });
  }
  int count_maybe(const vector<int>& line, const Maybe &m) const {
    return count_if(line.begin(), line.end(), [&](int i) {
      return pos(i).maybe.has_maybe(m);
    });
  }
  auto first_non_empty(const vector<int>& order) const {
    return find_if(order.begin(), order.end(), [&](int i) {
      return !pos(i).empty();
    });
  }
  bool has_single_digit(const vector<int>& line, int digit) const {
    return 1 == count_if(line.begin(), line.end(), [&](auto i) {
      return pos(i).maybe.has_value(digit);
    });
  }
  template<typename T>
  void iter_grid(T action) const {
    for (int j = 0; j < n; j++) {
      for (int i = 0; i < n; i++) {
        action(j, i);
      }
    }
  }
  bool is_sequence(const vector<int>& line) const {
    set<int> sorted(line.begin(), line.end());
    int current = -1;
    for (int i : sorted) {
      if (!pos(i).empty()) {
        if (current == -1) {
          current = i;
        } else if (i == current + 1) {
          current = i;
        } else {
          return false;
        }
      }
    }
    return true;
  }
  bool has_before(int start, const Maybe &m) const {
    for (int i = 0; i < start; i++) {
      if (pos(i).maybe.has_maybe(m)) {
        return true;
      }
    }
    return false;
  }
  bool done() const {
    return 3 * n == count_if(begin(pos_), end(pos_), [](auto &cell) {
      return cell.value.has_value();
    });
  }
  const int n;
  vector<Cell> pos_;
  const Path &path;
};

struct StatePrinter {
  StatePrinter(const Path &path) : path(path), printer() {
  }
  string border(int j, int i) const {
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
  string print(const State &state, const map<int, Cell> &changed) const {
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
        if (!state.pos(j, i).head.empty()) {
          oss << "<div class=\"headtail\">";
          oss << print_maybes(state.pos(j, i).head);
          oss << "</div>";
        }
        if (state.pos(j, i).value) {
          oss << "<div class=\"value\">";
          oss << printer.print_value(state.pos(j, i)) << "</div>";
          oss << "<div class=\"maybe-groups\">";
          oss << printer.maybe_groups(state.pos(j, i)) << "</div>";
        } else {
          oss << print_maybes(state.pos(j, i).maybe);
        }
        if (!state.pos(j, i).tail.empty()) {
          oss << "<div class=\"headtail\">";
          oss << print_maybes(state.pos(j, i).tail);
          oss << "</div>";
        }
        oss << "</div></td>";
      }
      oss << "</tr>";
    }
    oss << "</table></div>\n";
    return oss.str();
  }
  string print_maybes(const MaybeSet &maybes) const {
    ostringstream oss;
    oss << "<div class=\"maybe-values\">";
    for (int d = 1; d <= 3; d++) {
      if (maybes.has_value(d)) {
        oss << d << " : <span class=\"maybe-groups\">";
        oss << printer.groups_from_value(maybes, d);
        oss << "</span><br>";
      }
    }
    oss << "</div>";
    return oss.str();
  }
  const Path &path;
  const CellPrinter printer;
};

struct Strategy {
  virtual string name() = 0;
  virtual map<int, Cell> strategy(const State &state) = 0;
  virtual ~Strategy() = default;
  bool skip = false;
};

struct Filter {
  const State &state;
  map<int, Cell> ans;
  void put(int j, int i, Cell &&cell) {
    if (state.pos(j, i) != cell) {
      ans.insert(make_pair(state.path.line[j][i], cell));
    }
  }
  void put(int i, Cell &&cell) {
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
  void print() {
    for (auto &x : ans) {
      cout << x.first << " ";
    }
    cout << "<br>";
  }
};

struct AddGivens final : public Strategy {
  const vector<string> &grid;
  AddGivens(const vector<string> &grid) : grid(grid) {
  }
  string name() override {
    return "Add givens";
  }
  map<int, Cell> strategy(const State &state) override {
    Filter filter{state};
    state.iter_grid([&](int j, int i) {
      if (grid[j][i] != '.') {
        int value = grid[j][i] - '0';
        filter.put(j, i, state.pos(j, i).filter_maybe(
            value, [&](const Maybe &m) {
          return m.value == value;
        }));
      }
    });
    skip = true;
    return filter.flush();
  }
};

struct RemoveCross final : public Strategy {
  int bj, bi;
  RemoveCross(int bj, int bi) : bj(bj), bi(bi) {
  }
  string name() override {
    ostringstream oss;
    oss << "Remove from row " << bj << " and column "<< bi;
    return oss.str();
  }
  map<int, Cell> strategy(const State &state) override {
    Filter filter{state};
    if (!state.pos(bj, bi).value) {
      return {};
    }
    state.iter_grid([&](int j, int i) {
      if ((j != bj || i != bi) && (j == bj || i == bi)) {
        filter.put(j, i, state.pos(j, i).filter_maybe([&](const Maybe &m) {
          return m.value != *state.pos(bj, bi).value;
        }));
      }
    });
    if (!filter.empty()) {
      skip = true;
    }
    return filter.flush();
  }
};

struct DuplicateGroup final : public Strategy {
  int digit, group;
  DuplicateGroup(int d, int g) : digit(d), group(g) {
  }
  string name() override {
    ostringstream oss;
    oss << "Digit " << digit << " from group ";
    oss << group << " was already found";
    return oss.str();
  }
  map<int, Cell> strategy(const State &state) override {
    Filter filter{state};
    Maybe goal{digit, group};
    const vector<int> &order = state.path.forward;
    auto it = find_if(order.begin(), order.end(), [&](int i) {
      return state.pos(i).found() && state.pos(i).maybe.has_maybe(goal);
    });
    if (it == order.end()) {
      return {};
    }
    int ri = distance(order.begin(), it);
    for (int i = 0; i < state.n * state.n; i++) {
      if (i != ri) {
        filter.put(i, state.pos(i).filter_maybe([&](auto &m) {
          return goal != m;
        }));
      }
    }
    skip = true;
    return filter.flush();
  }
};

struct LimitSequence final : public Strategy {
  const vector<int> line;
  string line_name;
  int line_pos;
  LimitSequence(const vector<int> line, string line_name, int pos)
      : line(line), line_name(line_name), line_pos(pos) {
  }
  string name() override {
    ostringstream oss;
    oss << "Can't have more than 3 digits in " << line_name << " " << line_pos;
    return oss.str();
  }
  map<int, Cell> strategy(const State &state) override {
    if (!state.is_sequence(line)) {
      return {};
    }
    Filter filter{state};
    set<Maybe> maybes;
    int start = state.n * state.n;
    for (int i : line) {
      for (auto &m : state.pos(i).maybe()) {
        maybes.insert(m);
      }
      if (!state.pos(i).empty() && i < start) {
        start = i;
      }
    }
    set<Maybe> firsts;
    for (auto &m : maybes) {
      if (!state.has_before(start, m)) {
        firsts.insert(m);
      }
    }
    set<Maybe> avoid;
    for (auto &f : firsts) {
      for (auto &m : maybes) {
        if (f.value == m.value && f.group < m.group) {
          avoid.insert(m);
        }
      }
    }
    for (int i : line) {
      filter.put(i, state.pos(i).filter_maybe([&](auto &m) {
        return find(avoid.begin(), avoid.end(), m) == avoid.end();
      }));
    }
    return filter.flush();
  }
};

struct BoundedSequence final : public Strategy {
  int start, end, middle;
  const vector<int> line;
  string line_name;
  int line_pos;
  BoundedSequence(int start, int end, int middle,
                  const vector<int> line, string line_name, int pos)
      : start(start), end(end), middle(middle),
        line(line), line_name(line_name), line_pos(pos) {
  }
  string name() override {
    ostringstream oss;
    oss << "Digit " << middle << " between " << start << " and ";
    oss << end << " only appear on " << line_name << " " << line_pos;
    return oss.str();
  }
  map<int, Cell> strategy(const State &state) override {
    Filter filter{state};
    int current = -1;
    for (int i = 0; i < state.n * state.n; i++) {
      if (state.pos(i).value) {
        if (current == -1) {
          current = i;
          continue;
        }
        if (state.pos(current).value == start && state.pos(i).value == end) {
          if (done.find(make_pair(current, i)) != done.end()) {
            current = i;
            continue;
          }
          if (bounded(current + 1, i - 1, state)) {
            for (int j : line) {
              if (state.pos(j).maybe.has_value(middle) &&
                  (j < current || j > i)) {
                filter.put(j, state.pos(j).filter_maybe([&](auto &m) {
                  return m.value != middle;
                }));
              }
            }
            done.insert(make_pair(current, i));
            break;
          }
        }
        current = i;
      }
    }
    return filter.flush();
  }
  bool bounded(int a, int b, const State &state) {
    bool present = false;
    for (int i = a; i <= b; i++) {
      if (state.pos(i).maybe.has_value(middle)) {
        if (find(line.begin(), line.end(), i) == line.end()) {
          return false;
        }
        present = true;
      }
    }
    return present;
  }
  set<pair<int, int>> done;
};

struct OnlyValue final : public Strategy {
  int digit, group;
  const vector<int> line;
  string line_name;
  int line_pos;
  OnlyValue(int d, int g, const vector<int> line,
            string line_name, int pos)
      : digit(d), group(g), line(line),
        line_name(line_name), line_pos(pos) {
  }
  string name() override {
    ostringstream oss;
    oss << "Digit " << digit << " in group " << group;
    oss << " only appears on " << line_name << " " << line_pos;
    return oss.str();
  }
  map<int, Cell> strategy(const State &state) override {
    Filter filter{state};
    if (state.has_value(line, digit)) {
      skip = true;
      return {};
    }
    Maybe m{digit, group};
    int line_count = state.count_maybe(line, m);
    int grid_count = state.count_maybe(state.path.forward, m);
    if (line_count != grid_count) {
      return {};
    }
    for (int i : line) {
      filter.put(i, state.pos(i).filter_maybe([&](const Maybe &m) {
        return m.value != digit || m.group == group;
      }));
    }
    skip = true;
    return filter.flush();
  }
};

struct OnlyGroup final : public Strategy {
  int digit, group;
  const vector<int> line;
  string line_name;
  int line_pos;
  OnlyGroup(int d, int g, const vector<int> line,
            string line_name, int pos)
      : digit(d), group(g), line(line),
        line_name(line_name), line_pos(pos) {
  }
  string name() override {
    ostringstream oss;
    oss << line_name << " " << line_pos << " only has ";
    oss << digit << " in group " << group;
    return oss.str();
  }
  map<int, Cell> strategy(const State &state) override {
    Filter filter{state};
    if (state.has_value(line, digit)) {
      skip = true;
      return {};
    }
    Maybe m{digit, group};
    int line_count = state.count_maybe(line, m);
    int count = 0;
    for (int i : line) {
      count += state.pos(i).maybe.count([&](const auto& m) {
        return m.value == digit;
      });
    }
    if (line_count != count) {
      return {};
    }
    set<int> skipline(line.begin(), line.end());
    for (int i : state.path.forward) {
      if (skipline.find(i) == skipline.end()) {
        filter.put(i, state.pos(i).filter_maybe([&](const Maybe &m) {
          return !(m.value == digit && m.group == group);
        }));
      }
    }
    skip = true;
    return filter.flush();
  }
};

struct ExactlyNValues final : public Strategy {
  int n;
  const vector<int> line;
  string line_name;
  int line_pos;
  ExactlyNValues(int n, const vector<int> line,
            string line_name, int pos)
      : n(n), line(line), line_name(line_name), line_pos(pos) {
  }
  string name() override {
    ostringstream oss;
    oss << line_name << " " << line_pos << " must have exactly ";
    oss << n << " values";
    return oss.str();
  }
  map<int, Cell> strategy(const State &state) override {
    Filter filter{state};
    if (!state.is_sequence(line)) {
      return {};
    }
    set<int> valid;
    copy_if(line.begin(), line.end(), inserter(valid, valid.begin()),
        [&](int i){ return !state.pos(i).empty(); });
    set<Maybe> first = get_first(state, valid);
    map<int, set<Maybe>> allow;
    for (auto i = valid.begin(); i != valid.end(); ++i) {
      for (auto &m : state.pos(*i).maybe()) {
        if (first.find(m) == first.end()) {
          continue;
        }
        auto m2 = m.next();
        for (auto i2 = next(i); i2 != valid.end(); ++i2) {
          if (state.pos(*i2).maybe.has_maybe(m2)) {
            auto m3 = m2.next();
            for (auto i3 = next(i2); i3 != valid.end(); ++i3) {
              if (state.pos(*i3).maybe.has_maybe(m3)) {
                allow[*i].insert(m);
                allow[*i2].insert(m2);
                allow[*i3].insert(m3);
              }
            }
          }
        }
      }
    }
    for (auto kv : allow) {
      filter.put(kv.first, Cell({kv.second}, state.pos(kv.first).value));
    }
    return filter.flush();
  }
  set<Maybe> get_first(const State &state, const set<int>& line) {
    set<Maybe> valid;
    for (int i = *min_element(begin(line), end(line)) - 1; i >= 0; i--) {
      if (state.pos(i).value) {
        for (auto &m : state.pos(i).maybe()) {
          valid.insert(m.next());
        }
        return valid;
      }
      for (auto &m : state.pos(i).maybe()) {
        valid.insert(m);
        valid.insert(m.next());
      }
    }
    valid.insert(Maybe{1, 1});
    return valid;
  }
};

struct SingleLine final : public Strategy {
  int digit;
  const vector<int> line;
  string line_name;
  int line_pos;
  SingleLine(int d, const vector<int> line, string line_name, int pos)
      : digit(d), line(line), line_name(line_name), line_pos(pos) {
  }
  string name() override {
    ostringstream oss;
    oss << "Single " << digit << " in " << line_name << " " << line_pos;
    return oss.str();
  }
  map<int, Cell> strategy(const State &state) override {
    Filter filter{state};
    if (state.has_value(line, digit)) {
      skip = true;
      return {};
    }
    if (!state.has_single_digit(line, digit)) {
      return {};
    }
    for (int j : line) {
      if (state.pos(j).maybe.has_value(digit)) {
        filter.put(j, state.pos(j).filter_maybe(digit, [&](auto &m) {
          return m.value == digit;
        }));
        skip = true;
      }
    }
    return filter.flush();
  }
};

template<typename T>
struct SequenceImplication final : public Strategy {
  const vector<int> order, reverse;
  T action;
  string dir_name;
  SequenceImplication(const vector<int> order, T action, string name)
    : order(order), reverse(order.rbegin(), order.rend()),
      action(action), dir_name(name) {
  }
  string name() override {
    return dir_name + " implication";
  }
  map<int, Cell> strategy(const State &state) override {
    Filter filter{state};
    auto it = next(state.first_non_empty(order));
    for (; it != order.end(); ++it) {
      if (state.pos(*it).found()) {
        continue;
      }
      filter.put(*it, state.pos(*it).filter_maybe([&](const Maybe &m) {
        return search(state, *next(it, -1), m);
      }));
    }
    return filter.flush();
  }
  bool search(const State &state, int start, const Maybe &m) {
    auto it = find(reverse.begin(), reverse.end(), start);
    auto next = action(m);
    for (; it != reverse.end(); ++it) {
      auto &succ = state.pos(*it).maybe;
      auto &succ_value = state.pos(*it).value;
      if (succ.has_maybe(m) && succ_value) {
        return false;
      }
      if (succ.has_maybe(next) || succ.has_maybe(m)) {
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
unique_ptr<Strategy> newSequenceImplication(
    const vector<int> order, T action, string name){
  return make_unique<SequenceImplication<T>>(order, action, name);
}

struct FixEndpoint final : public Strategy {
  const vector<int> order;
  const Maybe endpoint;
  string endpoint_name;
  FixEndpoint(const vector<int> order,
              const Maybe endpoint, string name)
      : order(order), endpoint(endpoint),
        endpoint_name("Fix " + name + " cell") {
  }
  string name() override {
    return endpoint_name;
  }
  map<int, Cell> strategy(const State &state) override {
    Filter filter{state};
    auto it = state.first_non_empty(order);
    while (!state.pos(*it).maybe.has_value(endpoint.value)) {
      filter.put(*it++, Cell());
    }
    filter.put(*it, Cell{{set<Maybe>{endpoint}}, state.pos(*it).value});
    return filter.flush();
  }
};

enum struct Status {
  SOLVED,
  CHANGED,
  UNCHANGED
};

struct Snail {
  Snail(int n, const vector<string>& grid)
      : n(n), path(PathBuilder(n).build()), state(path), printer(path) {
    easy.push_back(make_unique<AddGivens>(grid));
    for (int d = 1; d <= 3; d++) {
      for (int j = 0; j < n; j++) {
        easy.push_back(make_unique<SingleLine>(d, path.row[j], "row", j));
        easy.push_back(make_unique<SingleLine>(d, path.column[j], "column", j));
      }
    }
    for (int j = 0; j < n; j++) {
      for (int i = 0 ; i < n; i++) {
        easy.push_back(make_unique<RemoveCross>(j, i));
      }
    }
    for (int d = 1; d <= 3; d++) {
      for (int g = 1; g <= n; g++) {
        easy.push_back(make_unique<DuplicateGroup>(d, g));
      }
    }
    hard.push_back(make_unique<FixEndpoint>(
        path.forward, Maybe{1, 1}, "first"));
    hard.push_back(make_unique<FixEndpoint>(
        path.backward, Maybe{3, n}, "last"));
    hard.push_back(newSequenceImplication(
        path.forward, [](auto &m){ return m.prev(); }, "Forward"));
    hard.push_back(newSequenceImplication(
        path.backward, [](auto &m){ return m.next(); }, "Backward"));
    for (int d = 1; d <= 3; d++) {
      for (int g = 1; g <= n; g++) {
        for (int i = 0; i < n; i++) {
          hard.push_back(make_unique<OnlyValue>(d, g, path.row[i], "row", i));
          hard.push_back(make_unique<OnlyValue>(d, g, path.column[i], "column", i));
          hard.push_back(make_unique<OnlyGroup>(d, g, path.row[i], "Row", i));
          hard.push_back(make_unique<OnlyGroup>(d, g, path.column[i], "Column", i));
        }
      }
    }
    for (int i = 0; i < n; i++) {
      hard.push_back(make_unique<LimitSequence>(path.row[i], "row", i));
      hard.push_back(make_unique<LimitSequence>(path.column[i], "column", i));
    }
    for (int s = 1; s <= 3; s++) {
      for (int e = 1; e <= 3; e++) {
        for (int m = path.modinc(s); m != e; m = path.modinc(m)) {
          for (int i = 0; i < n; i++) {
            hard.push_back(
                make_unique<BoundedSequence>(s, e, m, path.row[i], "row", i));
            hard.push_back(
                make_unique<BoundedSequence>(s, e, m, path.column[i], "column", i));
          }
        }
      }
    }
    for (int i = 0; i < n; i++) {
      hard.push_back(make_unique<ExactlyNValues>(3, path.row[i], "Row", i));
      hard.push_back(make_unique<ExactlyNValues>(3, path.column[i], "Column", i));
    }
  }
  void solve() {
    while (true) {
      auto easy_status = round(easy);
      if (easy_status == Status::SOLVED) {
        break;
      }
      if (easy_status == Status::CHANGED) {
        continue;
      }
      if (round(hard) != Status::CHANGED) {
        break;
      }
    }
    if (state.done()) {
      cout << "<div class=\"strategy\">Solved</div>";
    }
  }
  Status round(const vector<unique_ptr<Strategy>> &strategies) {
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
  const int n;
  const Path path;
  State state;
  const StatePrinter printer;
  vector<unique_ptr<Strategy>> easy, hard;
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
  cout << ".maybe-values {color: brown;}\n";
  cout << ".maybe-groups {color: green;}\n";
  cout << ".headtail {background-color: orange;}\n";
  cout << "</style></head><body>\n";
  Snail snail(n, grid);
  snail.solve();
  cout << "</body></html>";
  return 0;
}
