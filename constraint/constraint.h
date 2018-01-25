#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <set>
#include <cstdio>
#include <limits>
#include <queue>

struct VariableId {
  int id;
  VariableId() : id(-1) {}
  VariableId(int id_) : id(id_) {}
  operator int() const {
    return id;
  }
};

struct Variable {
  int lmin, lmax;
  VariableId id;
  std::vector<int> constraints;
};

struct Bounds {
  int lmin, lmax;
};

struct Metadata {
  VariableId id;
  std::vector<int> constraints;
};

class State {
  std::vector<Bounds> bounds, solution;
  std::vector<Metadata> metadata;
 public:
  State(const std::vector<Variable>& variables) 
      : bounds(variables.size()), metadata(variables.size()) {
    for (const Variable& var : variables) {
      bounds[var.id].lmin = var.lmin;
      bounds[var.id].lmax = var.lmax;
      metadata[var.id].id = var.id;
      metadata[var.id].constraints = var.constraints;
    }
  }

  void save_solution() {
    solution = bounds;
  }

  int read_lmax(VariableId id) const {
    return bounds[id].lmax;
  }

  int read_lmin(VariableId id) const {
    return bounds[id].lmin;
  }

  bool fixed(VariableId id) const {
    return bounds[id].lmin == bounds[id].lmax;
  }

  int value(VariableId id) const {
    return solution[id].lmin;
  }

  void change_var(VariableId var_id, int lmin, int lmax) {
    bounds[var_id].lmin = lmin;
    bounds[var_id].lmax = lmax;
  }

  const std::vector<Bounds>& get_variables() {
    return bounds;
  }

  void set_variables(const std::vector<Bounds>& new_vars) {
    bounds = new_vars;
  }
};

class ExternalConstraint {
 public:
  virtual bool operator()(const State* state) const = 0;
};

class ConstraintQueue;

class TightenConstraint {
 public:
  virtual bool update_constraint(
      State *state, ConstraintQueue* cqueue) const = 0;
  virtual const std::vector<VariableId>& get_variables() const = 0;
};

class ConstraintQueue {
  const std::vector<Variable>& variables;
  const std::vector<const TightenConstraint*>& constraints;
  std::queue<int> active_constraints;
  std::vector<bool> queued_constraints;
 public:
  ConstraintQueue(const std::vector<Variable>& variables_,
      const std::vector<const TightenConstraint*>& constraints_)
      : variables(variables_), constraints(constraints_) {
    queued_constraints.resize(constraints.size(), true);
    for (int i = 0; i < int(constraints.size()); i++) {
      active_constraints.push(i);
    }
  }

  void push_variable(VariableId index) {
    for (int cons : variables[index].constraints) {
      if (!queued_constraints[cons]) {
        active_constraints.push(cons);
        queued_constraints[cons] = true;
      }
    }
  }

  int pop_constraint() {
    int cons = active_constraints.front();
    active_constraints.pop();
    queued_constraints[cons] = false;
    return cons;
  }

  bool empty() {
    return active_constraints.empty();
  }

  void clear() {
    while (!active_constraints.empty()) {
      int id = active_constraints.front();
      active_constraints.pop();
      queued_constraints[id] = false;
    }
  }
};

class LinearConstraint : public TightenConstraint {
  int lmin, lmax;
  std::vector<VariableId> variables;
 public:
  LinearConstraint(int lmin_, int lmax_) : lmin(lmin_), lmax(lmax_) {}
  virtual ~LinearConstraint() {}

  void add_variable(VariableId id) {
    variables.push_back(id);
  }

  virtual const std::vector<VariableId>& get_variables() const {
    return variables;
  }

  virtual bool update_constraint(State *state, ConstraintQueue* cqueue) const {
    int allmax = 0, allmin = 0;
    for (const VariableId& ivar : variables) {
      allmax += state->read_lmax(ivar);
      allmin += state->read_lmin(ivar);
    }
    if (allmax < lmin || allmin > lmax) {
      return false;
    }
    for (const VariableId& ivar : variables) {
      // increase min
      int limit = lmin - allmax + state->read_lmax(ivar);
      if (limit > state->read_lmax(ivar)) {
        return false;
      }
      if (state->read_lmin(ivar) < limit) {
        state->change_var(ivar, limit, state->read_lmax(ivar));
        cqueue->push_variable(ivar);
        return true;
      }
      // decrease max
      limit = lmax - allmin + state->read_lmin(ivar);
      if (limit < state->read_lmin(ivar)) {
        return false;
      }
      if (state->read_lmax(ivar) > limit) {
        state->change_var(ivar, state->read_lmin(ivar), limit);
        cqueue->push_variable(ivar);
        return true;
      }
    }
    return true;
  }
};

class ConstraintSolver {
  int recursion_nodes, constraints_checked;
  State* state;
  ConstraintQueue* cqueue;
  std::vector<Variable> variables;
  std::vector<const ExternalConstraint*> external;
  std::vector<const TightenConstraint*> tighten;
 public:
  ConstraintSolver() 
      : recursion_nodes(0), constraints_checked(0), 
        state(nullptr), cqueue(nullptr) {}
  ~ConstraintSolver() { 
    delete state;
    delete cqueue;
  }

  int create_variable(int lmin, int lmax) {
    Variable v;
    v.lmin = lmin;
    v.lmax = lmax;
    v.id = variables.size();
    variables.push_back(v);
    return variables.size() - 1;
  }

  void add_external_constraint(const ExternalConstraint* cons) {
    external.push_back(cons);
  }

  int value(VariableId id) {
    return state->value(id);
  }

  void add_constraint(const TightenConstraint* cons) {
    int id = tighten.size();
    tighten.push_back(cons);
    for (const VariableId& var : cons->get_variables()) {
      variables[var].constraints.push_back(id);
    }
  }

  bool solve() {
    state = new State(variables);
    std::cout << "Variables: " << variables.size() << "\n";
    std::cout << "Constraints: " << tighten.size() << "\n";
    cqueue = new ConstraintQueue(variables, tighten);
    tight();
    int freevars = 0;
    for (const auto& var : variables) {
      if (!state->fixed(var.id)) {
        freevars++;
      }
    }
    std::cout << "Free variables: " << freevars << "\n";
    bool result = recursion();
    std::cout << "Recursion nodes: " << recursion_nodes << "\n";
    std::cout << "Constraints checked: " << constraints_checked << "\n";
    std::cout << "Solution " << (result ? "" : "not ") << "found\n";
    return result;
  }

  bool recursion() {
    recursion_nodes++;
    if (finished()) {      
      state->save_solution();
      return true;
    }
    VariableId index = choose();
    std::vector<Bounds> bkp = state->get_variables();
    int savemin = state->read_lmin(index), savemax = state->read_lmax(index);
    //for (int i = savemax; i >= savemin; i--) {
    for (int i = savemin; i <= savemax; i++) {
      state->set_variables(bkp);
      state->change_var(index, i, i);
      cqueue->push_variable(index);
      if (tight() && valid()) {
        int x = 0;
    for (const Variable& var : variables) {
      if (state->fixed(var.id)) {
        x++;
      }
    }
    std::cout << "x " << x << "\n";
        if (recursion()) {
          return true;
        }
      }
    }
    state->set_variables(bkp);
    return false;
  }

  bool valid() {
    for (auto& cons : external) {
      if (!(*cons)(state)) {
        return false;
      }
    }
    return true;
  }

  VariableId choose() {
    VariableId chosen = 0;
    int diff = std::numeric_limits<int>::max();
    for (const Variable& var : variables) {
      if (!state->fixed(var.id)) {
        int cur_diff = state->read_lmax(var.id) - state->read_lmin(var.id);
        if (cur_diff < diff) {
          chosen = var.id;
          diff = cur_diff;
        } else if (cur_diff == diff && 
            var.constraints.size() > variables[chosen].constraints.size()) {
          chosen = var.id;
        }
      }
    }
    return chosen;
  }

  bool finished() {
    for (const Variable& var : variables) {
      if (!state->fixed(var.id)) {
        return false;
      }
    }
    return true;
  }

  bool tight() {
    while (!cqueue->empty()) {
      int id = cqueue->pop_constraint();
      constraints_checked++;
      if (!tighten[id]->update_constraint(state, cqueue)) {
        cqueue->clear();
        return false;
      }
    }
    return true;
  }
};

