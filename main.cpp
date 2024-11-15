/*#include <GL/gl.h>*/
/*#define GLX_GLXEXT_PROTOTYPES*/
/*#include <GL/glx.h>*/
#include <GLFW/glfw3.h>

#define GLAD_GL_IMPLEMENTATION
#include <GL/gl.h>
#include <GL/glu.h>

#include <stdlib.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <map>

#define STB_IMAGE_IMPLEMENTATION
#include "libs/stb_image.h"


// -----

using tick_t = size_t;

constexpr
tick_t tick_t_never = 0;

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

// ----

using TexObj =
  unsigned int;

struct World
{
  tick_t elapsed_ticks = 0;
  std::map<int, KeyMapEntry> keymap {};

  float px, py;
  const float psize;
  const float pspeed;

  TexObj tex;
};

World mkWorld ()
{
  return World
    { .px = 0
    , .py = 0
    , .psize  = 100
    , .pspeed = 10
    };
}

World world =
  mkWorld ();


// ----

void key_callback (GLFWwindow * win, int key, int scancode, int action, int mods);


KeyMapEntry & getKey (int && key)
{
  return world.keymap [key];
}

bool key_pressed (int && key)
{
  if (auto search = world.keymap.find (key); search == world.keymap.end ())
    return false;
  else
    return search->second.state;
}


void key_callback (GLFWwindow * win, int key, int scancode, int action, int mods)
{
  if (mods != 0)
    return;

  // ==== Update the keymap ====

  if (action == GLFW_PRESS || action == GLFW_RELEASE)
  {
    KeyMapEntry newE
      { .state = action == GLFW_PRESS
      , .time  = world.elapsed_ticks
      };

    if (auto search = world.keymap.find (key); search == world.keymap.end ())
      world.keymap.insert_or_assign (key, newE);
    else
    if (auto & oldE = search->second; oldE.state ^ newE.state)
      oldE = newE;
  }

  // ----

  if (action != GLFW_PRESS)
    return;

  /*std::cout << key << ", " << mods << " : " << (int) 'Q' << std::endl;*/
  /*std::cout << key << ", " << scancode << ", " << action << ", " << mods << std::endl;*/

  if (key == GLFW_KEY_ESCAPE || key == 'Q')
  {
    std::cout << "Quit!" << std::endl;
    glfwSetWindowShouldClose (win, GLFW_TRUE);
    return;
  }

}


int winW, winH;

void reshape (GLFWwindow * win, int w, int h)
{
  glViewport (0,0, (GLsizei) w, (GLsizei) h);
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluOrtho2D (0, w, h, 0);
  glMatrixMode (GL_MODELVIEW);
}

// ====

void drawRectPtPt (float x1, float y1, float x2, float y2)
{
  glBegin (GL_QUADS);
  {
    glVertex2f (x1, y1);
    glVertex2f (x2, y1);
    glVertex2f (x2, y2);
    glVertex2f (x1, y2);
  }
  glEnd ();
}
void drawRectPtSz (float x, float y, float w, float h)
{
  drawRectPtPt (x, y, x+w, y+h);
}
void drawRectSz (float w, float h)
{
  drawRectPtSz (0.0, 0.0, w, h);
}



TexObj load_texture (char * pth)
{
  unsigned int tex;
  glGenTextures (1, &tex);
  glBindTexture (GL_TEXTURE_2D, tex);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  int width, height, nchannels;
  unsigned char * img = stbi_load (pth, &width, &height, &nchannels, 0);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
  stbi_image_free (img);

  /*std::cout << width << ", " << height << std::endl;*/

  return tex;
}

void init_textures ()
{
  /*glGenerateMipmap(GL_TEXTURE_2D);*/
  const auto tex = load_texture ("res/tilesets/girder.png");
  world.tex = tex;
}

void draw_tile (float tw, float th, float s, float tx, float ty, float x, float y)
{
  const auto x1 = x;
  const auto x2 = x1+s;
  const auto y1 = y;
  const auto y2 = y1+s;

  const auto tx1 = tx*tw;
  const auto tx2 = tx1*tw;
  const auto ty1 = ty*th;
  const auto ty2 = ty1+th;

  glBegin (GL_QUADS);
  {
    glTexCoord2f (tx1, ty1);
    glVertex2f (x1, y1);
    glTexCoord2f (tx2, ty1);
    glVertex2f (x2, y1);
    glTexCoord2f (tx2, ty2);
    glVertex2f (x2, y2);
    glTexCoord2f (tx1, ty2);
    glVertex2f (x1, y2);
  }
  glEnd ();
}


// ----

void display ()
{
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glPushMatrix();
  {
    glColor4f (1.0, 0.0, 0.0, 1.0);
    drawRectPtSz (world.px, world.py, world.psize, world.psize);

    glEnable (GL_TEXTURE_2D);
    {
      glColor3f (1,1,1);
      glBindTexture (GL_TEXTURE_2D, world.tex);

      const auto s = 128.0;
      const auto tw = 1.0; // 8.0/48.0;
      const auto th = 1.0; // 8.0/120.0;

      const auto x = 2.0;
      const auto y = 2.0;
      const auto tx = 0.0;
      const auto ty = 0.0;

      draw_tile (tw,th,s,tx,ty,s*x,s*y);

    }
    glDisable (GL_TEXTURE_2D);

  }
  glPopMatrix();
}


// ----

template <class T>
struct V2
{
  T x, y;
};

template <class N>
V2 <N> get_movement ()
{
  V2 <N> move { .x = 0, .y = 0 };

  if (key_pressed ('S')) move.x -= 1;
  if (key_pressed ('F')) move.x += 1;
  if (key_pressed ('E')) move.y -= 1;
  if (key_pressed ('D')) move.y += 1;

  return move;
}

void tick ()
{
  const auto move = get_movement <float> ();
  world.px += move.x * world.pspeed;
  world.py += move.y * world.pspeed;
}

// ----


size_t get_time_chrono ()
{
  using namespace std::chrono_literals;
  const auto now = std::chrono::high_resolution_clock::now();
  const auto x = now.time_since_epoch().count();
  return x;
}

// size_t get_time_asm ()
// {
//   return __rdtsc ();
// }

// --------


// -----


int main ()
{
  GLFWwindow * win;

  if (! glfwInit ())
    exit (EXIT_FAILURE);

  win = glfwCreateWindow (640, 480, "title", NULL, NULL);

  if (! win)
  {
    glfwTerminate ();
    exit (EXIT_FAILURE);
  }

  // configure window
  {
    glfwSetInputMode (win, GLFW_STICKY_KEYS, GLFW_TRUE);
    /*glfwSetInputMode (win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);*/
    glfwSetWindowAspectRatio (win, 16, 9);
  }

  // window callbacks
  {
    glfwSetKeyCallback (win, key_callback);
  }

  // configure gl context
  {
    glfwMakeContextCurrent (win);
    glfwSwapInterval (1); // vsync

    glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
    glShadeModel (GL_FLAT);

    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glfwGetFramebufferSize (win, &winW, &winH);
    reshape (win, winW, winH);
  }
  /*glXGetCurrentDisplay();*/

  {
    init_textures ();
  }

  glfwSetTime (0.0);


  static constexpr bool verbose = false;

  auto before = get_time_chrono ();

  while (!glfwWindowShouldClose (win))
  {
    glfwPollEvents ();
    tick ();
    display ();
    auto middle = get_time_chrono ();


    glFlush ();
    glfwSwapBuffers (win);
    world.elapsed_ticks++;

    auto after = get_time_chrono ();

    const auto dt1 = middle - before;
    const auto dt2 = after  - before;
    before = after;

    if (verbose)
      std::cout << dt1 << ", " << dt2 << std::endl;
  }

  glfwTerminate ();
  exit (EXIT_SUCCESS);
}

