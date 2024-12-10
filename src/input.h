#pragma once

#include <GLFW/glfw3.h>
#include "V2.h"
#include <iostream>
#include <map>
#include <fmt/printf.h>

// -----

using tick_t = size_t;

constexpr tick_t
  tick_t_never = 0;

using secs = double;

namespace global
{
  static tick_t  ticks_elapsed = 0;
  static uint8_t ticks_to_skip = 0;

  static secs dt = 0;
  static secs time = 0;
  static secs time_frozen = 0;

  static secs time_elapsed ()
  {
    return time - time_frozen;
  }

  static secs tick_time ()
  {
    return time_elapsed () / ticks_elapsed;
  }
  static secs ticks_per_sec ()
  {
    return 1.0 / tick_time ();
  }

  const tick_t intended_ticks_per_sec = 144;
  const secs intended_tick_time = 1.0 / intended_ticks_per_sec;

  static secs tick_mult ()
  {
    return ticks_per_sec () / intended_ticks_per_sec;
  }
  static secs ticks_to_secs (tick_t t)
  {
    const secs s = tick_time ();
    const secs actual = 1.0 / s;
    return t * s * (actual / intended_ticks_per_sec);
  }


  static int is_frozen = false;

  static secs end_of_freeze_time = 0;
  static secs start_of_freeze_time = 0;

  // The actual freezing is performed in main
  static void freeze_time (tick_t duration)
  {
    is_frozen = 1;
    ticks_to_skip += duration;
  }
  static void freeze_time_seconds (double duration)
  {
    is_frozen = 2;
    start_of_freeze_time = time;
    end_of_freeze_time = time + duration;
  }


  static void tick_time (double _time)
  {
    dt = _time - time;
    time = _time;

    if (is_frozen == 0)
    {
      ticks_elapsed++;
    }
    else if (is_frozen == 1)
    {
      if (ticks_to_skip)
      {
        ticks_to_skip--;
        time_frozen += dt;
      }
      else
        is_frozen = false;
    }
    else if (is_frozen == 2)
    {
      if (time >= end_of_freeze_time)
      {
        time_frozen += end_of_freeze_time - start_of_freeze_time;
        is_frozen = false;
      }
    }
  }
}

template <tick_t framesT = 0>
struct Timer
{
  static constexpr tick_t frames = framesT;
  tick_t deadline;

  void start_exact (tick_t frames = framesT)
  {
    deadline = global::ticks_elapsed + frames;
  }
  void start (tick_t frames = framesT)
  {
    const tick_t t = std::ceil (((secs) frames) * global::tick_mult () - 0.25);
    // fmt::print("time given: {}\n", t);
    start_exact (t);
  }
  void stop ()
  {
    deadline = tick_t_never;
  }

  bool alive ()
  {
    return global::ticks_elapsed <= deadline;
  }
  bool dead ()
  {
    return global::ticks_elapsed > deadline;
  }
};

template <tick_t framesT = 0>
struct Sec_Timer
{
  static constexpr tick_t frames = framesT;
  secs deadline = -1;

  void start_secs (secs s)
  {
    deadline = global::time_elapsed () + s;
  }
  void start_ms (secs ms)
  {
    start_secs (ms / 1000);
  }
  void start (tick_t frames = framesT)
  {
    start_secs (global::intended_tick_time * frames);
  }
  void stop ()
  {
    deadline = -1;
  }

  bool alive ()
  {
    return deadline > 0 && global::time_elapsed() <= deadline;
  }
  bool dead ()
  {
    return deadline < 0 || global::time_elapsed() > deadline;
  }
};

// ----

struct KeyMapEntry
{
  bool   state { false        };
  tick_t time  { tick_t_never };

  static constexpr
  KeyMapEntry nil ()
  {
    return KeyMapEntry { .state = false, .time = tick_t_never };
  }
};

struct KeyMap
{
  std::map<int, KeyMapEntry> m {};

  void put (tick_t time, int key, int scancode, int action, int mods)
  {
    if (mods != 0) return;
    if (action != GLFW_PRESS && action != GLFW_RELEASE) return;

    KeyMapEntry newE
      { .state = action == GLFW_PRESS
      , .time  = time
      };

    if (auto search = m.find (key); search == m.end ())
      m.insert_or_assign (key, newE);
    else
    if (auto & oldE = search->second; oldE.state ^ newE.state)
      oldE = newE;
    else
      return;
  }

  KeyMapEntry & get (int key)
  {
    return m [key];
  }
};

namespace global
{
  static KeyMap keymap {};
}

// ----

struct Key
{
  const int val;
  template <class T> constexpr Key (T a) : val {a} {};

  const int operator () () const { return val; }

  KeyMapEntry & entry () const
  {
    return global::keymap.get (val);
  }

  bool pressed () const
  {
    return entry () . state;
  }

  tick_t time () const
  {
    return entry () . time;
  }

  bool fresh (int expiration = 8) const
  {
    expiration = std::ceil (((secs) expiration) * global::tick_mult () - 0.25);

    const auto & e = entry ();
    return e.state && global::ticks_elapsed - e.time <= expiration;
  }
};

namespace input
{
  static constexpr Key
    /*
    */ move_N    = 'E'
     , move_E    = 'F'
     , move_S    = 'D'
     , move_W    = 'S'
     , jump      = 'J'
     , dash      = 'K'
     , dash_down = 'L'
     , climb     = 'A'
     , debug_key = ' '
     ;

  static int move_x ()
  {
    return move_E.pressed () - move_W.pressed ();
  }
  static int move_y ()
  {
    return move_S.pressed () - move_N.pressed ();
  }
};

