// tag sequence with mouse parent
VPt newmousepoint(Pt position, CS tag) {
  VPt mouse;
  mouse.point = position;
  mouse.eref = malloctagsequence(tag, NULL);
  if (!mouse.eref) {
    cout << "failed to initialise tag processing\n";
    return mouse;
  } else { // need to manually set starting position
    mouse.eref->start = position;
    mouse.eref->here = position;
    mouse.eref->spotpoints[0] = position;
  }
  return mouse;
}

SI lastlevel = 0;
VPt nextmousepoint(VPt mouse) {
  VPt next;
  next.point = mouse.point;
  next.eref = NULL;
  SI level = 0;
  if (mouse.eref) {
    // expect previous mouse point to return endmost subsequence
    // as its values may have been needed by the parent function
    EPt *topleveleref = mouse.eref;
    while (topleveleref->prev) {
      topleveleref = topleveleref->prev;
      level++;
    }
    if (level != lastlevel) {
//      cout << "(" << level << ")";
      lastlevel = level;
    }

    VPt incremented = iteratetagsequence(mouse.eref);
    next.point = incremented.point; // always or conditional??
    next.eref = incremented.eref;   // break on NULL ?
  } else {
    cout << "<NO ETAG>\n";
  }
  return next;
}

