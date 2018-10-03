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
#include <type_traits>

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
    }
    return Maybe{1, group + 1};
  }
  Maybe prev() const {
    if (value > 1) {
      return Maybe{value - 1, group};
    }
    return Maybe{3, group - 1};
  }
};

struct Coord {
  int y, x;
};

template<typename T>
struct PositionContainer {
  template<typename Type, typename Action>
  Type accumulate(Type initial, Action action) const {
    return std::accumulate(convert()->begin(), convert()->end(), initial,
        [action](Type x, const auto &y) {
      return x + action(y);
    });
  }
 private:
  auto convert() const {
    return static_cast<const T*>(this);
  }
};

struct PositionVector : public PositionContainer<PositionVector> {
  const vector<int> line;
  PositionVector(vector<int> line) : line(move(line)) {
  }
  auto begin() const {
    return line.cbegin();
  }
  auto end() const {
    return line.cend();
  }
  auto rbegin() const {
    return line.crbegin();
  }
  auto rend() const {
    return line.crend();
  }
};

struct Line : PositionVector {
};

struct Path : PositionVector {
};

template<typename T>
struct TrimmedLine : public PositionContainer<TrimmedLine<T>> {
  using category = typename iterator_traits<T>::iterator_category;
  TrimmedLine(T begin, T end) : begin_(begin), end_(end) {
    static_assert(is_base_of<bidirectional_iterator_tag, category>::value);
  }
  auto begin() const {
    return begin_;
  }
  auto end() const {
    return end_;
  }
  auto rbegin() const {
    return make_reverse_iterator(end_);
  }
  auto rend() const {
    return make_reverse_iterator(begin_);
  }
 private:
  const T begin_, end_;
};

struct Sequence : PositionVector {
};

struct Geom {
  const int n;
  const vector<Coord> grid;
  const vector<vector<int>> line;
  const vector<Line> row, column;
  const Path forward, backward;
  int modinc(int a) const {
    return 1 + ((a - 1) + 1) % 3;
  }
  optional<int> move(int j, int i, int dir) const {
    int nj = j + dy[dir];
    int ni = i + dx[dir];
    if (ni < 0 || ni >= n || nj < 0 || nj >= n) {
      return {};
    }
    return line[nj][ni];
  }
  int row_getter(int i, int j) const {
    return row[i].line[j];
  }
  int column_getter(int i, int j) const {
    return column[i].line[j];
  }
  constexpr static int dx[4]{1, 0, -1, 0};
  constexpr static int dy[4]{0, 1, 0, -1};
};

struct GeomBuilder {
  GeomBuilder(int n): n(n), grid(n * n), line(n, vector<int>(n)) {
  }
  Geom build() {
    int miny = -1, maxy = n;
    int minx = -1, maxx = n;
    for (int i = 0; i < n * n; i++) {
      grid[i] = Coord{cury, curx};
      line[cury][curx] = i;
      curx += Geom::dx[dir];
      cury += Geom::dy[dir];
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
      row.push_back(build_row(i));
      column.push_back(build_column(i));
    }
    return Geom{n, grid, line, row, column, build_forward(), build_backward()};
  }
 private:
  void change() {
    curx -= Geom::dx[dir];
    cury -= Geom::dy[dir];
    dir = (dir + 1) % 4;
    curx += Geom::dx[dir];
    cury += Geom::dy[dir];
  }
  void print() const {
    for (int j = 0; j < n; j++) {
      for (int i = 0; i < n; i++) {
        cout << line[j][i] << " ";
      }
      cout << "\n";
    }
  }
  const Path build_forward() const {
    vector<int> ans(n * n);
    iota(begin(ans), end(ans), 0);
    return {ans};
  }
  const Path build_backward() const {
    Path forward = build_forward();
    return {vector<int>(rbegin(forward), rend(forward))};
  }
  Line build_column(int c) const {
    vector<int> ans;
    ans.reserve(n);
    for (int i = 0; i < n; i++) {
      ans.push_back(line[i][c]);
    }
    return {ans};
  }
  Line build_row(int r) const {
    vector<int> ans;
    ans.reserve(n);
    for (int i = 0; i < n; i++) {
      ans.push_back(line[r][i]);
    }
    return {ans};
  }
  const int n;
  vector<Coord> grid;
  vector<vector<int>> line;
  vector<Line> row, column;
  int cury = 0, curx = 0;
  int dir = 0;
};

struct MaybeSet {
  set<Maybe> maybe;
  bool operator==(const MaybeSet &b) const {
    return equal(begin(maybe), end(maybe), begin(b.maybe), end(b.maybe));
  }
  bool has_value(int v) const {
    return end(maybe) != find_if(begin(maybe), end(maybe), [&](auto &m) {
      return m.value == v;
    });
  }
  bool has_maybe(const Maybe &m) const {
    return maybe.find(m) != end(maybe);
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
  template<typename T>
  set<int> extract(T action) const {
    set<int> seq;
    std::transform(begin(maybe), end(maybe),
        inserter(seq, begin(seq)), action);
    return seq;
  }
  template<typename T>
  MaybeSet transform(T action) const {
    set<Maybe> seq;
    std::transform(begin(maybe), end(maybe),
        inserter(seq, begin(seq)), action);
    return {seq};
  }
  MaybeSet valid_groups(int max_group) const {
    return filter([max_group](const auto &m) {
      return m.group >= 1 && m.group <= max_group;
    });
  }
  template<typename T>
  void iterate(T action) const {
    for (const auto &m : maybe) {
      action(m);
    }
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
  bool operator==(const Cell& b) const {
    return value == b.value && maybe == b.maybe &&
        head == b.head && tail == b.tail;
  }
  bool empty() const {
    return maybe.empty();
  }
  Cell with_maybe(MaybeSet newmaybe) const {
    return Cell{move(newmaybe), value, head, tail};
  }
  template<typename T>
  Cell filter_maybe(T filter) const {
    return Cell{maybe.filter(filter), value, head, tail};
  }
  Cell set_value(int new_value) const {
    return Cell{maybe.filter([new_value](const Maybe &m) {
      return m.value == new_value;
    }), new_value, head, tail};
  }
  Cell set_head(const MaybeSet newhead) const {
    return Cell{maybe, value, newhead, tail};
  }
  Cell set_tail(const MaybeSet newtail) const {
    return Cell{maybe, value, head, newtail};
  }
  bool found() const {
    return value && maybe.size() == 1;
  }
 private:
  Cell(MaybeSet maybe, optional<int> value, MaybeSet head, MaybeSet tail)
      : maybe(maybe), value(value),
        head(value.has_value() ? maybe : head),
        tail(value.has_value() ? maybe : tail) {
  }
};

struct CellPrinter {
  string maybe_values(const Cell &cell) const {
    return print_values(cell.maybe.extract([](auto &m) {
      return m.value;
    }));
  }
  string maybe_groups(const Cell &cell) const {
    return format_sequence(cell.maybe.extract([](auto &m) {
      return m.group;
    }));
  }
  string print_value(const Cell& cell) const {
    if (cell.value) {
      return string(1, '0' + *cell.value);
    }
    return "&nbsp;";
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
  State(const Geom &geom)
      : n(geom.n), pos_(n * n, Cell(n)), geom(geom) {
  }
  const Cell &pos(int i) const {
    return pos_[i];
  }
  const Cell &pos(int j, int i) const {
    return pos_[geom.line[j][i]];
  }
  Cell &pos(int i) {
    return pos_[i];
  }
  Cell &pos(int j, int i) {
    return pos_[geom.line[j][i]];
  }
  template<typename T>
  bool has_value(const T &line, int value) const {
    return end(line) != find_if(begin(line), end(line), [&](int i) {
      return pos(i).value == value;
    });
  }
  template<typename T>
  int count_maybe(const T& line, const Maybe &m) const {
    return count_if(begin(line), end(line), [&](int i) {
      return pos(i).maybe.has_maybe(m);
    });
  }
  template<typename T>
  auto first_non_empty(const T& order) const {
    return find_if(begin(order), end(order), [&](int i) {
      return !pos(i).empty();
    });
  }
  template<typename T>
  bool has_single_digit(const T& line, int digit) const {
    return 1 == count_if(begin(line), end(line), [&](auto i) {
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
  auto trim(const Line& line) const {
    auto notempty = [&](auto i) { return !pos(i).empty(); };
    auto first = find_if(begin(line), end(line), notempty);
    auto last = find_if(rbegin(line), rend(line), notempty).base();
    return TrimmedLine(first, last);
  }
  template<typename T>
  optional<Sequence> sequence(const TrimmedLine<T> &line) const {
    vector<int> sorted(begin(line), end(line));
    sort(begin(sorted), end(sorted));
    auto check = adjacent_find(begin(sorted), end(sorted),
        [](const auto &a, const auto &b) { return b != a + 1; });
    if (check == end(sorted)) {
      return Sequence{sorted};
    }
    return {};
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
  const Geom &geom;
};

struct StatePrinter {
  StatePrinter() : printer() {
  }
  string border(const Geom &geom, int j, int i) const {
    ostringstream oss;
    const static string name[4]{"right", "bottom", "left", "top"};
    for (int d = 1; d <= 4; d++) {
      auto coord = geom.move(j, i, d % 4);
      oss << "border-" << name[d % 4] << "-width: ";
      oss << (coord && (abs(*coord - geom.line[j][i]) == 1) ? 1 : 3);
      oss << "px;";
    }
    return oss.str();
  }
  string print(const State &state, const map<int, Cell> &changed) const {
    ostringstream oss;
    oss << "<div><table>";
    for (int j = 0; j < state.geom.n; j++) {
      oss << "<tr>";
      for (int i = 0; i < state.geom.n; i++) {
        oss << "<td style=\"border-style: solid; border-color: black;";
        oss << border(state.geom, j, i);
        if (changed.find(state.geom.line[j][i]) != end(changed)) {
          oss << "background-color: yellow;";
        }
        oss << "\"><div class=\"outercontent\">";
        if (!state.pos(j, i).head.empty()) {
          oss << "<div class=\"head\">";
          oss << print_maybes(state.pos(j, i).head);
          oss << "</div>";
        }
        oss << "<div class=\"content\">";
        if (state.pos(j, i).value) {
          oss << "<div class=\"value\">";
          oss << printer.print_value(state.pos(j, i)) << "</div>";
          oss << "<div class=\"maybe-groups\">";
          oss << printer.maybe_groups(state.pos(j, i)) << "</div>";
        } else {
          oss << print_maybes(state.pos(j, i).maybe);
        }
        oss << "</div>";
        if (!state.pos(j, i).tail.empty()) {
          oss << "<div class=\"tail\">";
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
      ans.insert(make_pair(state.geom.line[j][i], cell));
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
        filter.put(j, i, state.pos(j, i).set_value(value));
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
    const Path &order = state.geom.forward;
    auto it = find_if(begin(order), end(order), [&](int i) {
      return state.pos(i).found() && state.pos(i).maybe.has_maybe(goal);
    });
    if (it == end(order)) {
      return {};
    }
    int ri = distance(begin(order), it);
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
  const Line &line;
  const string line_name;
  LimitSequence(const Line &line, const string &line_name_, int pos)
      : line(line), line_name(build_name(line_name_, pos)) {
  }
  string name() override {
    return line_name;
  }
  string build_name(const string &line_name, int line_pos) const {
    ostringstream oss;
    oss << "Can't have more than 3 digits in " << line_name << " " << line_pos;
    return oss.str();
  }
  map<int, Cell> strategy(const State &state) override {
    auto seq = state.sequence(state.trim(line));
    if (!seq.has_value()) {
      return {};
    }
    Filter filter{state};
    set<Maybe> maybes;
    int start = state.n * state.n;
    for (int i : *seq) {
      state.pos(i).maybe.iterate([&](auto &m) {
        maybes.insert(m);
      });
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
    for (int i : *seq) {
      filter.put(i, state.pos(i).filter_maybe([&](auto &m) {
        return find(begin(avoid), end(avoid), m) == end(avoid);
      }));
    }
    return filter.flush();
  }
};

struct BoundedSequence final : public Strategy {
  int start, end, middle;
  const Line &line;
  const string line_name;
  BoundedSequence(int start, int end, int middle,
                  const Line &line, const string &line_name, int pos)
      : start(start), end(end), middle(middle),
        line(line), line_name(build_name(line_name, start, end, middle, pos)) {
  }
  string name() override {
    return line_name;
  }
  string build_name(const string &line_name,
      int start, int end, int middle, int line_pos) const {
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
          if (done.find(make_pair(current, i)) != std::end(done)) {
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
        if (find(begin(line), std::end(line), i) == std::end(line)) {
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
  const Line &line;
  const string line_name;
  int line_pos;
  OnlyValue(int d, int g, const Line &line,
            const string &line_name, int pos)
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
    int grid_count = state.count_maybe(state.geom.forward, m);
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
  const Line &line;
  const string line_name;
  int line_pos;
  OnlyGroup(int d, int g, const Line &line,
            const string &line_name, int pos)
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
    int count = line.accumulate(0, [&](const auto &i) {
      return state.pos(i).maybe.count([&](const auto& m) {
        return m.value == digit;
      });
    });
    if (line_count != count) {
      return {};
    }
    set<int> skipline(begin(line), end(line));
    for (int i : state.geom.forward) {
      if (skipline.find(i) == end(skipline)) {
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
  const Line &line;
  const string line_name;
  ExactlyNValues(int n_, const Line &line_, const string &line_name_, int pos_)
      : n(n_), line(line_), line_name(build_name(line_name_, n_, pos_)) {
  }
  string name() override {
    return line_name;
  }
  const string build_name(const string &line_name, int n, int line_pos) const {
    ostringstream oss;
    oss << line_name << " " << line_pos << " must have exactly ";
    oss << n << " values";
    return oss.str();
  }
  map<int, Cell> strategy(const State &state) override {
    auto seq = state.sequence(state.trim(line));
    if (!seq.has_value()) {
      return {};
    }
    Filter filter{state};
    set<int> valid;
    copy_if(begin(*seq), end(*seq), inserter(valid, begin(valid)),
        [&](int i){ return !state.pos(i).empty(); });
    set<Maybe> first = get_first(state, valid);
    map<int, set<Maybe>> allow;
    for (auto i = begin(valid); i != end(valid); ++i) {
      state.pos(*i).maybe.iterate([&](auto &m) {
        if (first.find(m) == end(first)) {
          return;
        }
        auto m2 = m.next();
        for (auto i2 = next(i); i2 != end(valid); ++i2) {
          if (state.pos(*i2).maybe.has_maybe(m2)) {
            auto m3 = m2.next();
            for (auto i3 = next(i2); i3 != end(valid); ++i3) {
              if (state.pos(*i3).maybe.has_maybe(m3)) {
                allow[*i].insert(m);
                allow[*i2].insert(m2);
                allow[*i3].insert(m3);
              }
            }
          }
        }
      });
    }
    for (auto kv : allow) {
      filter.put(kv.first,
          state.pos(kv.first).with_maybe({kv.second}));
    }
    return filter.flush();
  }
  set<Maybe> get_first(const State &state, const set<int>& line) {
    set<Maybe> valid;
    for (int i = *min_element(begin(line), end(line)) - 1; i >= 0; i--) {
      if (state.pos(i).value) {
        state.pos(i).maybe.iterate([&](auto &m) {
          valid.insert(m.next());
        });
        return valid;
      }
      state.pos(i).maybe.iterate([&](auto &m) {
        valid.insert(m);
        valid.insert(m.next());
      });
    }
    valid.insert(Maybe{1, 1});
    return valid;
  }
};

struct BitMask {
  uint16_t mask;
  bool has_line(int line) const {
    return mask & (1 << line);
  }
  int size() const {
    int ans = 0, m = mask;
    while (m) {
      if (m & 1) {
        ans++;
      }
      m >>= 1;
    }
    return ans;
  }
};

template<typename Getter>
struct XWing final : public Strategy {
  const int digit;
  const BitMask mask;
  const string line_name;
  const Getter getter;
  XWing(int d, BitMask mask, const string &line_name, Getter getter)
      : digit(d), mask(mask), line_name(build_name(line_name, d, mask)),
        getter(getter) {
  }
  string name() override {
    return line_name;
  }
  const string build_name(const string &line_name,
      int digit, BitMask mask) const {
    ostringstream oss;
    oss << "XWing for digit " << digit << " in " << line_name << " : ";
    for (int i = 0; i < 20; i++) {
      if (mask.has_line(i)) {
        oss << i << " ";
      }
    }
    return oss.str();
  }
  map<int, Cell> strategy(const State &state) override {
    set<int> inside, outside, diff;
    for (int i = 0; i < state.n; i++) {
      for (int j = 0; j < state.n; j++) {
        state.pos(getter(i, j)).maybe.iterate([&](const auto &m) {
          if (m.value == digit) {
            if (mask.has_line(i)) {
              inside.insert(m.group);
            } else {
              outside.insert(m.group);
            }
          }
        });
      }
    }
    set_difference(
        begin(inside), end(inside), begin(outside), end(outside),
        inserter(diff, begin(diff)));
    if (static_cast<int>(diff.size()) != mask.size()) {
      return {};
    }
    Filter filter{state};
    for (int i = 0; i < state.n; i++) {
      for (int j = 0; j < state.n; j++) {
        if (!mask.has_line(i)) {
          filter.put(getter(i, j), state.pos(getter(i, j)).filter_maybe(
              [&](const auto &m) {
            return m.value != digit ||
                   find(begin(diff), end(diff), m.group) == end(diff);
          }));
        } else {
          filter.put(getter(i, j), state.pos(getter(i, j)).filter_maybe(
              [&](const auto &m) {
            return m.value != digit ||
                   find(begin(diff), end(diff), m.group) != end(diff);
          }));
        }
      }
    }
    return filter.flush();
  }
};

template<typename Getter>
unique_ptr<Strategy> newXWing(
  int d, BitMask mask, const string &line_name, Getter getter) {
    return make_unique<XWing<Getter>>(d, mask, line_name, getter);
}

struct SingleLine final : public Strategy {
  int digit;
  const Line &line;
  const string line_name;
  SingleLine(int d, const Line &line, const string &line_name, int pos)
      : digit(d), line(line), line_name(build_name(line_name, d, pos)) {
  }
  string name() override {
    return line_name;
  }
  const string build_name(const string &line_name,
      int digit, int line_pos) const {
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
        filter.put(j, state.pos(j).set_value(digit));
        skip = true;
      }
    }
    return filter.flush();
  }
};

template<typename T>
struct SequenceImplication final : public Strategy {
  const Path &order, &reverse;
  T action;
  const string dir_name;
  SequenceImplication(
      const Path &order, const Path &reverse, T action, const string &name)
    : order(order), reverse(reverse), action(action),
      dir_name(build_name(name)) {
  }
  string name() override {
    return dir_name;
  }
  const string build_name(const string &dir_name) const {
    return dir_name + " implication";
  }
  map<int, Cell> strategy(const State &state) override {
    Filter filter{state};
    auto it = next(state.first_non_empty(order));
    for (; it != end(order); ++it) {
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
    auto it = find(begin(reverse), end(reverse), start);
    auto next = action(m);
    for (; it != end(reverse); ++it) {
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
    const Path &order, const Path &reverse, T action, string name){
  return make_unique<SequenceImplication<T>>(order, reverse, action, name);
}

template<typename Get, typename Set>
struct HeadTailPropagation final : public Strategy {
  const string dir_name;
  const Path &forward;
  Get get;
  Set set;
  HeadTailPropagation(
      const string &name, const Path &forward, Get get, Set set)
      : dir_name(name + " propagation"), forward(forward),
        get(get), set(set) {
  }
  string name() override {
    return dir_name;
  }
  map<int, Cell> strategy(const State &state) override {
    Filter filter{state};
    for (auto i : forward) {
      if (!get(state.pos(i)).empty()) {
        auto start = find(begin(forward), end(forward), i);
        for (auto j = next(start); j != end(forward); ++j) {
          if (!state.pos(*j).empty()) {
            filter.put(*j, set(state.pos(*j), get(state.pos(i))));
            break;
          }
        }
      }
    }
    return filter.flush();
  }
};

template<typename Get, typename Set>
unique_ptr<Strategy> newHeadTailPropagation(
    const string &name, const Path &forward, Get get, Set set) {
  return make_unique<HeadTailPropagation<Get, Set>>(name, forward, get, set);
}

struct CompleteSequence final : public Strategy {
  const Line &line;
  const string line_name;
  CompleteSequence(const Line &line, const string &line_name, int pos)
      : line(line), line_name(build_name(line_name, pos)) {
  }
  string name() override {
    return line_name;
  }
  const string build_name(const string &line_name, int line_pos) const {
    ostringstream oss;
    oss << "Complete head and tail in " << line_name << " " << line_pos;
    return oss.str();
  }
  map<int, Cell> strategy(const State &state) override {
    auto seq = state.sequence(state.trim(line));
    if (!seq.has_value()) {
      return {};
    }
    Filter filter{state};
    const auto &first = *seq->begin();
    const auto &last = *seq->rbegin();
    if (state.pos(last).tail.empty() && !state.pos(first).head.empty()) {
      filter.put(last, state.pos(last).set_tail(
          state.pos(first).head.transform([&](const auto &m) {
            return m.next().next();
          })));
    }
    if (!state.pos(last).tail.empty() && state.pos(first).head.empty()) {
      filter.put(first, state.pos(first).set_head(
          state.pos(last).tail.transform([&](const auto& m) {
            return m.prev().prev();
          })));
    }
    return filter.flush();
  }
};

struct UnitSequence final : public Strategy {
  int row, column;
  const string description;
  UnitSequence(int row_, int column_)
      : row(row_), column(column_), description(build_name(row, column)) {
  }
  string name() override {
    return description;
  }
  const string build_name(int row, int column) const {
    ostringstream oss;
    oss << "Equal head and tail in row " << row << " column " << column;
    return oss.str();
  }
  map<int, Cell> strategy(const State &state) override {
    auto &cell = state.pos(row, column);
    if (cell.value.has_value() || (cell.head != cell.tail)) {
      return {};
    }
    Filter filter{state};
    for (int d = 1; d <= 3; d++) {
      if (cell.head.has_value(d)) {
        filter.put(row, column, cell.set_value(d));
      }
    }
    return filter.flush();
  }
};

struct FixEndpoint final : public Strategy {
  const Path &order;
  const Maybe endpoint;
  const string endpoint_name;
  FixEndpoint(const Path &order,
              const Maybe endpoint, const string &name)
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
    filter.put(*it, state.pos(*it).with_maybe({{endpoint}}));
    return filter.flush();
  }
};

enum struct Status {
  SOLVED,
  CHANGED,
  UNCHANGED
};

template<typename SnailPrinter>
struct Snail {
  Snail(int n, const vector<string>& grid, SnailPrinter printer)
      : n(n), geom(GeomBuilder(n).build()), state(geom), printer(printer),
        easy(build_easy(n, geom, grid)), hard(build_hard(n)) {
  }
  const vector<unique_ptr<Strategy>> build_easy(
      int n, const Geom& geom, const vector<string>& grid) {
    vector<unique_ptr<Strategy>> easy;
    easy.push_back(make_unique<AddGivens>(grid));
    for (int d = 1; d <= 3; d++) {
      for (int j = 0; j < n; j++) {
        easy.push_back(make_unique<SingleLine>(d, geom.row[j], "row", j));
        easy.push_back(make_unique<SingleLine>(d, geom.column[j], "column", j));
      }
    }
    for (int j = 0; j < n; j++) {
      for (int i = 0 ; i < n; i++) {
        easy.push_back(make_unique<RemoveCross>(j, i));
        easy.push_back(make_unique<UnitSequence>(j, i));
      }
    }
    for (int d = 1; d <= 3; d++) {
      for (int g = 1; g <= n; g++) {
        easy.push_back(make_unique<DuplicateGroup>(d, g));
      }
    }
    return easy;
  }
  const vector<unique_ptr<Strategy>> build_hard(int n) {
    vector<unique_ptr<Strategy>> hard;
    hard.push_back(make_unique<FixEndpoint>(
        geom.forward, Maybe{1, 1}, "first"));
    hard.push_back(make_unique<FixEndpoint>(
        geom.backward, Maybe{3, n}, "last"));
    hard.push_back(newSequenceImplication(
        geom.forward, geom.backward,
        [](auto &m){ return m.prev(); }, "Forward"));
    hard.push_back(newSequenceImplication(
        geom.backward, geom.forward,
        [](auto &m){ return m.next(); }, "Backward"));
    for (int d = 1; d <= 3; d++) {
      for (int g = 1; g <= n; g++) {
        for (int i = 0; i < n; i++) {
          hard.push_back(make_unique<OnlyValue>(d, g, geom.row[i], "row", i));
          hard.push_back(make_unique<OnlyValue>(d, g, geom.column[i], "column", i));
          hard.push_back(make_unique<OnlyGroup>(d, g, geom.row[i], "Row", i));
          hard.push_back(make_unique<OnlyGroup>(d, g, geom.column[i], "Column", i));
        }
      }
    }
    for (int i = 0; i < n; i++) {
      hard.push_back(make_unique<LimitSequence>(geom.row[i], "row", i));
      hard.push_back(make_unique<LimitSequence>(geom.column[i], "column", i));
    }
    for (int s = 1; s <= 3; s++) {
      for (int e = 1; e <= 3; e++) {
        for (int m = geom.modinc(s); m != e; m = geom.modinc(m)) {
          for (int i = 0; i < n; i++) {
            hard.push_back(
                make_unique<BoundedSequence>(s, e, m, geom.row[i],
                  "row", i));
            hard.push_back(
                make_unique<BoundedSequence>(s, e, m, geom.column[i],
                  "column", i));
          }
        }
      }
    }
    for (int i = 0; i < n; i++) {
      hard.push_back(make_unique<ExactlyNValues>(3, geom.row[i],
          "Row", i));
      hard.push_back(make_unique<ExactlyNValues>(3, geom.column[i],
          "Column", i));
    }
    hard.push_back(newHeadTailPropagation("Tail", geom.forward,
        [](const auto &cell) { return cell.tail; },
        [n](const auto &cell, const auto &maybeset) {
          return cell.set_head(maybeset.transform([](const Maybe& m) {
            return m.next();
          }).valid_groups(n));
        }));
    hard.push_back(newHeadTailPropagation("Head", geom.backward,
        [](const auto &cell) { return cell.head; },
        [n](const auto &cell, const auto &maybeset) {
          return cell.set_tail(maybeset.transform([](const Maybe& m) {
            return m.prev();
          }).valid_groups(n));
        }));
    for (int i = 0; i < n; i++) {
      hard.push_back(make_unique<CompleteSequence>(geom.row[i],
          "Row", i));
      hard.push_back(make_unique<CompleteSequence>(geom.column[i],
          "Column", i));
    }
    for (int d = 0; d < 3; d++) {
      for (uint16_t j = 0; j < (1 << n); j++) {
        BitMask mask{j};
        if (mask.size() <= 4) {
          hard.push_back(newXWing(d, mask, "Row", [&](int x, int y) {
            return state.geom.row_getter(x, y);
          }));
          hard.push_back(newXWing(d, mask, "Column", [&](int x, int y) {
            return state.geom.column_getter(x, y);
          }));
        }
      }
    }
    return hard;
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
      printer.solved();
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
        printer.strategy_header(s->name());
        printer.print_state(state, ans);
        for (auto &x : ans) {
          state.pos(x.first) = x.second;
        }
        printer.print_state(state, ans);
        printer.strategy_footer();
        if (state.done()) {
          return Status::SOLVED;
        }
        changed = true;
      }
    }
    return changed ? Status::CHANGED : Status::UNCHANGED;
  }
  const int n;
  const Geom geom;
  State state;
  const SnailPrinter printer;
  const vector<unique_ptr<Strategy>> easy, hard;
};

struct SnailPrinter {
  SnailPrinter() : state_printer() {
    cout << "<html>";
    cout << "<head><style>";
    cout << "table { border-collapse: collapse; margin-bottom: 20px;";
    cout << "        margin-top: 20px; margin-right: 20px;}\n";
    cout << "td { width: 70px; height: 70px; padding: 0px;";
    cout << "     margin: 0px; text-align: center}\n";
    cout << ".strategy { margin-top: 20px; ";
    cout << "    display: flex; flex-direction: column; font-size: 20px}\n";
    cout << ".compare {display: flex;}\n";
    cout << ".content {display: flex; flex-direction: column;";
    cout << "     justify-content: space-evenly; height: 100%;}\n";
    cout << ".outercontent {display: flex; flex-direction: column;";
    cout << "     justify-content: space-between; height: 100%;}\n";
    cout << ".maybe-values, .maybe-groups {";
    cout << "     font-size: 12px; font-family: sans-serif;}\n";
    cout << ".value {font-size: 30px; font-family: sans-serif;";
    cout << "        font-weigth: bold}\n";
    cout << ".maybe-values {color: brown;}\n";
    cout << ".maybe-groups {color: green;}\n";
    cout << ".head {background-color: #ffa50061;}\n";
    cout << ".tail {background-color: #ffa50061;}\n";
    cout << "</style></head><body>\n";
  }
  ~SnailPrinter() {
    cout << "</body></html>";
  }
  void print_state(const State &state, const map<int, Cell> &changed) const {
    cout << state_printer.print(state, changed);
  }
  void solved() const {
    cout << "<div class=\"strategy\">Solved</div>";
  }
  void strategy_header(const string &name) const {
    cout << "<div class=\"strategy\"><div>" << name << "</div>";
    cout << "<div class=\"compare\">";
  }
  void strategy_footer() const {
    cout << "</div></div><hr>";
  }
  const StatePrinter state_printer;
};

struct NullPrinter {
  void print_state(
      const State &/*unused*/, const map<int, Cell> &/*unused*/) const {
  }
  void solved() const {
    cout << "Solved\n";
  }
  void strategy_header(const string &/*unused*/) const {
  }
  void strategy_footer() const {
  }
};

int main() {
  int n;
  cin >> n;
  vector<string> grid(n);
  for (int i = 0; i < n; i++) {
    cin >> grid[i];
  }
  SnailPrinter printer;
  Snail snail(n, grid, printer);
  snail.solve();
  return 0;
}
