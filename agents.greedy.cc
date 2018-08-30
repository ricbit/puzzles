#include <iostream>
#include <cmath>
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

double dist(Point &a, Point &b) {
  return hypot(a.x - b.x, a.y - b.y);
}

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
  vector<int> assigned(nv, -1);
  for (int a = 0; a < na; a++) {
    int curv = -1;
    Point curp = agents[a];
    int bestv = -1;
    int curslot = -1;
    do {
      bestv = -1;
      for (int v = 0; v < nv; v++) {
        if (assigned[v] >= 0 || visits[v].slot <= curslot) {
          continue;
        }
        if (bestv == -1) {
          bestv = v;
        } else if (visits[v].slot < visits[bestv].slot) {
          bestv = v;
        } else if (visits[v].slot == visits[bestv].slot &&
            dist(curp, visits[v].pos) < dist(curp, visits[bestv].pos)) {
          bestv = v;
        }
      }
      if (bestv >= 0) {
        curv = bestv;
        curp = visits[bestv].pos;
        assigned[curv] = a;
        curslot = visits[bestv].slot;
      }
    } while (bestv >= 0);
  }
  for (int v = 0; v < nv; v++) {
    cout << v << " " << assigned[v] << "\n";
  }
  return 0;
}
