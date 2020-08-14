// ----------------------- MALLOC AND ITERATE TAG SEQUENCE --------------------------------------------------------------
// iteratetagsequence needs to be able to malloctagsequence for next subset ....

// these two functions recursively call each other
EPt *malloctagsequence(CS tag, EPt *parent);
VPt iteratetagsequence(EPt *etag);

EPt *malloctagsequence(CS tag, EPt *parent) {
  EPt *etag = (EPt *)malloc(sizeof(EPt));
  etag->mode = '!';   // mode not set
  etag->action = '!'; // error action
  etag->start = Pt(0.0, 0.0);
  etag->prev = parent; // if parent is NULL, this is the last (first) sequence (last to process)
  etag->next = NULL;
  if (!tag) {
    cout << "null tag input for tag sequence malloc\n";
    return etag;
  } else if (!etag) {
    cout << "tag sequence malloc fail\n";
    return etag;
  } else if (*tag == ':' || *tag == '%') {
    etag->mode = *tag;       // mode set accordingly
    etag->tag = ++tag;       // wait at next char (first action char)
    etag->action = (*tag) ? ';' : '.'; // wait action unless no actions
    if (parent) {
      if (parent->next) {
        cout << "currently not expecting parent with existing iterant\n";
        return NULL; // something will break
      } else { parent->next = etag; } // etag->prev was set earlier
      etag->start = parent->spotpoints[0]; // inherit parent's current position
      etag->here = etag->start;
      etag->shift = Pt(0.0, 0.0);
      etag->spotpoints[0] = etag->start;
    } else { // else etag->prev is already NULL (this is top-level!)
      etag->start = Pt(0.0, 0.0); // parent function needs to set starting point !
      etag->here = etag->start;
      etag->shift = Pt(0.0, 0.0);
      etag->spotpoints[0] = etag->start;
      cout << "    { "; // start new drawing
    }
    // iterate function mallocs subsequence then calls iterate on it for first point
  } else {
    cout << "sequence without mode header or invalid mode\n";
    return etag;
  }
  return etag;
}
