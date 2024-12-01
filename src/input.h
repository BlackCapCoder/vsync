#pragma once

#include <GLFW/glfw3.h>
#include "V2.h"
#include <iostream>



constexpr char jump_key = 'J';

// -----

#include <map>

using tick_t = size_t;

constexpr
tick_t tick_t_never = 0;

namespace global
{
  static tick_t ticks_elapsed = 0;
  static uint8_t ticks_to_skip = 0;
}

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
    /*std::cout << time << std::endl;*/
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


/*bool key_pressed (int key);*/

// if a key was pressed and released, but we didn't check
// `key_pressed` still report the key as being pressed.
//
bool key_pressed_ (int key)
{
  return global::keymap.get (key) . state;
}

template <class T>
T get_move (char l, char r)
{
  T move = 0;
  if (key_pressed_ (l)) move -= 1;
  if (key_pressed_ (r)) move += 1;
  return move;
}

template <class T>
V2 <T> get_movement (char n, char e, char s, char w)
{
  return V2 <T>
    { .x = get_move <T> (w, e)
    , .y = get_move <T> (n, s)
    };
}

