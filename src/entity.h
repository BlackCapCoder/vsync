#pragma once

#include "V2.h"
#include "shader.h"
#include "input.h"
#include <fmt/printf.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <stack>

// ----

static Shader shader, simple_shader;

// ----

struct Entity
{
  virtual void tick ()
  {
  }

  virtual void render (glm::mat4 model)
  {
  }
};

static std::vector<Entity *> entities;

// ----

struct Particle
{
  V2 <float> pos {}, vel {}, grav {};
  float size = 0.1;
  tick_t birth;
  int ttl = 10;
  float alpha = 1.0;
  float start_size = 0.2;
  float r,g,b = 1.0;
  float fric = 3.5;

  bool tick ()
  {
    const int age = global::ticks_elapsed - birth;
    if (age > ttl)
      return false;

    const float a = (float) age / ttl;

    pos += vel * global::dt;
    vel += grav * 100 * global::dt;
    vel -= vel * fric * global::dt;
    alpha = 1.0 - std::pow(a, 4.0);
    size  = start_size * (1.0 - std::pow(a, 5.0) * 1.0);

    return true;
  }
};

static int roll (int size)
{
  return rand () % size;
}
static float rollf ()
{
  return (float) (rand ()) / RAND_MAX;
}
static float rollf (float lo)
{
  return 1.0 - lo * rollf ();
}
static int roll_sign ()
{
  const int rng = rand ();
  return (rng & 2) - 1;
}

struct Particles : Entity
{
  std::stack <Particle> ps {};

  void tick () override
  {
    std::stack <Particle> next;
    while (! ps.empty ())
    {
      if (ps.top().tick ())
        next.push (ps.top ());
      ps.pop ();
    }
    ps = next;
  }
  void render (glm::mat4 model) override
  {
    simple_shader.use ();

    std::stack <Particle> next;
    while (! ps.empty ())
    {
      const auto p = ps.top ();
      ps.pop ();
      {
        const auto model_ = glm::translate(model, {p.pos.x - p.size/2, p.pos.y - p.size/2, 0.0});
        simple_shader.setMat4("model", model_);
        simple_shader.setVec2("size", p.size, p.size);
        simple_shader.setVec4 ("color", p.r, p.g, p.b, p.alpha);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
      }
      next.push (p);
    }
    ps = next;

  }


  void spawn ( V2 <float> pos, V2 <float> off, float angle, float spread, float speed_min, float speed
             , float grav, float r, float g, float b
             , float size_, int ttl_, float fric
             )
  {
    const int ttl = 20 + roll (ttl_);
    const float size = size_ * rollf (0.7);

    const float speed_ = speed_min + speed * rollf (0.8);
    const float angle_ = (angle + spread * (rollf () - 0.5)) * 3.141592653589793238 * 2.0;
    V2 <float> v = V2 <float> { std::sin(angle_), std::cos(angle_) };

    ps.push (Particle
      { .pos = pos + v * off
      , .vel = v * speed_
      , .grav = { 0.0, grav }
      , .size = size
      , .birth = global::ticks_elapsed
      , .ttl = (int) global::scaled_ticks (ttl)
      , .start_size = size
      , .r = r, .g = g, .b = b
      , .fric = fric
      });

  }

};

// ----

static Particles * particles = new Particles {};

static void init_entities ()
{
  entities.push_back
    ( particles
    );
}

