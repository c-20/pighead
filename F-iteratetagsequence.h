// ----------------------- MALLOC AND ITERATE TAG SEQUENCE --------------------------------------------------------------

// iteratetagsequence needs to be able to malloctagsequence for next subset ....
SI totalnestediterations = 0;
SI iterationlimit = 10000;
VPt iteratetagsequence(EPt *etag) {
  VPt vtag;
  vtag.point = Pt(0.0, 0.0); // needs to be set from etag spotpoints[0]
  vtag.eref = etag;
  SI todomatch = 0;  // special handling routine for match on loop start / iteration
  SI todofinish = 0; // special handling routine for loop end when limit reached
  if (!etag) {
    cout << "null iterate sequence input\n";
    etag->action = '!';
    vtag.eref = NULL;
    return vtag;
  } else if (etag->next) {
    if (++totalnestediterations >= iterationlimit) {
      cout << "ITERATION LIMIT REACHED\n";
      etag->action = '!';
      vtag.eref = NULL; // note: does not deallocate
      return vtag;
    } // else jump into subsequence by iterating it (RECURSION WARNING)
    if (etag->action >= 'A' && etag->action <= 'Z') {
      cout << "WAITINGFORMATCH";
//cout << etag->action;
//      cout << "in a loop, waiting for match-related subsequence\n";
    } else if (etag->action == ';') {
      cout << "WAITINGFORWUT"; // first iteration call to subsequence may end up here
//      cout << "waiting, waiting for .... recently ended subsequence?\n";
    } else if (etag->action == '!') {
      cout << "leaving because error\n";
      vtag.eref = NULL;
      return vtag;
    } else {
      cout << "leaving because confused\n";
      vtag.eref = NULL;
      return vtag;
    } // else looping or waiting, safe to iterate subsequence
    VPt subpt = iteratetagsequence(etag->next);
    if (!subpt.eref) { // the object deallocated and has no point !
      if (etag->next) {
        cout << "subsequence failed to sync with parent sequence list\n";
        etag->action = '!'; // change action to prevent recurse
        return vtag; // null etag first?
      // else if (etag and subpt.eref are different, subpt is a sub[sub[sub]]subpt)
      } else { // subsequence deallocated, list synced
        // vtag.point = subpt.point; // final point should be (0, 0) -- nope, deprecated
        vtag.point = etag->spotpoints[0]; // return current position of this tag instead
        // vtag.eref = etag; presumed to still be true
        return vtag;
      }
    } else { return subpt; } // sequence iterated, subpt object still active
  } else if (etag->action == '.') { // if iterating a stopped sequence, deallocate !
    // subsequences ending on an offset action (e.g. j1) will quickdealloc
    // objects that end after step actions (via todofinish) deallocate here
    if (etag->next) {
      cout << "not expecting a stopped sequence to have an active subsequence ! (it should be looping @ or waiting maybe)\n";
      vtag.eref = NULL;
      return vtag;
    } else { // parent returns its spotpoints[0] as it was overridden by subsequence start
      if (etag->prev) { // return spotpoints[0] of calling ancestor
        if (etag->prev->action >= 'A' && etag->prev->action <= 'Z') {
          vtag.point = etag->prev->spotpoints[0];
        } else {
          cout << "RETURNING TO NON-STEP ACTION WUT\n";
          vtag.point = etag->prev->spotpoints[0];
          // todo: offset on return ?
        }
        vtag.eref = etag->prev;
        etag->prev->next = NULL;
      } else { // top-level object returns (0, 0), NULL on deallocation
        // cout << "TOP LEVEL 0 RETURN\n";
        vtag.point = Pt(0.0, 0.0);
        vtag.eref = NULL;
      }
      free(etag);
      // cout << "DEALLOC\n";
      return vtag;
    }
  // ------------------------ RUN: handle offsets, initialise a loop ..... ---------------------------------------
  } else if (etag->action == ';') { // if waiting, run !
    etag->action = ':'; // run (then ; wait after action or . stop if nothing after)
    CH offsetmode = etag->mode;
    Pt offsetpoint = etag->spotpoints[0]; // spotpoints[0] is current point ! (set from prev at malloc)
    FP norwidth = 0.0;
    FP norheight = 0.0; // todo: import window size?
    FP nortime = 0.0;   // top-level object will have 0 values
    if (!etag->prev) {
      if (offsetmode == '%') {
        cout << "no percentage at top-level object - create parent with ->w and ->h\n";
        cout << "todo: import current screen or desktop size\n";
        etag->action = '!';
        return vtag;
      } // else pixel mode : shouldn't rely on norvalues
    } else { // if % mode but : should ignore
      norwidth = etag->prev->w; // parent object width used for %, for : it should cancel out
      norheight = etag->prev->h; // used for %
      nortime = etag->prev->time; // uses time value of parent (step) object !
    }
    while (etag->tag[0] >= 'a' && etag->tag[0] <= 'z') { // lowercase for offsets
      CH offsetaction = etag->tag[0]; // NOTE!!!!! etag->mode could be : not % !!!!!!!!!! BUGG!!TODO!!!
      CS tagp = &(etag->tag[1]);
      FP firstvalue = readadecimal(&tagp); // assume all offsets have at least one input value
      if (offsetaction == 'd') {
        FP downoffset = (offsetmode == '%') ? (norheight * (firstvalue / 100.0)) : firstvalue;
        offsetpoint.y += downoffset; // actually, downoffset is the % of the height to add on .....
      } else if (offsetaction == 'u') {
        FP upoffset = (offsetmode == '%') ? (norheight * (firstvalue / 100.0)) : firstvalue;
        offsetpoint.y -= upoffset;
      } else if (offsetaction == 'l') {
        FP leftoffset = (offsetmode == '%') ? (norwidth * (firstvalue / 100.0)) : firstvalue;
        offsetpoint.x -= leftoffset;
      } else if (offsetaction == 'r') {
        FP rightoffset = (offsetmode == '%') ? (norwidth * (firstvalue / 100.0)) : firstvalue;
        offsetpoint.x += rightoffset;
      } else if (offsetaction == 'o' || offsetaction == 'i') { // consider out-vertical and out-horizontal are possible too 'h' 'v'
        FP outvalue = (offsetaction == 'i') ? (0.0 - firstvalue) : firstvalue;
        FP outscale = 1.0 + (outvalue / 100.0); // value = -10% -> 1.0-0.1
        FP outwidth  = (offsetmode == '%') ? (norwidth  * outscale) : (norwidth  + outvalue);
        FP outheight = (offsetmode == '%') ? (norheight * outscale) : (norheight + outvalue);
        Pt norpoint = ellipse(offsetpoint, nortime, norwidth, norheight); // or (0.0, 0.0)
        Pt outpoint = ellipse(offsetpoint, nortime, outwidth, outheight); // but relative anyway
        Pt outdiff = Pt(outpoint.x - norpoint.x, outpoint.y - norpoint.y);
        offsetpoint.x += outdiff.x;
        offsetpoint.y += outdiff.y;
      } else if (offsetaction == 'b' || offsetaction == 'f') {
        FP forwarddegrees = (offsetaction == 'b') ? (0.0 - firstvalue) : firstvalue;
        FP forwardtime = nortime + forwarddegrees;
        Pt norpoint     = ellipse(offsetpoint, nortime,     norwidth, norheight);
        Pt forwardpoint = ellipse(offsetpoint, forwardtime, norwidth, norheight);
        Pt forwarddiff = Pt(forwardpoint.x - norpoint.x, forwardpoint.y - norpoint.y);
        offsetpoint.x += forwarddiff.x;
        offsetpoint.y += forwarddiff.y;
      } else if (offsetaction == 's') {
        UI uispotindex = (UI)(SI)firstvalue;
        if (uispotindex >= 0 && uispotindex <= 9) {
          etag->spotpoints[uispotindex] = offsetpoint;
        } else {
          cout << "invalid spot index " << uispotindex << "\n";
        }
      } else if (offsetaction == 'j') {
        UI uispotindex = (UI)(SI)firstvalue;
        if (uispotindex >= 0 && uispotindex <= 9) {
          offsetpoint = etag->spotpoints[uispotindex];
        } else {
          cout << "invalid spot index " << uispotindex << "\n";
        }
      } else {
        cout << "unrecognised offsetaction " << offsetaction << "\n";
        etag->action = '!';
        vtag.eref = NULL;
        return vtag;
      } // end of offsetaction switch
      etag->tag = tagp; // tag should be at next offset/action
//      if (!tagp) { cout << "no tag\n"; } // expect tag..
      etag->spotpoints[0] = offsetpoint; // update after each offset, here
      etag->here = etag->spotpoints[0];  //  implies no active step action
    } // end of offsetaction loop
    // should not return early without reason ... maybe there's an extra iteration?
    //    vtag.point = etag->here; // update in case of early return
    // assert action still equals :
    // if ending action was in wait state, it ends up here, so forego detection
    // of missing step action so that @ and ] and \0 can be quietly handled
    if (etag->tag[0] == '@' || etag->tag[0] == ']') { // end of subsequence
      // best to deallocate here
      // cout << "DEALLOCAFTEROFFSET";
      if (etag->prev) {
        vtag.eref = etag->prev;
        etag->prev->next = NULL;
        vtag.point = etag->here; // actually, it wasn't set ... maybe this is the fix
        // point was set by offset ... j0 for example ...................
//        vtag.point = vtag.eref->spotpoints[0];
      } else { // top-level object returns (0, 0), NULL on deallocation
        // cout << "TOPLEVELDEALLOCAFTEROFFSET\n";
        vtag.point = Pt(0.0, 0.0);
        vtag.eref = NULL;
      }
      free(etag);
      return vtag;
    } else if (etag->tag[0] == '\0') { // end of top level sequence
      cout << "TOPDEALLOC\n";
      cout << "dealloc seems to happen elsewhere, todo:confirm\n";
      if (etag->prev) {
        cout << "non-top-level object finds tag end\n";
        vtag.eref = NULL;
        vtag.point = Pt(0.0, 0.0);
      } else {
        vtag.eref = NULL;
        vtag.point = Pt(0.0, 0.0);
      } // same effect, no message
      free(etag);
      return vtag;
    } else if (etag->tag[0] >= 'A' && etag->tag[0] <= 'Z') { // uppercase for actions
      CH stepaction = etag->tag[0];
      CS tagp = &(etag->tag[1]);
      if (stepaction == 'E' || stepaction == 'B') {
        cout << "(" << stepaction;
        etag->action = stepaction; // 'B' iterates differently
        FP baseradiusvalue = readadecimal(&tagp);
        // it may be based on a height but it's a radius
        if (offsetmode == '%') {
          baseradiusvalue = norheight * (baseradiusvalue / 100.0) / 2.0; // half-height percentage
        } else { baseradiusvalue /= 2.0; } // value given is pixel diameter height
        FP xratiovalue = 1.0; // s-sx radius ratio
        if (*tagp == ',') {
          tagp++;
          xratiovalue = readadecimal(&tagp);     // E,xratio..[]
        } // Eradius[,xradiusratio]
        etag->w = baseradiusvalue * xratiovalue * 2.0;
        etag->h = baseradiusvalue * 2.0;
        etag->step = 2.0;    // every 5 degrees in case of triggers
        // = 180 points ... todo: step modifier ?
        if (*tagp == ',') {
          tagp++;
          etag->time = readadecimal(&tagp);      // E,,angle..[]
        } else { etag->time = 0.0; }             // start of arc
        if (*tagp == ',') {
          tagp++;
          etag->limit = 360.0 * readadecimal(&tagp); // E,,,rotations..[]
        } else { etag->limit = 360.0; }          // end of arc
        if (*tagp == ',') {
          tagp++;
          FP stepsperrotation = 360.0 / etag->step; // points per circle
          FP shiftangle = readadecimal(&tagp);
          FP shiftvelocityx = etag->w / stepsperrotation; // default one diameter per rotation
          FP shiftvelocityy = etag->h / stepsperrotation; // default one diameter per rotation
          if (*tagp == ',') {
            tagp++;
            FP shiftvelocity = readadecimal(&tagp);       // or n*diameter pixels per rotation
            shiftvelocityx = (etag->w * shiftvelocity) / stepsperrotation;
            shiftvelocityy = (etag->h * shiftvelocity) / stepsperrotation;
          }
          etag->shift = ellipse(Pt(0.0, 0.0), shiftangle, shiftvelocityx, shiftvelocityy);
          // if shifting, then the here point is offset so that first point is parent spotpoints[0] 
          Pt curlstart = ellipse(Pt(0.0, 0.0), etag->time, etag->w, etag->h); // get the offset
          etag->here.x -= curlstart.x;
          etag->here.y -= curlstart.y;     // jump that far away
        } else { etag->shift = Pt(0.0, 0.0); }   // 0 from malloc unless >1 step action in a sequence 
        etag->spotpoints[0] = ellipse(etag->here, etag->time, etag->w, etag->h);
        todomatch = (*tagp == '[') ? 1 : 0; // match for [matchset] but not iterator *
      } else if (stepaction == 'L') {
        cout << "(" << stepaction;
        etag->action = 'L';
        Pt fromspot, tospot;
        FP fpfromspotindex = readadecimal(&tagp);
        UI uifromspotindex = (UI)(SI)fpfromspotindex;
        if (uifromspotindex >= 0 && uifromspotindex <= 9) {
          fromspot = etag->spotpoints[uifromspotindex];
        } else {
          cout << "invalid spot index in line draw\n";
          etag->action = '!';
          return vtag;
        }
        if (*tagp == ',') {
          tagp++;
          FP fptospotindex = readadecimal(&tagp);
          UI uitospotindex = (UI)(SI)fptospotindex;
          if (uitospotindex >= 0 && uitospotindex <= 9) {
            tospot = etag->spotpoints[uitospotindex];
          } else {
            cout << "invalid second spot index in line draw\n";
            etag->action = '!';
            return vtag;
          }
        } else {
          cout << "missing second spot index in line draw\n";
          etag->action = '!';
          return vtag;
        }
        etag->spotpoints[0] = fromspot;  // set current position to starting point
        etag->here = fromspot;           // start step function at fromspot
        etag->w = tospot.x - fromspot.x; // radiusvalue * xradiusvalue;
        etag->h = tospot.y - fromspot.y; // radiusvalue * yradiusvalue;
        etag->time = 0.0;                // start of line at t = 0        
        etag->step = 1.0;                // 1 pixel per increment
        FP absw = fabs(etag->w);         // ->w and ->h store negatives,
        FP absh = fabs(etag->h);         // ->limit and ->step are positive amplitude sweep
        etag->limit = (absw > absh) ? absw : absh; // num increments = num px in longer dimension
        // consider % time if calculated from %? seems logical ..... consider fraction triggers
        etag->spotpoints[0] = line(etag->here, etag->time, etag->w, etag->h);
        // if *tagp != '*' or '%', error: expected iterator or matchset
        todomatch = (*tagp == '[') ? 1 : 0; // match for [matchset] but not iterator *
        // return vtag? not until 0.0 is detected for matching tags!!!!
      } else {
        cout << "unrecognised step action " << stepaction << "\n";
        etag->action = '!';
        return vtag;
      }
      etag->tag = tagp; // be sure to update the tag to new offset position (* or [matchset])
      vtag.point = etag->spotpoints[0]; // save the first point for return
      // if there's a match, subsequence happens first, todomatch overrides this point with
      // first subsequence point. this spotpoints[0] returned when subsequence deallocates.
    } else { // end of step-action A-Z detect, should have started one action, todo match if []
      cout << "step action expected\n";
      cout << "tag: " << etag->tag << "\n";
      etag->action = '!';
      vtag.eref = NULL;
      return vtag;
    }
  // -------------------- LOOP: iterate a loop -------------------------------------------------------------------
  // if tag is not in wait or deallocate mode and actioned as above, it should be in step mode
  } else if (etag->action >= 'A' && etag->action <= 'Z') { //(etag->action == '@') {
    // a step action needs to increment for the next iteration, then search if []
    // note that the initial setup of 'E' and 'L' don't test if time < limit
    etag->here.x += etag->shift.x;
    etag->here.y += etag->shift.y;
    etag->time += etag->step;
    if (etag->time >= etag->limit) {
      FP overrun = etag->time - etag->limit; // -bound
//      if (overrun > 0.0) {
//        cout << "overrun: " << overrun << "\n";
//      } // ignoring overrun because hooks to limit anyway
      etag->time = etag->limit;
      todofinish = 1;
    }
    if (etag->action == 'E' || etag->action == 'B') {
      etag->spotpoints[0] = ellipse(etag->here, etag->time, etag->w, etag->h);
      if (etag->action == 'B' && todofinish == 1) { // if 'B' time at limit angle
        if (etag->w > 1.0 && etag->h > 1.0) { // if still more steps to fill
          etag->w -= 2.0;
          etag->h -= 2.0;
          etag->time = 0.0; // TODO: if a start angle was given, go back to the start angle !
          todofinish = 0;
        } // else { todofinish stays 1 }
      }
    } else if (etag->action == 'L') {
      etag->spotpoints[0] = line(etag->here, etag->time, etag->w, etag->h);
    } else {
      cout << "unrecognised step action continuation\n";
      etag->action = '!';
      return vtag;
    }
    vtag.point = etag->spotpoints[0];
    // if a match triggers, a subsequence will start and return its point
    // this point will be retrieved again upon subsequence deallocation
    CS tagp = etag->tag;
    todomatch = (*tagp == '[') ? 1 : 0; // if stepping, do match ... including last step to >=limit
  } else if (etag->action == ':') {
    cout << "running tag did not return to wait state\n";
//    etag->action = ';'; // next iteration will run
    etag->action = '!'; // don't expect this to happen ....
    return vtag; // assumes vtag.ref == etag
  } else {
    cout << "invalid etag action\n";
    cout << "action: " << etag->action << "\n";
    etag->action = '!'; // prevent recurse
    vtag.eref = NULL; // do not continue
    return vtag; // (TODO: dealloc eref? no, to prevent crash-on-cancel)
  }
  // ------------------- @ iterated, : run until expected @ ... in both cases, todomatch
  // so, when would todomatch be false? if the iterator is *
  // ------------------- LOOP: for start or an iteration, match if [matchset] -----------------------------------------
  if (todomatch == 1) { // expect etag->action to be '@' for start or iteration
    CS tagp = etag->tag;
    if (!*tagp || *tagp != '[') {
      cout << "todomatch without [matchset]\n";
      etag->action = '!'; // error action
      return vtag;
    } // else { // matchset []
    while ((tagp = matchtag(tagp, etag->time))) {
      if (!*tagp) { // no more @triggers to check
        cout << "condition should have been false\n";
        break;
      } else if (*tagp == '@') { // this trigger did not match
        cout << "@";
        continue; // ready to try next match on next iteration
      } else if (*tagp == ':' || *tagp == '%') {
        // a tag matched and a subsequence needs to me malloced
        if (etag->next) {
          cout << "unexpected subsequence exists instantiating subsequence\n";
          etag->action = '!';
          return vtag;
        } // else instantiate a subsequence
        EPt *subsequence = malloctagsequence(tagp, etag);
        if (!subsequence) {
          cout << "no subsequence instantiated\n";
          etag->action = '!';
          return vtag;
        } else {
          cout << "[";
          VPt subiteration = iteratetagsequence(subsequence);
          if (!subiteration.eref) {
            cout << "sequence deallocated on first iteration (!)\n"; // this would be normal for tag = ""
            vtag.point = etag->spotpoints[0]; // or ->here ?
            //etag->action = '!';
            //return vtag;         // return the point for here
            // still continue down to todofinish
          } else {
            if (subiteration.eref->action == '.') {
              cout << "subsequence deallocated on first iteration (.)\n";
              vtag.point = etag->spotpoints[0]; // return to parent spotpoint 0
              vtag.eref = etag;
              cout << ">!<]";
            } else { vtag = subiteration; }   // return the point for the subiteration
//          todofinish = 0; // don't end the loop if a subiteration has started... actually, do ....
            break; // only allow one match ! (otherwise have to check for next->next and which first)
          } // cannot return subiteration in case limit reached and loop must end
        }
      } else {
        cout << "unexpected result from tagmatch\n";
      }
    } // end of while matchtag
  } // end of todomatch - it will only allow one match for now !
  // ------------------- FINISH: if time >= limit, finish loop and continue --------------------------------------  
  if (todofinish == 1) { // after todomatch in case of match at finish point
    cout << ")";
    CH finishedaction = etag->action;
    if (finishedaction == 'E' || finishedaction == 'B') {
      vtag.point = etag->spotpoints[0]; // still return the last ellipse point
      etag->spotpoints[0] = etag->here; // then ellipse returns to centrepoint
      // note: this point may have shifted ! save before and jump after to undo~~~!!!!
    } else if (finishedaction == 'L') {
      etag->here = etag->spotpoints[0]; // line updates here to new location
      vtag.point = etag->here;          // new location is the last line point
    } else {
      cout << "unrecognised action at todofinish\n";
      etag->action = '!';
      return vtag;
    }
    CS tagp = etag->tag;
    if (*tagp == '*') { tagp++; } // move past iterator *
    else if (*tagp == '[') { // move past matchset [....]
      tagp++;
      SI level = 1;
      while (*tagp) {
        if      (*tagp == '[' && level > 0) { tagp++; level++;  } // assume no neglevel traps
        else if (*tagp == ']' && level > 1) { tagp++; level--;  } // decrement sublevels
        else if (*tagp == ']')              { tagp++; break;    } // next byte in sequence is here
        else if (level >= 1)                { tagp++; continue; } // skip any chars between [ and ]s
        else {
          cout << "this condition should be unreachable\n";
          break;
        }
      }
      if (level != 1) { // !*tagp ||
        cout << "unbalanced level skipping non-matching subsequence\n";
        etag->action = '!';
        return vtag;
      } // in case of top object, end of tag after [matchset] is acceptable
    } else {
      cout << "an iterator * or matchset [] was expected in the sequence\n";
      return vtag;
    } // tagp should now be positioned after the * or [matchset]
    etag->tag = tagp; // save the new tag position, then check for NULL
    // check for end of subsequence
    if (*tagp && (*tagp == '@' || *tagp == ']')) { // this subsequence ends
      etag->action = '.';
      cout << "]";
    } else if (!*tagp) {  // top-level subsequence ends
      etag->action = '.';
      cout << " }\n";
    } else {              // subsequence continues
      etag->action = ';';
      cout << ".";
    }
  }
  return vtag; // return here after loop start or iteration, including if subsequence allocated
}
