#include <cmath>
#include <iostream>
#include <utils.h>
#include <interception.h>

#include <windows.h>

using namespace std;
namespace scancode {
  enum {
    esc   = 0x01,
    ctrl  = 0x1D,
    num0  = 0x0B,
    latP  = 0x19
  };
}

//typedef int         IN; // used by windows for input type
typedef double        FP;
typedef unsigned int  UI;
typedef signed int    SI;
typedef char *        CS;
typedef char          CH;
typedef unsigned long UL;

const FP pi = 3.14159265358979323;
const UL amillisecond = calculate_busy_wait_millisecond();
const FP fptolerance = 0.0000000000001;

#include "A-VPt.h"
#include "B-decimal.h"
#include "C-curves.h"
#include "D-matchtag.h"
#include "E-malloctagsequence.h"
#include "F-iteratetagsequence.h"
#include "G-mousepoint.h"

// --- CONTEXT -----------------------------------------------------------------------------
// =========================================================================================
// ---  CODE   -----------------------------------------------------------------------------

const char pighead[] = ":E160,1.5[@90%d30s2l12B5*j2r12B5*"
                                 "@60%s1o15s2j1b5s3j1f5s4L3,2*L2,4*"
                                "@120%s1o15s2j1b5s3j1f5s4L3,2*L2,4*"
                                "@140%s1E10,1,0,2.25,135,2*j1"
                                "@270%u30E35,1.5[@0%r30E25*@180%l30E25*]]";

void clickmouse(InterceptionContext ctxt, InterceptionDevice mouse, SI x, SI y) {
  if (!x && !y) { return; } // ignore null movements / repeat clicks to save time
  InterceptionMouseStroke mstroke;
  mstroke.flags = INTERCEPTION_MOUSE_MOVE_RELATIVE;
  mstroke.x = x;
  mstroke.y = y;
  mstroke.state = 0; // 0 for move (no button action)
  interception_send(ctxt, mouse, (InterceptionStroke *)&mstroke, 1);
  busy_wait(amillisecond * 2);
  mstroke.x = 0;
  mstroke.y = 0;
  mstroke.state = INTERCEPTION_MOUSE_LEFT_BUTTON_DOWN;
  interception_send(ctxt, mouse, (InterceptionStroke *)&mstroke, 1);
  busy_wait(amillisecond * 2);
  mstroke.state = INTERCEPTION_MOUSE_LEFT_BUTTON_UP;
  interception_send(ctxt, mouse, (InterceptionStroke *)&mstroke, 1);
  busy_wait(amillisecond * 2);
}

void trackmouse(InterceptionContext ctxt, InterceptionDevice mouse, Pt start) {
  lower_process_priority();
  VPt actiondraw = newmousepoint(start, (CS)pighead); // start at 0 because relative ....
  Pt lastactionpt = actiondraw.point; // return point is start point
  Pt lastshifterror = Pt(0.0, 0.0); // assumes we start at an integer point (todo: start-error)
  if (!actiondraw.eref) {
    cout << "PROCESS FAILED TO START\n";
  } else {
    SI actiondrawcount = 0;
    while (1) {
      actiondraw = nextmousepoint(actiondraw);
      if (!actiondraw.eref) {
        // cout << "DRAW ENDED\n";
        break;
      } else if (++actiondrawcount > 1000000) {
        cout << "STOP AT A MILLI\n";
      } else if (actiondraw.eref->action == '!') {
        cout << "STOP BECAUSE ERROR\n";
      }
      Pt thisactionpt = actiondraw.point;
      Pt relativeshift = Pt(thisactionpt.x - lastactionpt.x, thisactionpt.y - lastactionpt.y);
      relativeshift.x += lastshifterror.x;
      relativeshift.y += lastshifterror.y;
      SI relativex = (SI)(relativeshift.x); // + 0.5);
      SI relativey = (SI)(relativeshift.y); // + 0.5);
      Pt shifterror = Pt(relativeshift.x - (FP)relativex, relativeshift.y - (FP)relativey);

      if (shifterror.x >  0.5) { shifterror.x -= 1.0; relativex++; }
      if (shifterror.y >  0.5) { shifterror.y -= 1.0; relativey++; }

      if (shifterror.x < -0.5) { shifterror.x += 1.0; relativex--; }
      if (shifterror.y < -0.5) { shifterror.y += 1.0; relativey--; }

      clickmouse(ctxt, mouse, relativex, relativey);
      lastactionpt = thisactionpt;
      lastshifterror = shifterror;
    }
  }
  if (actiondraw.eref) {
    cout << "process memory is only freed if the whole process is completed !!!!";
  } // else cout << "PROCESS COMPLETED\n";

  raise_process_priority();
}

int main() {
  cout << "<pighead>\n";

  DWORD windesktopwidth  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
  DWORD windesktopheight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
  cout << "  desktop: " << windesktopwidth << "x" << windesktopheight << "\n";
  DWORD winscreenwidth  = GetSystemMetrics(SM_CXSCREEN);
  DWORD winscreenheight = GetSystemMetrics(SM_CYSCREEN);
  cout << "  screen:  " << winscreenwidth  << "x" << winscreenheight  << "\n";

  InterceptionDevice device, mouse = 0;
  UI mouseid = 0;
  InterceptionStroke stroke;
  Pt position(0.0, 0.0); // ...all relative anyway

  raise_process_priority();
  InterceptionContext context = interception_create_context();

  cout << "  <intercept>\n";

  interception_set_filter(context, interception_is_keyboard,
    INTERCEPTION_FILTER_KEY_DOWN | INTERCEPTION_FILTER_KEY_UP);
  interception_set_filter(context, interception_is_mouse,
    INTERCEPTION_FILTER_MOUSE_MOVE);


  SI ctrldown = 0;
  while (interception_receive(context, device = interception_wait(context), &stroke, 1) > 0) {
    // end if any ESC received
    if (interception_is_keyboard(device)) {
      InterceptionKeyStroke &kstroke = *(InterceptionKeyStroke *)&stroke;
      if (kstroke.code == scancode::esc) {
        break; // context will be destroyed after loop, releasing mouse and keyboard
      }
    }
    if (interception_is_mouse(device)) {
      if (!mouse) {
        mouse = device;
        UI mouseid = device - INTERCEPTION_MOUSE(0);
        cout << "    { using mouse " << mouseid << " }\n";
      }
      InterceptionMouseStroke &mstroke = *(InterceptionMouseStroke *)&stroke;
      // todo: limit to bounds of paint window
      mstroke.flags = INTERCEPTION_MOUSE_MOVE_RELATIVE;
      interception_send(context, device, &stroke, 1);
    }
    if (mouse && interception_is_keyboard(device)) {
      InterceptionKeyStroke &kstroke = *(InterceptionKeyStroke *)&stroke;
      if (kstroke.state == INTERCEPTION_KEY_DOWN) {
        if (kstroke.code == scancode::ctrl) {
          ctrldown = 1;
          interception_send(context, device, &stroke, 1);
        } else if (ctrldown == 1 && kstroke.code == scancode::latP) { // ctrl+P down
          trackmouse(context, mouse, position);
          // no _send: do not bubble ctrl+P key
        } else { // default action is to send key as normal - todo: hotkey replacements
          interception_send(context, device, &stroke, 1);
        }
      } else if (kstroke.state == INTERCEPTION_KEY_UP) {
        if (kstroke.code == scancode::ctrl) {
          ctrldown = 0;
          interception_send(context, device, &stroke, 1);
        } else if (ctrldown == 1 && kstroke.code == scancode::latP) { // ctrl+P up
          // no up action as yet -- but only cancels P if P released before ctrl !
        } else if (kstroke.code == scancode::esc) { // end program loop on esc release
          break; // this should have been detected earlier on any-esc press anyway
        } else { // default action is to send key as normal - todo: hotkeyup replacements ?
          // cout << "keycode int: " << kstroke.code << "\n";
          interception_send(context, device, &stroke, 1);
        }
      } else {
        interception_send(context, device, &stroke, 1);
      }
    } // else if no mouse is found, no keyboard events send, press esc to exit
  }
  interception_destroy_context(context);

  cout << "  </intercept>\n";
  cout << "</pighead>\n";
  return 0;
}
