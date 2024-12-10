#pragma once

#include "input.h"
#include "V2.h"
#include <math.h>
#include <iostream>
#include <fmt/core.h>

// ----

template <class T>
T signum (T a)
{
  if (a < 0) return -1;
  if (a > 0) return  1;
  return 0;
}

bool hit_test (V2 <float> pos, V2 <float> size);

// ----

struct Player
{
  V2 <float> pos;
  V2 <float> vel {};
  int facing   = 1; // which direction the player facing. If we had a sprite we'd flip it when this is -1
  int n_dashes = 1; // number of times the player can dash without getting a refill

  // size of the players hitbox
  static constexpr V2 <float> size = { 7.9/8.0, 11.0/8.0 };

  enum DashState
  { NotDashing
  , DirectionPending
  , Dashing
  }
  dash_state = NotDashing;

  // -----

  Player (V2 <float> pos) : pos {pos}
  {
  }

  void tick ()
  {
    do_grounding ();

    if (dash_state == NotDashing)
      try_dash ();
    else if (dash_state == DirectionPending)
      on_dash_dir_pending ();

    do_jumping ();

    if (dash_state == Dashing)
      on_dashing ();

    if (dash_state == NotDashing)
    {
      if (is_climbing)
        climb_update ();
      else if (! try_climb ())
      {
        do_movement ();
        do_gravity ();
      }
    }

    // just for debugging
    if (input::debug_key.pressed ())
      vel.y = -10;

    if (dash_state != DirectionPending)
      apply_velocity ();
  }

private:
  // Before starting a dash we freeze the game for a few frames
  // to give the player time to input a direction to dash in.
  //
  // If we didn't do this you'd have to be careful to press
  // all the buttons at the same time; diagonal dashes become
  // inconsistent!
  //
  static constexpr tick_t freeze_frames = 5;
  V2 <int> dash_direction;

  Timer <(tick_t) (120 * .15f)> dash_timer;
  Timer <(tick_t) (120 * .2f )> dash_cooldown_timer;
  Timer <(tick_t) (120 * .1f )> dash_refresh_timer;
  Timer <(tick_t) (120 * .3f )> dash_bounce_timer;

  static constexpr float dash_speed     = 240.f / 8.f;
  static constexpr float end_dash_speed = 160.f / 8.f;
  static constexpr float dash_up_mult   = .75f;

  V2 <float> dash_vel;
  tick_t time_dash_started {}; // only used for debug output


  bool try_dash ()
  {
    if ( dash_state != NotDashing
      || n_dashes < 1
      || dash_cooldown_timer.alive ()
      || !( input::dash.fresh ()
         || input::dash_down.fresh ()
          )
       )
      return false;

    n_dashes--;
    time_dash_started = global::ticks_elapsed;
    dash_cooldown_timer.start ();
    dash_refresh_timer.start ();

    dash_state = DirectionPending;
    dash_direction.x = 0;
    dash_direction.y = 0;

    // global::freeze_time (freeze_frames);
    // global::freeze_time_seconds (global::intended_tick_time * freeze_frames);
    global::freeze_time (global::scaled_ticks (freeze_frames));

    return true;
  }
  void on_dash_dir_pending ()
  {
    const float mx = input::move_x ();
    const float my = input::move_y ();

    if (mx != 0 && mx != dash_direction.x)
      dash_direction.x = mx;
    if (my != 0 && my != dash_direction.y)
      dash_direction.y = my;
    if (input::dash_down.pressed ())
      dash_direction.y = 1;

    if
      /*(global::ticks_elapsed >= freeze_timeout)*/
      (true)
      begin_dash ();
  }

  bool pending_ultra = false;

  void begin_dash ()
  {
    dash_state = Dashing;
    dash_timer.start ();
    dash_bounce_timer.start ();

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

    pending_ultra =
      dash_direction.y > 0 && dash_direction.x != 0;
  }
  void on_dashing ()
  {
    if (dash_timer.dead ()) end_dash ();
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

  // when hitting something, wait a few frames before killing the speed.
  // if we have both x and y velocity we can sort of slide around corners
  // and keep our speed
  static constexpr int slide_grace = 8;
  int x_slide = 0;
  int y_slide = 0;

  // if we have only x or y speed, try jiggling slightly on the other axis
  static constexpr float corner_correction_slow     = 0.2f;
  static constexpr float corner_correction_fast     = 0.45f;
  static constexpr float corner_correction_fast_vel = 22.f;

  void apply_velocity ()
  {
    if (vel.x == 0 && vel.y == 0)
      return;

    // const float scale = 0.01;
    const float scale = 0.01 * global::intended_ticks_per_sec * global::dt;
    // const float scale = global::dt / (5.f / 6.f);
    //const float scale = global::dt / (2.f / 3.f);
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
          { pos = p; moveY(+c); if (!is_grounded) early_grounding (); } else
        if (const V2 <float> p {new_pos.x, new_pos.y + c}; !hit_test (p, size))
          { pos = p; moveY(-c); }
        else
        {
          moveX (vel.x * scale);
          if (! try_corner_boost ())
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
          const float old_vy = vel.y; vel.y = 0;

          if (old_vy > 0 && !is_grounded)
            early_grounding ();
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
      const int grace = std::round (((secs) slide_grace) * global::tick_mult ());

      if (x_moved)
        x_slide = 0;
      else if (y_moved && x_slide < grace)
        x_slide++;
      else
      {
        /*if (y_moved) std::cout << "killed vel.x\n";*/
        vel.x = 0;
        x_slide = 0;
      }

      if (y_moved)
        y_slide = 0;
      else if (x_moved && y_slide < grace)
        y_slide++;
      else
      {
        /*if (x_moved) std::cout << "killed vel.y\n";*/
        vel.y = 0;
        y_slide = 0;
      }
    }

    if ( y_moved
      && y_more
      && vel.y > 0
      && ! is_grounded
       )
    {
      early_grounding ();
    }
  }

  // This is not required, but if we become grounded after `apply_velocity`,
  // we attempt to dash/jump immediately, instead of at the beginning of
  // the next frame.
  //
  // The user is already waiting for their inputs to be reflected on the screen,
  // so why draw a useless frame they don't care about?
  //
  // If the relevant input is buffered and the buffer would have expired on the
  // next frame, this function trigger behaviour that wouldn't have been triggered
  // on the next frame, however, it is probably what the user intended.
  //
  void early_grounding ()
  {
    is_grounded = true;
    time_grounded = global::ticks_elapsed;
    on_grounded ();

    if (try_dash ())
    {
      std::cout << "early dash" << std::endl;

      // Crucially we do NOT attempt to jump here because we are
      // waiting for a game freeze, which is handeled downstream.
      // 
      // It could make sense to do an immediate super/hyper here when
      // we already have movement input- skipping the freeze entirely,
      // however this function is not called consistently!
      //
      // It would put us in a situation where we have a desirable
      // behaviour that saves a significant number of frames, but
      // it cannot be triggered consistently, even on identical inputs.
    }
    else if (do_jumping ())
    {
      apply_velocity ();
      std::cout << "early jump" << std::endl;
    }
  }

  bool try_corner_boost ()
  {
    return false; // TODO:

    if (! input::jump.fresh () )
      return false;
    if (! input::climb.pressed () )
      return false;

    fmt::print ("corner boost\n");

    dash_state = NotDashing;
    /*is_climbing = true;*/
    climb_jump ();

    return true;
  }

  // ----

  bool is_grounded = false;
  Timer <5> cayotee_timer;
  tick_t time_ungrounded = tick_t_never;
  tick_t time_grounded   = tick_t_never;

  bool check_is_grounded ()
  {
    const float feet_box_h
      = dash_state == NotDashing
      ? 0.15 // for the purpose of landing
      : 0.40 // for the purpose of refreshing dashes and things like air-supers
      ;
    const V2 <float> fpos  { pos.x, pos.y + size.y };
    const V2 <float> fsize { size.x, feet_box_h };
    return hit_test(fpos, fsize);
  }

  void do_grounding ()
  {
    const bool gnew =
      check_is_grounded ();

    if (gnew)
      on_grounded ();

    if (is_grounded == gnew)
      return;

    is_grounded = gnew;

    if (gnew)
    {
      cayotee_timer.stop();
      time_grounded = global::ticks_elapsed;
    }
    else
    {
      cayotee_timer.start();
      time_ungrounded = global::ticks_elapsed;
    }
  }

  void on_grounded ()
  {
    if (dash_refresh_timer.dead ())
      n_dashes = 1;
  }

  // ----

  static constexpr float walk_max      = 90.f / 8;
  static constexpr float walk_accel    = 1;
  static constexpr float walk_reduce   = 400.f / (8 * 120);
  static constexpr float walk_stopping = 0.03;
  static constexpr float fric_ground   = 1.0;
  static constexpr float fric_air      = 0.65;

  float get_friction ()
  {
    return is_grounded ? fric_ground : fric_air;
  }

  // While this timer is on, the player cannot control their x-velocity,
  // but also is not subject to friction. Used for forced movement.
  // tick_t no_move_timer{};
  Sec_Timer <> no_move_timer;

  void do_movement ()
  {
    if (no_move_timer.alive ())
      return;

    const float mx = input::move_x ();
    float vx = vel.x;

    const auto m_sign = signum (mx);
    const auto v_sign = signum (vx);

    const auto mult = global::intended_ticks_per_sec * global::dt;

    if (mx != 0)
      facing = mx;

    if (mx == 0)
    {
      const auto fric = get_friction ();
      const auto vx2  = vx - v_sign * fric * mult;
      vx = signum (vx2) == v_sign ? vx2 : 0;
      vx *= 1 - walk_stopping;
    }
    else if (v_sign != m_sign)
    {
      vx = m_sign * walk_accel * mult;
    }
    else if (vx*v_sign < walk_max)
    {
      const auto vx2 = vx + walk_accel*v_sign * mult;
      vx = vx2*v_sign > walk_max ? walk_max*v_sign : vx2;
    }
    else
    {
      const auto fric = get_friction ();
      const auto vx2  = vx - v_sign * fric * walk_reduce * mult;
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
  Sec_Timer <> zero_grav_timer;

  float get_gravity ()
  {
    float g = gravity_accel;
    if (input::jump.pressed ()) // gravity discounts if holding jump
    {
      if (zero_grav_timer.alive ())
        g = 0;
      else
        g *= 0.5;
    }
    g *= global::intended_ticks_per_sec * global::dt;
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



  tick_t time_last_jump = 0;

  static constexpr
  float jump_liftoff = 13;

  static constexpr float
    /*
    */ super_speed  = 260.f / 8
     , hyper_x_mult = 1.25
     , hyper_y_mult = 0.5
     , ultra_boost  = 1.2
     ;

  bool do_jumping ()
  {
    if (!input::jump.fresh ())
      return false;
    if (input::jump.time () <= time_last_jump)
      return false;

    if (is_climbing)
    {
      climb_jump ();
      return true;
    }

    bool is_cayotee = false;
    bool is_bunny   = false;

    if (! is_grounded)
    {
      /*if ( time_ungrounded + cayotee_time < global::ticks_elapsed*/
      if ( cayotee_timer.dead ()
        || time_ungrounded <= time_last_jump // you only get 1 jump!
         ) return try_walljump ();

      is_cayotee = true;
    }
    else
    {
      is_grounded = false;
      cayotee_timer.stop ();
      time_ungrounded = global::ticks_elapsed;
      is_bunny = time_grounded == time_ungrounded;
    }

    time_last_jump = global::ticks_elapsed;
    zero_grav_timer.start (zero_grav_time);
    vel.y = -jump_liftoff;

    if (dash_state == Dashing)
    {
      dash_state = NotDashing;

      if (dash_direction.x == 0)
        return true;

      //vel.x = dash_vel.x;
      vel.x = super_speed * dash_direction.x;

      if (n_dashes > 0)
        fmt::print ("extended ");

      // allows reversing supers and hypers
      {
        const float mx = input::move_x ();
        if (mx != 0 && mx != dash_direction.x)
        {
          vel.x *= -1;
          fmt::print ("reverse ");
        }
      }

      if (is_cayotee) fmt::print ("cayotee ");
      if (is_bunny  ) fmt::print ("bunny ");

      if (dash_direction.y > 0)
      {
        vel.x *= hyper_x_mult;
        vel.y *= hyper_y_mult;

        // hypers and wave dashes are equivallent, but it is technically
        // called a wave dash if you started mid-air.
        if (time_dash_started < time_grounded)
          fmt::print ("wave ");
        else
          fmt::print ("hyper ");
      }
      else
        fmt::print ("super ");

      fmt::print ("dash\n");
    }
    else
    {
      if (is_cayotee) fmt::print ("cayotee ");
      if (is_bunny  ) fmt::print ("bunny ");

      // We always apply the ultra boost, but don't
      // bother logging it unless significant speed was gained.

      bool nice_ultra = false;
      if (pending_ultra)
      {
        nice_ultra = std::abs(vel.x) >= dash_speed;

        vel.x *= ultra_boost;
      }

      // Unlike supers and hypers, which give constant speed, ultras
      // are multiplicative and can be chained for infinite speed.
      //
      // You usually start a chain of ultras by performing an extended hyper (to obtain some speed),
      // then immediately dash diagonally down. If the dash ends before you reach the ground and you
      // jump, you'll get an ultra boost (vel.x *= 1.2).
      // If the dash does not end before you jump you'll get a hyper instead, which sets your speed to a
      // constant. Continue the chain of ultras by immediately dashing diagonally down after every jump.
      //
      // https://youtu.be/2XQJRG32dzI
      //
      // When ultras are performed on flat ground they are known as "gultras", and are only slightly
      // faster hypers. To get the full infinite scaling from ultras you rely on terrain that looks like this:
      //
      // ####
      //
      //           ####
      //
      //                          ####
      //
      //                                                  ####
      //
      //                                                                             ####
      //
      //                                                                                                                      ####

      if (nice_ultra)
        fmt::print ("ultra\n");
      else if (is_cayotee || is_bunny)
        fmt::print ("jump\n");
    }

    pending_ultra = false;

    return true;
  }

  static constexpr float
    wallbounce_wall_dist = 0.35f;
  static constexpr float
    wallbounce_speed_y = 28.f;
  static constexpr float
    wallbounce_speed_x = 24.f;

  static constexpr float
    walljump_wall_dist = 0.2f;
  static constexpr float
    walljump_speed_x = 12.f;

  int wall_check (float dist)
  {
    return hit_test ({pos.x - dist, pos.y}, {dist, size.y})
         ? -1
         : hit_test ({pos.x + size.x, pos.y}, {dist, size.y})
         ? +1
         : 0;
  }

  bool try_walljump ()
  {
    if (vel.x != 0 || vel.y >= 0 || dash_bounce_timer.dead ())

    // wall jump
    {
      if (dash_state != NotDashing)
        return false;

      const int wall_dir = wall_check (walljump_wall_dist);
      if (wall_dir == 0) return false;

      const float mx = input::move_x ();

      vel.x = -walljump_speed_x * wall_dir;
      vel.y = -jump_liftoff;

      if (mx == 0)
      {
        zero_grav_timer.start(12);
        no_move_timer.start (4);

        fmt::print ("neutral jump\n");

        // Neutral jumps are performed by letting go of all directional input
        // before a wall jump. Unlike regular walljumps, neutrals allow you
        // to gain height; the climbing mechanic is entirely redundant.
        //
        // https://youtu.be/vfinx7aIL3o
      }
      else if (mx != wall_dir)
      {
        // kick away from the wall
        zero_grav_timer.start(zero_grav_time);
        no_move_timer.start (8);
        facing = -mx;
      }
      else
      {
        no_move_timer.start (18);
      }

    }
    else

    // wallbounce
    {
      const int wall_dir = wall_check (wallbounce_wall_dist);
      if (wall_dir == 0) return false;

      dash_state = NotDashing;
      dash_bounce_timer.stop ();

      zero_grav_timer.start(zero_grav_time);

      vel.x = -wallbounce_speed_x * wall_dir;
      vel.y = -wallbounce_speed_y;

      fmt::print ("wall bounce\n");
    }

    time_last_jump = global::ticks_elapsed;

    return true;
  }

  // ----

  static constexpr float climb_attach_wall_dist = 0.1;
  static constexpr float climb_speed = 10.0;
  bool is_climbing = false;

  bool try_climb ()
  {
    if (!input::climb.pressed ())
      return false;

    if (facing > 0)
    {
      if (! hit_test ({pos.x + size.x, pos.y}, {climb_attach_wall_dist, size.y}))
        return false;
    }
    else
    {
      if (! hit_test ({pos.x - climb_attach_wall_dist, pos.y}, {climb_attach_wall_dist, size.y}))
      {
        return false;
      }
    }

    moveX (climb_attach_wall_dist * facing);

    vel.x = 0; vel.y = 0;
    is_climbing = true;
    dash_state = NotDashing;

    return true;
  }
  void climb_update ()
  {
    {
      is_climbing = false;

      if (!input::climb.pressed ())
        return;

      if (facing > 0)
      {
        if (! hit_test ({pos.x + size.x, pos.y}, {climb_attach_wall_dist, size.y}))
          return;
      }
      else
      {
        if (! hit_test ({pos.x - climb_attach_wall_dist, pos.y}, {climb_attach_wall_dist, size.y}))
          return;
      }

      is_climbing = true;
    }

    const float my = input::move_y ();
    if (my == 0)
    {
      if (vel.y >= 0)
        vel.y = 0;
      else
      {
        const float g = get_gravity ();
        vel.y = std::min<float>(0, vel.y + g);
      }
    }
    else if (signum(vel.y) == my)
    {
      if (vel.y > 0 || vel.y * my < climb_speed)
        vel.y = my * climb_speed;
      else
      {
        const float g = get_gravity ();
        vel.y = std::min<float>(-climb_speed, vel.y + g);
      }
    }
    else
    {
      vel.y = my * climb_speed;
    }
  }

  void climb_jump ()
  {
    const float mx = input::move_x ();

    vel.y -= jump_liftoff;
    time_last_jump = global::ticks_elapsed;

    if (mx != -facing)
    {
      zero_grav_timer.start(10);
    }
    else
    {
      zero_grav_timer.start(zero_grav_time);
      vel.x = mx * walljump_speed_x;
      is_climbing = false;
      facing *= -1;
    }
  }

public:

  void die ()
  {
    // TODO: I have respawn points for each level over in `room_stuff.h`,
    // but how to access those in a clean way..
  }

private:

};


