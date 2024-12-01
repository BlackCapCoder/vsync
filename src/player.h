#pragma once

#include "input.h"
#include "V2.h"
#include <math.h>
#include <iostream>

// ----

template <class T>
T signum (T a)
{
  if (a < 0) return -1;
  if (a > 0) return  1;
  return 0;
}

bool hit_test (V2 <float> pos);
bool hit_test (V2 <float> pos, V2 <float> size);

// ----

struct Player
{
  V2 <float> pos;
  V2 <float> vel{};
  static constexpr V2 <float> size = { 0.7, 1.0 };
  int facing = 1;
  int n_dashes = 1;

  enum DashState
  { NotDashing
  , DirectionPending
  , Dashing
  };

  DashState dash_state = NotDashing;

  // -----

  Player (V2 <float> pos) : pos {pos}
  {
    /*std::cout << fractionf (1.2) << std::endl;*/
  }

  void tick ()
  {
    do_grounding ();

    if (dash_state == NotDashing)
      do_dash ();
    else if (dash_state == DirectionPending)
      on_dash_dir_pending ();

    do_jumping ();

    if (dash_state == Dashing)
      on_dashing ();

    if (dash_state == NotDashing)
    {
      do_walking ();
      do_gravity ();
    }

    // just for debugging
    if (key_pressed_('A'))
      vel.y = -10;

    if (dash_state != DirectionPending)
      apply_velocity ();
  }

private:

  void normal_update ()
  {
    do_walking ();
    do_gravity ();
  }

  // ----

  static constexpr tick_t input_buffer_frames =
    8;
    /*10;*/
    /*50;*/

  // like key_pressed, but the key will time-out after a few frames.
  // Allows early inputs, but the key must still be pressed when we check!
  //
  bool has_key_buffered (int key)
  {
    const auto entry = global::keymap.get (key);
    if (! entry.state) return false;
    return global::ticks_elapsed - entry.time <= input_buffer_frames;
  }

  // ----

  // Before starting a dash we freeze the game for a few frames
  // to give the player time to choose a dash-direction
  static constexpr tick_t freeze_time = 5;
  V2 <int> dash_direction;

  static constexpr tick_t dash_duration = 25;
  static constexpr tick_t dash_cooldown = 10; // time before we can dash again
  static constexpr tick_t dash_refresh_time = 10; // time before we can get our dash back
  tick_t dash_timeout;
  tick_t dash_cooldown_timeout;
  tick_t dash_refresh_timeout;

  static constexpr float dash_speed = 30;
  static constexpr float dash_y_mult = 0.7f;

  V2 <float> dash_vel;

  void do_dash ()
  {
    if ( n_dashes < 1
      || !( has_key_buffered ('K')
         || has_key_buffered ('L')
          )
       ) return;
    if (global::ticks_elapsed < dash_cooldown_timeout)
      return;

    n_dashes--;
    dash_cooldown_timeout = global::ticks_elapsed + dash_cooldown;
    dash_refresh_timeout  = global::ticks_elapsed + dash_refresh_time;

    dash_state = DirectionPending;
    dash_direction.x = 0;
    dash_direction.y = 0;
    global::ticks_to_skip = freeze_time;
  }
  void on_dash_dir_pending ()
  {
    const float mx = get_move <int> ('S', 'F');
    const float my = get_move <int> ('E', 'D');

    if (mx != 0 && mx != dash_direction.x)
      dash_direction.x = mx;
    if (my != 0 && my != dash_direction.y)
      dash_direction.y = my;
    if (key_pressed_('L'))
      dash_direction.y = 1;

    if
      /*(global::ticks_elapsed >= freeze_timeout)*/
      (true)
      begin_dash ();
  }
  void begin_dash ()
  {
    dash_state = Dashing;
    dash_timeout = global::ticks_elapsed + dash_duration;

    if (dash_direction.x == 0 && dash_direction.y == 0)
      dash_direction.x = facing;

    if (dash_direction.x != 0)
      facing = dash_direction.x;

    dash_vel.x = dash_direction.x * dash_speed;
    dash_vel.y = dash_direction.y * dash_speed;

    if (dash_direction.y < 0)
    {
      dash_vel.x *= dash_y_mult;
      dash_vel.y *= dash_y_mult;
    }

    // dashing in the direction you're already going should never slow you down
    if (signum(dash_vel.x) == signum(vel.x) && abs(vel.x) > abs(dash_vel.x)) dash_vel.x = vel.x;
    if (signum(dash_vel.y) == signum(vel.y) && abs(vel.y) > abs(dash_vel.y)) dash_vel.y = vel.y;
  }
  void on_dashing ()
  {
    // we could do this once in `begin_dash`, but doing it every frame
    // allows wrapping around corners.
    {
      vel.x = dash_vel.x;
      vel.y = dash_vel.y;
    }

    if (global::ticks_elapsed >= dash_timeout)
      end_dash ();
  }
  void end_dash ()
  {
    dash_state = NotDashing;
    if (dash_direction.y < 1)
    {
      vel.x = 0;
      vel.y = 0;
    }
  }

  // ----

  static constexpr float epsilon = 0.005;

  int moveX (float dx)
  {
    if (std::abs(dx) < epsilon) return 0;
    const float half = dx * 0.5;
    const V2<float> new_pos {pos.x + half, pos.y};

    if (! hit_test (new_pos, size))
    {
      pos = new_pos;
    }

    return 1 + moveX (half);
  }
  int moveY (float dy)
  {
    if (std::abs(dy) < epsilon) return 0;
    const float half = dy * 0.5;
    const V2<float> new_pos {pos.x, pos.y + half};

    if (! hit_test (new_pos, size))
    {
      pos = new_pos;
    }

    return 1 + moveY (half);
  }
  int moveXY (float dx, float dy)
  {
    constexpr float step = epsilon;
    const float sx = dx > 0 ? 1 : -1;
    const float sy = dy > 0 ? 1 : -1;

    if (dx*sx < step)
    {
      return moveY (dy);
    }
    if (dy*sy < step)
    {
      return moveX (dx);
    }

    bool stuck = true;

    if (! hit_test ({pos.x+step*sx, pos.y}, size))
    {
      pos.x += step*sx;
      dx -= step*sx;
      stuck = false;
    }
    if (! hit_test ({pos.x, pos.y+step*sy}, size))
    {
      pos.y += step*sy;
      dy -= step*sy;
      stuck = false;
    }

    if (!stuck)
    {
      return 1 + moveXY (dx, dy);
    }
    return 1;
  }

  float moveStep (float dx, float dy)
  {
    if (dx == 0) return foo (pos.y, size.y, dy);
    if (dy == 0) return foo (pos.x, size.x, dx);

    return std::min
      ( foo (pos.x, size.x, dx)
      , foo (pos.y, size.y, dy)
      );
  }
  float foo (float a, float c, float b)
  {
    float q0;
    if (b > 0)
      q0 = (a+c) - std::floor(a+c);
    else
    {
      q0 = a - std::floor (a);
      q0 = q0 == 0 ? -1 : -q0;
    }
    return q0 / b;
  }

  // TODO: naively kills all velocity
  void apply_velocity ()
  {
    if (vel.x == 0 && vel.y == 0)
      return;

    const float scale = 0.01;
    const V2 <float> new_pos
      { pos.x + vel.x * scale
      , pos.y + vel.y * scale
      };

    if (! hit_test (new_pos, size))
    {
      pos = new_pos;
      return;
    }

    /*float step = moveStep (vel.x, vel.y);*/
    /*V2 <float> res { pos.x + vel.x*step, pos.y + vel.y*step };*/

    const auto old_pos = pos;
    const int itts = moveXY (vel.x * scale, vel.y * scale);


    if ( old_pos.x != pos.x &&
         new_pos.x != pos.x &&
       ! hit_test ({new_pos.x, pos.y}, size)
    ) pos.x = new_pos.x;
    else if
      ( old_pos.y != pos.y &&
        new_pos.y != pos.y &&
      ! hit_test ({pos.x, new_pos.y}, size)
    ) pos.y = new_pos.y;

    if (old_pos.x == pos.x)
      vel.x = 0;

    if (old_pos.y == pos.y)
      vel.y = 0;
    else if (vel.y > 0 && pos.y != new_pos.y)
    {
      if (! is_grounded)
      {
        is_grounded = true;
        on_grounded ();
        if (do_jumping ())
        {
          apply_velocity ();
          std::cout << "bonus jump" << std::endl;
        }
      }
    }
  }

  // ----

  bool is_grounded = false;
  tick_t time_ungrounded = tick_t_never;

  bool check_is_grounded ()
  {
    const float feet_box_h = 0.20;
    const V2 <float> fpos  { pos.x, pos.y + size.y };
    const V2 <float> fsize { size.x, feet_box_h };
    return hit_test(fpos, fsize);
  }

  void do_grounding ()
  {
    bool gnew = check_is_grounded ();
    if (gnew)
      on_grounded ();
    if (is_grounded == gnew) return;
    is_grounded = gnew;

    if (gnew)
    {
      // we do this more accurately in apply_velocity
      // vel.y = 0;
    }
    else
    {
      time_ungrounded = global::ticks_elapsed;
    }
  }

  void on_grounded ()
  {
    if (global::ticks_elapsed >= dash_refresh_timeout)
      n_dashes = 1;
  }

  // ----

  static constexpr float walk_max = 10;
  static constexpr float walk_accel = 1;
  static constexpr float walk_stopping = 0.03;

  float get_friction ()
  {
    return is_grounded ? 1.0 : 0.3;
  }
  /*const auto move = get_movement <float> ('E','F','D','S');*/

  void do_walking ()
  {
    const float mx = get_move <float> ('S', 'F');
    float vx = vel.x;

    const auto m_sign = signum (mx);
    const auto v_sign = signum (vx);

    if (mx != 0)
      facing = mx;

    if (mx == 0)
    {
      const auto fric = get_friction ();
      const auto vx2  = vx - v_sign * fric;
      vx = signum (vx2) == v_sign ? vx2 : 0;
      vx *= 1 - walk_stopping;
    }
    else if (v_sign != m_sign)
    {
      vx = m_sign * walk_accel;
    }
    else if (vx*v_sign < walk_max)
    {
      const auto vx2 = vx + walk_accel*v_sign;
      vx = vx2*v_sign > walk_max ? walk_max*v_sign : vx2;
    }
    else
    {
      const auto fric = get_friction ();
      const auto vx2  = vx - v_sign * fric;
      vx = vx2*v_sign < walk_max ? walk_max*v_sign : vx2;
    }

    vel.x = vx;
  }

  // ----

  static constexpr float gravity_max = 30;
  static constexpr float gravity_accel = 1.0;

  float get_gravity ()
  {
    float g = gravity_accel;
    if (key_pressed_(jump_key)) // half gravity while holding jump
    {
      if (global::ticks_elapsed - time_last_jump <= 10)
        g = 0;
      else
      g *= 0.5;
    }
    return g;
  }

  void do_gravity ()
  {
    if (is_grounded) return;
    if (vel.y >= gravity_max) // gravity should not slow you down
      return;
    const float g = get_gravity ();
    vel.y = std::min<float>(gravity_max, vel.y + g);
  }


  // grace period where you can still jump after having left the ground
  static constexpr tick_t
    cayotee_time = input_buffer_frames;

  tick_t time_last_jump = 0;

  static constexpr float
    jump_liftoff = 13;

  static constexpr float
    super_boost = 1.2;
  static constexpr float
    hyper_boost = 1.4;
  static constexpr float
    ultra_boost = 1.2;

  static constexpr float
    hyper_y_mult = 0.5;

  bool do_jumping ()
  {
    if (! has_key_buffered (jump_key)) return false;
    const auto jt = global::keymap.get (jump_key).time;
    if (jt <= time_last_jump)
      return false;
    if (! is_grounded)
    {
      if (time_ungrounded + cayotee_time < global::ticks_elapsed)
        return false;
      if (time_ungrounded <= time_last_jump) // you only get 1 jump!
        return false;

      std::cout << "cayotee jump" << std::endl;
    }
    else
    {
      is_grounded = false;
      time_ungrounded = global::ticks_elapsed;
    }

    time_last_jump = global::ticks_elapsed;
    vel.y = -jump_liftoff;

    if (dash_state == Dashing)
    {
      dash_state = NotDashing;

      if (dash_direction.x == 0)
        return true;

      vel.x = dash_vel.x;

      // allows reversing supers and hypers
      {
        const int mx = get_move <int> ('S', 'F');
        if (mx != 0 && mx != dash_direction.x)
        {
          vel.x *= -1;
          std::cout << "reverse ";
        }
      }

      if (dash_direction.y > 0)
      {
        vel.x *= hyper_boost;
        vel.y *= hyper_y_mult;
        std::cout << "hyper" << std::endl;
      }
      else
      {
        vel.x *= super_boost;
        std::cout << "super" << std::endl;
      }
    }
    else
    {
      vel.x *= ultra_boost;
    }

    return true;
  }

};


