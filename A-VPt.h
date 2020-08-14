struct Pt {
  FP x, y;
  Pt() : x(0.0), y(0.0) { }
  Pt(FP x, FP y) : x(x), y(y) { }
//  Pt operator+(Pt &p) { return Pt(this->x + p.x, this->y + p.y); }
//  Pt operator-(Pt &p) { return Pt(this->x - p.x, this->y - p.y); }
//  Pt operator+=(Pt &p) { return Pt(this->x + p.x, this->y + p.y); }
//  Pt operator-=(Pt &p) { return Pt(this->x - p.x, this->y - p.y); }
//  ostream<char> operator<<(Pt &p) { return "(" << p.x << "," << p.y << ")"; }
};

struct EPt { // all object points are ellipse points for now
  CH mode;       // ':' or '%' for interpreting move actions
  CH action;     // 'E' for ellipse, 'L' for line
  FP s, xs, ys;
  FP w, h; // for line
  FP time, limit, step; // for 'E' time is angle in degrees
                        // for 'L' time is distance% along gradient
  Pt start;   // starting point
  Pt here;    // current point: reference location for step functions
  Pt shift;   // origin centre (here) shift per iteration

//  Pt point;   // current point <---- adjust spotpoints[0] instead? probably ... not
  CS tag;     // drawing details
  Pt spotpoints[10]; // memory for points (0 should be currentpoint
  EPt *prev;
  EPt *next;
};

struct VPt {
  Pt point;
  EPt *eref;
  VPt() : point(Pt(0.0, 0.0)), eref(NULL) { }
}; // is this valid C++ but not valid C? in C:

