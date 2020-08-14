CS matchtag(CS tag, FP ref) {
  // only match tags at level 1! i.e. tag should have [@:@:@:[@:[@:@:@:]@:]@:[@:@:]] syntax !
  // either return the matching subsequence (first char is mode : or %),
  //   or the ] ending the list (note: iterate full match set [@%@%@%@%] each iteration)
  // presuming negative matchref matches work fine
  SI level = 0; // allow negative in type or it rolls down to positive !
  FP matchtolerance = fptolerance; // assume error < 10^-12
  CS tagp = tag;
  if (*tagp == '[')      { level++; tagp++; } // this is a first-match (eat [)
  else if (*tagp == '@') { level++; } // this is a next-match ([ was eaten previously)
  while (level >= 1) { // careful of INFINITE LOOP ! tagp++ required for each condition !
    if (!*tagp) {
      cout << "unexpected end of tag in [matchset]\n";
      return NULL;
    } else if (*tagp == '[') { level++; tagp++; } // this is a subset
    else if (*tagp == ']')   { level--; tagp++; } // end of a subset (or this set)
    else if (level == 1 && *tagp == '@') { // @match on this level
      tagp++;
      FP tagfloat = readadecimal(&tagp);
      FP tagdiff = tagfloat - ref;
      if (tagdiff < matchtolerance && tagdiff > -matchtolerance) {
        if (*tagp == ']') { // single match without action
          cout << "match without action\n";
          return NULL;
        } else if (*tagp == '@') { // this is a multimatch with further matches in list
          while (*tagp && (*tagp == '@' || (*tagp >= '0' && *tagp <= '9') || *tagp == '.' || *tagp == '-'))
            { tagp++; } // multimatch skips past matches that follow @60@90@120@150
          if (!*tagp) { // unexpected end of tag
            cout << "unexpected end of tag (0) after @multi@match\n";
            return NULL; // no more matches to process ! (instead of :... or @... for todo or nextmatch)
          } else if (*tagp == ']') { // multimatch without action
            cout << "unexpected end of tag list (]) after @multi@match\n";
            return NULL; // no more matches to process since ] returns to level 0
//        } else if (*tagp == '[') { not expecting @10@20[....] but rather @10@20:E[@0:L1,2]]
// but maybe it can be used to initialise sensors or registers ?
          } // these are warnings, return tag position anyway ? or return NULL
        } // with multimatches jumped past, test for action, ':' for pixel offsets, '%' for percent offsets
        if (*tagp == ':' || *tagp == '%') {
          return tagp; // return action sequence, parent will create next EPt
        } else {
          cout << "invalid char, was expecting an action\n";
          return NULL;
        }
      } else { // the number does not match
        // need to skip past non-matching content
        // expect tagp to be : or % but skip until @ or ] while ignoring subsets....
        //tagp++;
        // tagp = skipuntilnextmatch();
        SI level = 1;
        while (*tagp) { // skips until next match @ or end of list ]
          if      (*tagp == '[' && level > 0) { tagp++;  level++; } // assume no neglevel traps
          else if (*tagp == ']' && level > 1) { tagp++;  level--; } // decrement sublevels
          else if (level > 1)                 { tagp++;           } // do skip @s in nested []
          else if (*tagp == ']' || *tagp == '@') {         break; } // break on level 1 @ or ] marker
          else if (level == 1)                { tagp++;           } // skip any non @ or ] at level 1
          else {
            cout << "this condition should be unreachable\n";
            break;
          }
        }
        if (!*tagp || level != 1) {
          cout << "expected @ or ] skipping non-matching subsequence (level imbalance?)\n";
          return NULL; //          etag->action = '!';
        }
      }
    } else if (level > 1) {
      cout << "TESTFLAG001\n";
      // expect to see ...................................?
      tagp++; // any char within [ and ] subsets is ignored
    } else { // anything not [, ] or @, or above level 1 are invalid since processing should have otherwise occured
      cout << "unexpected character " << *tagp << " in match set\n";
      return NULL;
    }
  } // end of while loop -- needs to tagp++ to continue !
  if (level != 0) { // expect balanced loop result
    cout << "mishandled ] caused a level error\n";
    return NULL;
  } else {
    return NULL; // end of list was reached, so NULL is returned
  }
} // returns NULL if error or end of list, ACTION (: or %) if a match was found
