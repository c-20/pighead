
// todo: allow (cos t) syntax
// ellipse t = 0 to 360
Pt ellipse(Pt start, FP t, FP w, FP h) {
  FP radiant = t * (pi / 180.0);
  FP deltax = (w / 2.0) * cos(radiant);
  FP deltay = (h / 2.0) * sin(radiant);
  return Pt(start.x - deltax, start.y - deltay); // flip x and y !
}

// line t = 0 to w or h
Pt line(Pt start, FP t, FP w, FP h) {
  FP absw = fabs(w);
  FP absh = fabs(h);
  FP tmax = (absw > absh) ? absw : absh;
  FP tpct = t / tmax;
  FP deltax = w * tpct;
  FP deltay = h * tpct;
  return Pt(start.x + deltax, start.y + deltay);
}

