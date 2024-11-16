#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <cstdio>

constexpr int width  = 800;
constexpr int height = 600;
const char * title   = "Boilerplate!";

void resize_callback(GLFWwindow *, int width, int height)
{
  glViewport(0, 0, width, height);
}

const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = vec4(aPos, 1.0);\n"
    "}\n";

const char* fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "    FragColor = vec4(0.2f, 0.3f, 0.3f, 1.0f);\n"
    "}\n";

int main()
{
  glfwInit();
  // 3.3 core
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow * win = glfwCreateWindow(width, height, title, nullptr, nullptr);
  if (!win) {
    printf("Failed to create window...\n");
    return -1;
  }

  glfwMakeContextCurrent(win);
  glfwSetFramebufferSizeCallback(win, resize_callback);

  // Load function pointers
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    printf("Failed to initialize GLAD\n");
    glfwTerminate();
    return -1;
  }

  glViewport(0, 0, width, height);

  // Build and compile the vertex shader
  unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
  glCompileShader(vertexShader);

  // Check for shader compile errors
  int success;
  char infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
    printf("Compilation of vertex shader failed: %s\n", infoLog);
  }

  // Build and compile the fragment shader
  unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
  glCompileShader(fragmentShader);

  // Check for fragment shader compile errors
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
    printf("Compilation of fragment shader failed: %s\n", infoLog);
  }

  // Link the shaders into a program
  unsigned int shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  // Check for linking errors
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
    printf("Linking failed: %s\n", infoLog);
  }

  // Delete the shaders as they're linked now
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  // Define vertices for a simple triangle
  float vertices[] = {
      // Positions
      0.0f,  0.5f,  0.0f, // Top vertex
      -0.5f, -0.5f, 0.0f, // Bottom left vertex
      0.5f,  -0.5f, 0.0f  // Bottom right vertex
  };

  // Generate a VAO and VBO
  unsigned int VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  // Bind the VAO
  glBindVertexArray(VAO);

  // Bind the VBO and load data into the buffer
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Specify the layout of the vertex data (position attribute)
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
  glEnableVertexAttribArray(0);

  // Unbind the VBO and VAO (good practice)
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  // Use the shader program
  glUseProgram(shaderProgram);

  while (!glfwWindowShouldClose(win)) {
    glfwPollEvents();

    glClearColor(0.2f, 0.2f, 0.2f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the triangle
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3); // Draw 3 vertices as a triangle

    glfwSwapBuffers(win);
  }

  // Delete the VBO and VAO- good practice I guess
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);

  glfwTerminate();
}


#if 0
/*#include <GL/gl.h>*/
/*#define GLX_GLXEXT_PROTOTYPES*/
/*#include <GL/glx.h>*/
#include <GLFW/glfw3.h>
#include <cstdlib>

#define GLAD_GL_IMPLEMENTATION
#include <OpenGL/OpenGL.h>
// #include <GL/gl.h>
// #include <GL/glu.h>

#include <stdlib.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <map>

/*#define STB_IMAGE_IMPLEMENTATION*/
/*#include "libs/stb_image.h"*/

#include "src/V2.h"
#include "src/drawing.h"
#include "src/tilemap.h"


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

struct World
{
  tick_t elapsed_ticks = 0;
  std::map<int, KeyMapEntry> keymap {};

  float px, py;
  const float psize;
  const float pspeed;

  TexObj tex;
  float tx, ty;
};

World mkWorld ()
{
  return World
    { .px = 0
    , .py = 0
    , .psize  = 100
    , .pspeed = 10
    , .tx = 0
    , .ty = 0
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

void init_textures ()
{
  const auto tex = load_texture ("res/tilesets/dirt.png");
  world.tex = tex;
}


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

      const auto s = 64.0;
      const auto tw = 8.0/48.0;
      const auto th = 8.0/120.0;

      const auto x = 1.0;
      const auto y = 1.0;
      const auto tx = world.tx;
      const auto ty = world.ty;

      const auto tx_ = tx;
      const auto ty_ = ty;

      draw_tile (tw,th,s,tx_,ty_,s*x,s*y);


    }
    glDisable (GL_TEXTURE_2D);

  }
  glPopMatrix();
}


// ----

template <class N>
V2 <N> get_movement (char n, char e, char s, char w)
{
  V2 <N> move { .x = 0, .y = 0 };

  if (key_pressed (w)) move.x -= 1;
  if (key_pressed (e)) move.x += 1;
  if (key_pressed (n)) move.y -= 1;
  if (key_pressed (s)) move.y += 1;

  return move;
}

void tick ()
{
  const auto move = get_movement <float> ('E','F','D','S');
  world.px += move.x * world.pspeed;
  world.py += move.y * world.pspeed;

  const float tspeed = 0.1;
  const auto tmove = get_movement <float> ('K','L','J','H');
  world.tx += tmove.x * tspeed;
  world.ty += tmove.y * tspeed;
}

// ----


size_t get_time_chrono ()
{
  using namespace std::chrono_literals;
  const auto now = std::chrono::high_resolution_clock::now();
  const auto x = now.time_since_epoch().count();
  return x;
}

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

#endif
