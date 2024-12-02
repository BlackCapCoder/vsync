#pragma once

#include <GLFW/glfw3.h>
#include "V2.h"
#include <iostream>
#include <map>

// -----

using tick_t = size_t;

constexpr tick_t
  tick_t_never = 0;

namespace global
{
  static tick_t  ticks_elapsed = 0;
  static uint8_t ticks_to_skip = 0;

  // The actual freezing is performed in main
  static void freeze_time (tick_t duration)
  {
    ticks_to_skip += duration;
  }
}

template <tick_t framesT = 0>
struct Timer
{
  static constexpr tick_t frames = framesT;
  tick_t deadline;

  void start (tick_t frames = framesT)
  {
    deadline = global::ticks_elapsed + frames;
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

