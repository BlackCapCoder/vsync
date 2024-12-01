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
  static constexpr V2 <float> size = { 7.9/8.0, 11.0/8.0 };
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

  static constexpr tick_t dash_duration = 120 * .15f;
  static constexpr tick_t dash_cooldown = 120 * .2f; // time before we can dash again
  static constexpr tick_t dash_refresh_time = 120 * .1f; // time before we can get our dash back
  tick_t dash_timeout;
  tick_t dash_cooldown_timeout;
  tick_t dash_refresh_timeout;

  static constexpr float dash_speed = 240.f / 8.f;
  static constexpr float dash_up_mult = .75f;
  static constexpr float end_dash_speed = 160.f / 8.f;

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
      dash_vel.x *= dash_up_mult;
      dash_vel.y *= dash_up_mult;
    }

    // dashing in the direction you're already going should never slow you down
    if (signum(dash_vel.x) == signum(vel.x) && abs(vel.x) > abs(dash_vel.x)) dash_vel.x = vel.x;
    if (signum(dash_vel.y) == signum(vel.y) && abs(vel.y) > abs(dash_vel.y)) dash_vel.y = vel.y;

    vel.x = dash_vel.x;
    vel.y = dash_vel.y;
  }
  void on_dashing ()
  {
    // we could do this once in `begin_dash`, but doing it every frame
    // allows wrapping around corners.
    // {
    //   vel.x = dash_vel.x;
    //   vel.y = dash_vel.y;
    // }

    if (global::ticks_elapsed > dash_timeout)
      end_dash ();
  }
  void end_dash ()
  {
    dash_state = NotDashing;
    if (dash_direction.y < 1)
    {
      vel.x = dash_direction.x * end_dash_speed;
      vel.y = dash_direction.y * end_dash_speed;
    }
    if (vel.y < 0)
      vel.y *= dash_up_mult;
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

  // when hitting something, wait a few frames before killing the speed.
  // if we have both x and y velocity we can sort of slide around corners
  // and keep our speed
  static constexpr int slide_grace = 8;
  int x_slide = 0;
  int y_slide = 0;

  // if we have only x or y speed, try jiggling slightly on the other axis
  static constexpr float corner_correction_slow = 0.2f;
  static constexpr float corner_correction_fast = 0.45f;
  static constexpr float corner_correction_fast_vel = 22.f;

  void apply_velocity ()
  {
    if (vel.x == 0 && vel.y == 0)
      return;

    const float scale = 0.01;
    const V2 <float> new_pos
      { pos.x + vel.x * scale
      , pos.y + vel.y * scale
      };

    // ---- no collision, or no collision after corner correction

    {
      bool ok = true;

      if (! hit_test (new_pos, size))
        pos = new_pos;
      else if (vel.y == 0)
      {
        const float c
          = std::abs(vel.x) < corner_correction_fast_vel
          ? corner_correction_slow
          : corner_correction_fast
          ;

        if (const V2 <float> p {new_pos.x, new_pos.y - c}; !hit_test (p, size))
          { pos = p; moveY(+c); } else
        if (const V2 <float> p {new_pos.x, new_pos.y + c}; !hit_test (p, size))
          { pos = p; moveY(-c); }
        else
        {
          moveX (vel.x * scale);
          vel.x = 0;
        }
      }
      else if (vel.x == 0)
      {
        const float c
          = std::abs(vel.y) < corner_correction_fast_vel
         || vel.y > 0 // don't want to slip into the pit!
          ? corner_correction_slow
          : corner_correction_fast
          ;

        if (const V2 <float> p {new_pos.x - c, new_pos.y}; !hit_test (p, size))
          { pos = p; moveX(+c); } else
        if (const V2 <float> p {new_pos.x + c, new_pos.y}; !hit_test (p, size))
          { pos = p; moveX(-c); }
        else
        {
          moveY (vel.y * scale);
          vel.y = 0;
        }
      }
      else ok = false;

      if (ok) return;
    }

    // ---- collision and velocity in both directions, crap!

    bool x_moved, x_more, y_moved, y_more;

    // jiggle towards the target as much as possible,
    // we'll get stuck on at least one axis
    {
      const auto old_pos = pos;
      moveXY (vel.x * scale, vel.y * scale);

      x_moved = pos.x != old_pos.x;
      x_more  = pos.x != new_pos.x;

      y_moved = pos.y != old_pos.y;
      y_more  = pos.y != new_pos.y;

      if ( x_moved
        && x_more
        && ! hit_test ({new_pos.x, pos.y}, size)
         ) { pos.x = new_pos.x; x_more = false; }
      else if
         ( y_moved
        && y_more
        && ! hit_test ({pos.x, new_pos.y}, size)
         ) { pos.y = new_pos.y; y_more = false; }
    }

    // if only one axis did not move at all, wait a few frames before
    // killing its velocity- the axis that does move might get us unstuck!
    //
    // TODO: refund lost mileage upon getting unstuck?
    //
    {
      if (x_moved)
        x_slide = 0;
      else if (y_moved && x_slide < slide_grace)
        x_slide++;
      else
      {
        /*if (y_moved) std::cout << "killed vel.x\n";*/
        vel.x = 0;
        x_slide = 0;
      }

      if (y_moved)
        y_slide = 0;
      else if (x_moved && y_slide < slide_grace)
        y_slide++;
      else
      {
        /*if (x_moved) std::cout << "killed vel.y\n";*/
        vel.y = 0;
        y_slide = 0;
      }
    }

    // not required and probably doesn't belong here,
    // but if we're going to jump immediately at the beginning of the next frame,
    // we might as well do it now so the user doesn't have to wait!
    if ( y_moved
      && y_more
      && vel.y > 0
      && ! is_grounded
       )
    {
      is_grounded = true;
      on_grounded ();
      if (do_jumping ())
      {
        apply_velocity ();
        std::cout << "early jump" << std::endl;
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

  static constexpr float walk_max = 90.f / 8;
  static constexpr float walk_accel = 1;
  static constexpr float walk_reduce = 400.f / (8 * 120);
  static constexpr float walk_stopping = 0.03;
  static constexpr float fric_ground = 1.0;
  static constexpr float fric_air = 0.65;

  float get_friction ()
  {
    return is_grounded ? fric_ground : fric_air;
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
      const auto vx2  = vx - v_sign * fric * walk_reduce;
      vx = vx2*v_sign < walk_max ? walk_max*v_sign : vx2;
    }

    vel.x = vx;
  }

  // ----

  static constexpr float gravity_max = 30;
  static constexpr float gravity_accel = 1.0;

  // We want `jump_liftoff` to be small so that if we quickly tap jump
  // we barely move off the ground, but we also want to be able to jump high!
  // Delay gravity for a few frames while jump is held.
  static constexpr int zero_grav_time = 15;

  float get_gravity ()
  {
    float g = gravity_accel;
    if (key_pressed_ (jump_key)) // half gravity while holding jump
    {
      if (global::ticks_elapsed - time_last_jump <= zero_grav_time)
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
    cayotee_time =
    // input_buffer_frames
    5
    ;

  tick_t time_last_jump = 0;

  static constexpr float
    /*jump_liftoff = 15;*/
    jump_liftoff = 13;

  static constexpr float
    super_speed = 260.f / 8;
  static constexpr float
    hyper_x_mult = 1.25;
  static constexpr float
    hyper_y_mult = 0.5;
  static constexpr float
    ultra_boost = 1.2;


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

      //vel.x = dash_vel.x;
      vel.x = super_speed * dash_direction.x;

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
        vel.x *= hyper_x_mult;
        vel.y *= hyper_y_mult;
        std::cout << "hyper" << std::endl;
      }
      else
      {
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


