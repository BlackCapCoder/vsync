//
// I wrote this code, but it's a while ago!
//
// source: 
//   https://github.com/glfw/glfw/issues/1157#issuecomment-1868690156

#include <GL/gl.h>
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>


struct Renderer
{
  Renderer
    ( Display *  dpy
    , Window     win
    , Drawable   xdr
    ) : dpy{dpy}, win{win}, xdr{xdr}
  {
    initSyncVars ();
  }

  // These are initialized by `initSyncVars`:

  // The time between screen refreshes, in microseconds,
  // is given by `mscA + mscB / mscC`, where `mscB / mscC`
  // is some rational number less than 1
  int32_t mscA;
  int32_t mscB;
  int32_t mscC;

  // Before we begin drawing, we wait for a screen refresh and
  // take take note of ..

  int64_t ust_init; // current time in microseconds
  int64_t msc_init; // total number of screen refreshes (globally)


  // -------- Just for convenience:
  
  int64_t ust, msc, sbc;

  void updateTime ()
  {
    glXGetSyncValuesOML (dpy, win, &ust, &msc, &sbc);
  }

  int waitForMscOML (int target_msc, int divisor, int remainder)
  {
    return glXWaitForMscOML (dpy, win, target_msc, divisor, remainder, &ust, &msc, &sbc);
  }

  // ----------


  void initSyncVars ()
  {
    int32_t a0, b0;
    glXGetMscRateOML (dpy, xdr, &a0, &b0);

    // double rate_hz = (double) a0 / b0;  //   60.0204  monitor refresh rate
    // double rate_ms = 1000.0 / rate_hz;  //   16.6610  ms between refreshes
    // double rate_us = 1000.0 * rate_ms;  //  ~16661    microseconds
    //
    // rate_us = (1000000.0 * (double) b0) / a0

    int32_t f  = gcd (1000000, a0); // to keep the numbers nice and small
    int32_t a1 = a0 / f;
    int32_t b1 = b0 * (1000000 / f);

    mscA = b1 / a1; // a2
    mscB = b1 % a1; // b2
    mscC = a1;      // c

    // b0/a0*1000000 = a2 + b2/c
    // a1/b1*1000000 = a0/b0

    // ---

    updateTime ();
    waitForMscOML (0, 0, 0);
    waitForMscOML (msc+1, 0, 0);

    msc_init = msc;
    ust_init = ust;
  }

  // ----

  constexpr int64_t calcRefreshTime (int64_t msc, double err)
  {
    return
      msc * mscA + std::floor (
        msc * (err + ((double) mscB / mscC))
      );
  }

  // Note the `err` parameter.
  //
  // The `ust` (current time) value produced by waitForMscOML
  // is very close to the time of the redraw, but it is
  // either not exact, amd/or the error introduced by floor
  // adds up over time. `err` is a workaround.


  // Error correction for refresh guesses
  double guessErrF = 0;

  // Call this once you know the `ust` that the `msc`th
  // refreh occed at.
  void reportRefreshTime (int64_t ust, int64_t msc)
  {
    int64_t delta_ust = ust - ust_init;
    int64_t delta_msc = msc - msc_init;

    // What we would have guessed with no error correction
    int64_t guess =
      calcRefreshTime (delta_msc, 0);

    // +-microseconds that guess would have been off
    int64_t errI =
      delta_ust - guess;

    // the error correction value that would have 
    // made the guess correct
    double errF =
      (double) errI / delta_msc;


    // Either one of these is fine; I don't know if one is better:

    // guessErrF = errF;
    guessErrF = (guessErrF + errF)/2;

    // If `waitForMscOML` where to report a time that is "way off"
    // the former would make our next guess also way off.
    // The later is intended to alleviate this.
  }


  int64_t guessTimeOfNextRefresh ()
  {
    int64_t delta_msc = msc - msc_init;

    return ust_init +
      calcRefreshTime (delta_msc + 1, guessErrF);
  }

  // On my system, ^ this function seems to be about
  // in the range of +-10us of the actual refresh.
  // +-3us is pretty typical, but it can make the
  // occasional bad guess.

  // This is more than enough for me though, and certanly
  // better than SwapBuffer which would block for MILLISECONDS

  // ----

  // Here's my rendering loop:

  // I call this in the main thread- less sdl would misbehave-
  // and have my event processing loop (using X, which is thread safe)
  // in a separate thread.


  // all rendering needs to happen on this thread,
  // so I have a queue of things to be rendered
  TQueue <Task> work {};

  // ..and some arbitrarily chosen deadline before the
  // refresh, after which I will no longer be accepting
  // new tasks. Overshooting the refresh would be terrible,
  // so I'd rather be safe than sorry.
  int renderDeadline = 50;


  void renderLoop ()
  {
    while (!shouldQuit)
    {
      if (work.empty ())
        work.snooze (); // block until we have something to draw
      work.pop () (); // draw it- now we're on a timer

      updateTime ();
      const int64_t deadline = 
        guessTimeOfNextRefresh () - renderDeadline;

      // keep drawing stuff until the deadline
      while (ust > deadline)
      {
        if (work.empty ())
        {
          bool ok = work.snooze (deadline - ust); // block with timeout
          if (!ok) break; // out of time!
        }
        work.pop () ();
        updateTime ();
      }

      glFlush ();
      waitForMscOML (msc+1, 0, 0);
      // glXSwapBuffers (dpy, xdr);

      reportRefreshTime (ust, msc);
    }
  }
};
