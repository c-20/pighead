This program uses interception.dll to access the Interception library, which it
presumes to be installed correctly.

Please note that installing the Interception library allows your keystrokes to be
intercepted, and this is obviously a security risk.

There may be other ways that keylogging can happen but permanent admin rights are
in this case granted so it's an open window but at least you can see it. 

With Interception installed and working, run pighead.exe.
This will enable interception of the first device from which the mouse moves.
Note that Interception sometimes doesn't detect movement from my laptop's
internal trackpad, but it's a bit flaky on the connection, so. Maybe a tofix.

Anyway, Win+R and run mspaint. Align the mouse in the centre of a canvas at least
500px by 500px in size (screen pixels, not image pixels), select a pencil-like
drawing tool and set the colour.

Once a drawing sequence starts, it cannot be cancelled. This will change later but
requires polling of intercept_receive. Maybe a task for multi-threading ?

To exit pighead.exe, press ESC. Regardless of device detection status, it should exit,
unless a drawing sequence is in progress.

Press Ctrl+P to start the pighead drawing sequence. Do not leave other important
windows open, as the mouse is clicking madly and does not check that it is
clicking onto an mspaint canvas! A future revision of pighead may detect canvas
boundaries, and adjust them, and check the before-and-after of each click and
cancel if the expected change is not seen. This also requires that the current
tool and colour be known, and thus, a future revision may also add new hotkeys.

The code to draw the pighead is:

    const char pighead[] = ":E160,1.5[@90%d30s2l12B5*j2r12B5*"
                                     "@60%s1o15s2j1b5s3j1f5s4L3,2*L2,4*"
                                    "@120%s1o15s2j1b5s3j1f5s4L3,2*L2,4*"
                                    "@140%s1E10,1,0,2.25,135,2*j1"
                                    "@270%u30E35,1.5[@0%r30E25*@180%l30E25*]]";

Example of program output can be found in pighead.png and pighead.log.

: is pixel mode, % is percent mode (relative to parent sequence).
A future revision will allow % mode in the top-level object by detecting
the resolution of the screen being drawn to.
Objects (curves) are drawn step-by-step, with trigger detection per increment.
Triggers start with @value and end after @ (next trigger) or ] (end of matchset).
If a trigger matches, a subsequence begins in the given mode, : or %.
Current ellipse increment is 2deg. Current line increment is 1px.
Ellipse triggers are @angle (from startangle to 360 * rotations).
Line triggers are @px (from 0 to max out of width and height).
Ellipse syntax is Ediameter,xratio,startangle,rotations,shiftangle,shiftspeed[].
Line syntax is Lpoint1,point2[]. * means the same as [].
Bead is a filled ellipse, that decrements size until 0. (tofix: startangle).
A subsequence starts at the same point as the triggering parent spotpoint.
If an ellipse has an origin shift, the first point will be drawn at the starting
point (the centre point is offset one radius). With no origin shift, the centre of
the ellipse will be at the parent spotpoint (edge points are offset one radius).
Points can be saved in index 1-9 with s (spotpoints[0] is the current point step).
Jump to a saved point with j. Shifting ellipses return to shifted origin, so s/j.
Later, memory actions will be added for other points (^2 for parent spotpoint[2]).
Offset actions are udlriobf (up down left right in out back forward), in % or :.

NOTE: If pointer precision is too low in Windows, the mouse driver will eat every
second small mouse movement! All movements are relative, so this can completely
ruin the attempted drawing. Obviously pointer precision being high can make mice
and trackpads harder to use, so hopefully a future revision will add a switch.

If you draw something interesting, please fork this project.
This code was created as a means of decoding action on paper.
Various instruction set extensions ought to be considered.
Note that the included dll and build libraries are the 32-bit variant.
Recompile pighead.exe by running ./pighead.gcc in MSYS2 (Mingw 32-bit).
