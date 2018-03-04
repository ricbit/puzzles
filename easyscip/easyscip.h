// EasySCIP 0.1
// A C++ interface to SCIP that is easy to use.
// by Ricardo Bittencourt 2013

// Please check the examples for a sample usage.

#include <vector>
#include <string>
#include "objscip/objscip.h"
#include "objscip/objscipdefplugins.h"

namespace easyscip {

class Constraint;
class MIPConstraint;
class Solution;
class MIPSolution;
class LPSolution;
class MIPSolver;
class Handler;
class DynamicConstraint;

class Variable {
 protected:
  Variable() : var_(NULL) {
  }
  SCIP_VAR *var_;
  friend Constraint;
  friend MIPConstraint;
  friend Solution;
  friend MIPSolution;
  friend LPSolution;
  friend MIPSolver;
};

class BinaryVariable : public Variable {
 protected:
  BinaryVariable(SCIP *scip, double objective, std::string name) {
    SCIPcreateVarBasic(
        scip, &var_, name.c_str(), 0, 1, objective, SCIP_VARTYPE_BINARY);
    SCIPaddVar(scip, var_);
  }
  friend MIPSolver;
};

class IntegerVariable : public Variable {
 protected:
  IntegerVariable(SCIP *scip, double lower_bound, double upper_bound,
                  double objective, std::string name) {
    SCIPcreateVarBasic(
        scip, &var_, name.c_str(), lower_bound, upper_bound, objective,
        SCIP_VARTYPE_INTEGER);
    SCIPaddVar(scip, var_);
  }
  friend MIPSolver;
};

class BaseConstraint {
 public:
  virtual void add_variable(Variable& var, double val) = 0;
  virtual void commit(double lower_bound, double upper_bound) = 0;
  virtual ~BaseConstraint() {
  }
};

class Constraint {
 public:
  void add_variable(Variable& var, double val) {
    constraint_->add_variable(var, val);
  }
  void commit(double lower_bound, double upper_bound) {
    constraint_->commit(lower_bound, upper_bound);
  }
  ~Constraint() {
    delete constraint_;
  }
 private:
  Constraint(BaseConstraint *constraint) : constraint_(constraint) {
  }
  BaseConstraint *constraint_;
  friend MIPSolver;
  friend DynamicConstraint;
};

class EmptyConstraint : public BaseConstraint {
 public:
  virtual void add_variable(Variable& var, double val) {
  }
  virtual void commit(double lower_bound, double upper_bound) {
  }
};

class MIPConstraint : public BaseConstraint {
 public:
  virtual void add_variable(Variable& var, double val) {
    vars_.push_back(var.var_);
    vals_.push_back(val);
  }
  virtual void commit(double lower_bound, double upper_bound) {
    SCIP_VAR **vars = new SCIP_VAR*[vars_.size()];
    SCIP_Real *vals = new SCIP_Real[vals_.size()];
    copy(vars_.begin(), vars_.end(), vars);
    copy(vals_.begin(), vals_.end(), vals);
    SCIP_CONS *cons;
    SCIPcreateConsLinear(
        scip_, &cons, name_.c_str(), vars_.size(), vars, vals,
        lower_bound, upper_bound,
        TRUE,   // initial
        TRUE,   // separate
        TRUE,   // enforce
        TRUE,   // check
        TRUE,   // propagate
        FALSE,  // local
        FALSE,  // modifiable
        FALSE,  // dynamic
        FALSE,  // removable
        FALSE); // stickatnode
    SCIPaddCons(scip_, cons);
    SCIPreleaseCons(scip_, &cons);
    delete[] vars;
    delete[] vals;
  }
 private:
  SCIP *scip_;
  std::string name_;
  std::vector<SCIP_VAR*> vars_;
  std::vector<SCIP_Real> vals_;
  MIPConstraint(SCIP *scip, int id)
      : scip_(scip), name_(std::string("constraint") + std::to_string(id)) {
  }
  friend MIPSolver;
  friend DynamicConstraint;
};

class BaseSolution {
 public:
  virtual double objective() = 0;
  virtual double value(Variable& var) = 0;
  virtual bool is_optimal() = 0;
  virtual ~BaseSolution() {
  }
};

class Solution {
 public:
  double objective() const {
    return solution_->objective();
  }
  double value(Variable& var) const {
    return solution_->value(var);
  }
  bool is_optimal() const {
    return solution_->is_optimal();
  }
  ~Solution() {
    delete solution_;
  }
 private:
  Solution(BaseSolution *solution) : solution_(solution) {
  }
  BaseSolution *solution_;
  friend MIPSolver;
  friend Handler;
};

class MIPSolution : public BaseSolution {
 public:
  virtual double objective() {
    return SCIPgetSolOrigObj(scip_, sol_);
  }
  virtual double value(Variable& var) {
    return SCIPgetSolVal(scip_, sol_, var.var_);
  }
  virtual bool is_optimal() {
    return SCIPgetStatus(scip_) == SCIP_STATUS_OPTIMAL;
  }
 private:
  MIPSolution(SCIP *scip, SCIP_Sol *sol) : scip_(scip), sol_(sol) {
  }
  virtual ~MIPSolution() {
  }
  SCIP *scip_;
  SCIP_Sol *sol_;
  friend MIPSolver;
  friend Handler;
};

class LPSolution : public BaseSolution {
 public:
  virtual double objective() {
    return 0;
  }
  virtual double value(Variable& var) {
    return SCIPgetVarSol(scip_, var.var_);
  }
  virtual bool is_optimal() {
    return SCIPgetStatus(scip_) == SCIP_STATUS_OPTIMAL;
  }
 private:
  LPSolution(SCIP *scip) : scip_(scip) {
  }
  virtual ~LPSolution() {
  }
  SCIP *scip_;
  friend Handler;
};

class MIPSolver {
 public:
  MIPSolver() : constraints_(0) {
    SCIPcreate(&scip_);
    SCIPsetMessagehdlrLogfile(scip_, "log.txt");
    SCIPprintVersion(scip_, NULL);
    SCIPsetEmphasis(scip_, SCIP_PARAMEMPHASIS_OPTIMALITY, FALSE);
    SCIPincludeDefaultPlugins(scip_);
    SCIPcreateProbBasic(scip_, "MIP");
  }
  ~MIPSolver() {
    for (auto var : variables_) {
      SCIPreleaseVar(scip_, &var.var_);
    }
    SCIPfree(&scip_);
  }
  Variable binary_variable(double objective) {
    return push_var(BinaryVariable(scip_, objective, next_name()));
  }
  Variable integer_variable(int lower_bound, int upper_bound,
                            double objective) {
    return push_var(IntegerVariable(
        scip_, lower_bound, upper_bound, objective, next_name()));
  }
  Variable push_var(Variable var) {
    variables_.push_back(var);
    return var;
  }
  std::string next_name() {
    return std::string("variable") + std::to_string(variables_.size());
  }
  Constraint constraint() {
    return Constraint(new MIPConstraint(scip_, constraints_++));
  }
  Solution solve() {
    SCIPsolve(scip_);
    return Solution(new MIPSolution(scip_, SCIPgetBestSol(scip_)));
  }
  Solution parallel_solve() {
    SCIPsolveConcurrent(scip_);
    return Solution(new MIPSolution(scip_, SCIPgetBestSol(scip_)));
  }
  void set_time_limit(int seconds) {
    SCIPsetRealParam(scip_, "limits/time", seconds);
  }
  int count_solutions() {
    SCIPcount(scip_);
    SCIP_Bool valid;
    return SCIPgetNCountedSols(scip_, &valid);
  }
  void write_model(std::string filename) {
    SCIPwriteOrigProblem(scip_, filename.c_str(), NULL, 0);
  }
 private:
  int constraints_;
  SCIP *scip_;
  std::vector<Variable> variables_;
};

}  // namespace easyscip
