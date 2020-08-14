// returns one or none, matchnexttag should include prev pointer
// but actually, just call matchtag after this @ (tag[1]) to start after current match
FP fractionof(UI leadingzeroes, UI fraction) {
  if (!fraction) { return 0.0; } // regardless of leading zero count 0.000000
  else { // offset fraction by fractiondigits + leadingzeroes
    FP fractionoffset = 0.1;
    UI fractiondigits = fraction;
    while ((fractiondigits /= 10))
      { fractionoffset /= 10.0; }
    while (leadingzeroes)
      { fractionoffset /= 10.0; leadingzeroes--; }
    return ((FP)fraction * fractionoffset);
  } // cannot be negative!
}
FP floatingpoint(UI neg, UI value, UI zeroes, UI fraction) {
  if (neg && !value && !fraction) { // -0.[0..] // zeroes should be the number of zeroes
    if (zeroes == 0) { // -0.
      cout << "no definition for '-0.'\n";
      return 0.0;
    } else if (zeroes == 1) { // -0.0
      return 0.0; // -0.0 is 0.0
    } else { // -0.00, -0.000 etc .....
      return 0.0; // no special meaning
    }
  } else if (!neg && !value && !fraction) { // 0.[0..]
    if (zeroes == 0) { // 0 or 0. (0. is the same as 0)
      return 0.0;
    } else if (zeroes == 1) { // 0.0
      return 0.0; // 0.0 is 0.0
    } else { // 0.00, 0.000 etc .....
      return 0.0; // no special meaning
    }
  } else {
    FP wholenumber = (FP)value;
    FP wholefraction = fractionof(zeroes, fraction);
    FP wholefloat = wholenumber + wholefraction;
    return (neg) ? 0.0 - wholefloat : wholefloat;
  }
}
FP readadecimal(CS *tagpp) {
  CS tagp = *tagpp;
  UI tagrefneg = 0;
  UI tagref = 0;
  UI tagrefdot = 0;
  UI tagreffrac = 0;
  UI tagreffracleadingzeroes = 0;
  while (*tagp) {
    if ((!tagref) && (!tagrefdot) && (*tagp == '-')) {
      tagrefneg = 1; // 00000-0.1 is -0.1 still ....?
    } else if ((*tagp >= '0') && (*tagp <= '9')) {
      if (tagrefdot) {
        if (*tagp == '0' && !tagreffrac) {
          tagreffracleadingzeroes++;
        } else {
          tagreffrac = (tagreffrac * 10) + (*tagp - '0');
        }
      } else {
        tagref = (tagref * 10) + (*tagp - '0');
      }
    } else if ((!tagrefdot) && (tagp[0] == '.')) {
      tagrefdot = 1;
//    } else if (tagp[0] == ',') {
    } else { // including comma
      break;
    }
    tagp++;
  }
  *tagpp = tagp;
  return floatingpoint(tagrefneg, tagref, tagreffracleadingzeroes, tagreffrac);
}


