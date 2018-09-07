#include <iostream>
#include "easyscip/easyscip.h"

using namespace std;
using namespace easyscip;

struct Point {
  double x, y;
};

struct Visit {
  int slot;
  Point pos;
  int pool;
};

struct HouseCalculator {
  int na, nv;
  const vector<Point>& agents;
  const vector<Visit>& visits;

  bool ishouse(int v) const {
    return v < na;
  }
  bool ishouse(int v, int a) const {
    return v == a;
  }
  const Point &pos(int v) const {
    if (ishouse(v)) {
      return agents[v];
    } else {
      return visits[v - na].pos;
    }
  }
  double distance(const Point &a, const Point &b) const {
    return hypot(a.x - b.x, a.y - b.y);
  }
  double distance(int v1, int v2) const {
    const Point &p1 = pos(v1), &p2 = pos(v2);
    return distance(p1, p2);
  }
  int slot(int v) const {
    return visits[v - na].slot;
  }
};

int main() {
  int na;
  cin >> na;
  vector<Point> agents;
  for (int i = 0; i < na; i++) {
    double x, y;
    cin >> x >> y;
    agents.push_back(Point{x, y});
  }
  int nv;
  cin >> nv;
  vector<Visit> visits;
  for (int i = 0; i < nv; i++) {
    int slot, pool;
    double x, y;
    cin >> slot >> x >> y >> pool;
    visits.push_back(Visit{slot, Point{x, y}, pool});
  }

  MIPSolver mip;
  vector<vector<vector<Variable>>> assign(na, vector<vector<Variable>>(na + nv));
  HouseCalculator h{na, nv, agents, visits};
  for (int a = 0; a < na; a++) {
    for (int v1 = 0; v1 < na + nv; v1++) {
      for (int v2 = 0; v2 < na + nv; v2++) {
        if (h.ishouse(v1)) {
          // Origin is a house.
          if (!h.ishouse(v1, a)) {
            // Visit is invalid if origin is a house not owned by agent.
            assign[a][v1].push_back(NullVariable());
          } else {
            // It's ok from owned house to any visit or itself.
            assign[a][v1].push_back(mip.binary_variable(h.distance(v1, v2)));
          }
        } else {
          // Origin is a visit.
          if (h.ishouse(v2) && !h.ishouse(v2, a)) {
            // Visit is invalid if destination is a house not owned.
            assign[a][v1].push_back(NullVariable());
          } else if (!h.ishouse(v2) && h.slot(v1) >= h.slot(v2)) {
            // Can't travel backwards in time.
            assign[a][v1].push_back(NullVariable());
          } else {
            // Otherwise allowed.
            assign[a][v1].push_back(mip.binary_variable(h.distance(v1, v2)));
          }
        }
      }
    }
  }
  // Every agent must leave the house.
  for (int a = 0; a < na; a++) {
    auto cons = mip.constraint();
    for (int v = 0; v < na + nv; v++) {
      cons.add_variable(assign[a][a][v], 1);
    }
    cons.commit(1, 1);
  }
  // Every visit must have one agent.
  for (int v = na; v < na + nv; v++) {
    auto cons = mip.constraint();
    for (int a = 0; a < na; a++) {
      for (int v2 = 0; v2 < na + nv; v2++) {
        cons.add_variable(assign[a][v][v2], 1);
      }
    }
    cons.commit(1, 1);
  }
  // If you enter a visit, you must exit the visit.
  for (int v = na; v < na + nv; v++) {
    auto cons = mip.constraint();
    for (int a = 0; a < na; a++) {
      for (int v2 = 0; v2 < na + nv; v2++) {
        cons.add_variable(assign[a][v][v2], 1);
        cons.add_variable(assign[a][v2][v], -1);
      }
    }
    cons.commit(0, 0);
  }
  // Solve and print.
  auto sol = mip.solve();
  for (int a = 0; a < na; a++) {
    for (int v1 = na; v1 < na + nv; v1++) {
      for (int v2 = 0; v2 < na + nv; v2++) {
        if (sol.value(assign[a][v1][v2]) > 0.5) {
          cout << "agent " << a << " from " << v1 << " to " << v2 << "\n";
          cerr << v1 - na << " " << a << "\n";
        }
      }
    }
  }
  return 0;
}
