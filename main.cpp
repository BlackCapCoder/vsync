#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fmt/core.h>
#include <vector>

#include "src/texture.h"
#include "src/shader.h"
#include "src/V2.h"
#include "src/tilemap.h"
#include "src/player.h"
#include "src/input.h"
#include "src/room_stuff.h"

// ----------

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

unsigned int window_width  = 800;
unsigned int window_height = 600;

GLFWwindow * window;
Shader shader, simple_shader;
glm::vec3 cam { 0, 0, 0 };

std::vector<TileMapEx> tilemaps;

// ----------

struct GlBuf
{
  const int buf_ty;
  unsigned int ID;

  GlBuf (const int buf_ty) : buf_ty { buf_ty }
  {
    glGenBuffers (1, &ID);
  }
  ~GlBuf ()
  {
    glDeleteBuffers (1, &ID);
  }

  void use ()
  {
    glBindBuffer (buf_ty, ID);
  }

  template <class T, int N>
  using Arr = T [N];

  template <class T, int N>
  void fill (const Arr <T, N> & data)
  {
    use ();
    glBufferData(buf_ty, sizeof (data), data, GL_STATIC_DRAW);
  }
};

// ----------

bool key_pressed (int key)
{
  return glfwGetKey (window, key) != GLFW_RELEASE;
}

template <class T>
bool pt_in_rect (V2 <T> p1, V2 <T> p2, V2 <T> size)
{
  if (p1.x < p2.x || p1.y < p2.y) return false;
  const V2 <T> p3 { p2.x+size.x, p2.y+size.y };
  if (p1.x >= p3.x || p1.y >= p3.y) return false;
  return true;
}

template <class T>
bool rect_in_rect (V2 <T> p1, V2 <T> s1, V2 <T> p2, V2 <T> s2)
{
  return std::max(p1.x, p2.x) < std::min(p1.x + s1.x, p2.x + s2.x)
      && std::max(p1.y, p2.y) < std::min(p1.y + s1.y, p2.y + s2.y);
}

bool hit_test_int (V2 <int> pos)
{
  for (auto tm : tilemaps)
  {
    if (pt_in_rect <int> (pos, tm.pos, tm.size))
    {
      const V2 <int> lpos { pos.x - tm.pos.x, pos.y - tm.pos.y };
      const auto & tile = tm[lpos];
      return tile.is_nonempty();
    }
  }
  return false;
}

bool hit_test (V2 <float> pos)
{
  return hit_test_int ({(int) pos.x, (int) pos.y});
}

bool hit_test (V2 <float> p1, V2 <float> s1)
{
  for (auto tm : tilemaps)
  {
    const V2 <float> p2 { (float) tm.pos.x,  (float) tm.pos.y  };
    const V2 <float> s2 { (float) tm.size.x, (float) tm.size.y };
    if (rect_in_rect (p1,s1,p2,s2))
    {
      const int x0 = std::max<int>((int) p1.x, tm.pos.x) - tm.pos.x;
      const int y0 = std::max<int>((int) p1.y, tm.pos.y) - tm.pos.y;
      const int x1 = std::min<int>((int) (p1.x + s1.x), tm.pos.x + tm.size.x) - tm.pos.x;
      const int y1 = std::min<int>((int) (p1.y + s1.y), tm.pos.y + tm.size.y) - tm.pos.y;

      const int off_y = tm.pos.y < 0 ? -1 : 0; // why do I need this?

      for (int y = y0; y <= y1; y++)
      {
        int y3 = y+off_y;
        if (y3 < 0 || y3 >= tm.size.y) continue;
        for (int x = x0; x <= x1; x++)
        {
          if (x < 0 || x >= tm.size.x) continue;
          const auto & tile = tm[{x, y3}];
          if (tile.is_nonempty())
            return true;
        }
      }
    }
  }
  return false;
}


// ----

// int current_screen = 0;
// Player player
//   (V2<float>{ 3, 16 }
//   );

// int current_screen = 12;
// Player player
//   (V2<float>{ 82+10, -174 + 17 }
//   );

int current_screen = 6;
Player player
  (V2<float>{ 162+8, -151 + 16 }
  );

int get_current_screen ()
{
  int i = 0;
  for (auto tm : tilemaps)
  {
    const V2 <float> p1 { player.pos.x + player.size.x/2, player.pos.y + player.size.y/2};
    const V2 <float> p2 { (float) tm.pos.x,  (float) tm.pos.y  };
    const V2 <float> s2 { (float) tm.size.x, (float) tm.size.y };
    if (pt_in_rect (p1,p2,s2))
      return i;
    i++;
  }
  return -1;
}

void main_tick ()
{
  processInput(window);

  if (global::ticks_to_skip)
  {
    global::ticks_to_skip--;
    return;
  }

  player.tick ();
  const int new_screen = get_current_screen ();

  if (new_screen >= 0 && current_screen != new_screen)
  {
    current_screen = new_screen;
    const auto tm = tilemaps[current_screen];
    fmt::print("new screen: {}, {}, {}\n", current_screen, tm.pos.x, tm.pos.y);

    global::ticks_to_skip = 20; // sleep a bit after changing screens
    player.n_dashes = 1;
  }

  {
    const auto tm = tilemaps[current_screen];
    const float w = 40.f;
    const float h = w/window_width*window_height;

    float x = player.pos.x - player.size.x/2 - w/2;
    float y = player.pos.y - player.size.y/2 - h/2;

    if (auto x2 = tm.pos.x + tm.size.x - w; x > x2) x = x2;
    if (auto y2 = tm.pos.y + tm.size.y - h; y > y2) y = y2;
    if (x < tm.pos.x) x = tm.pos.x;
    if (y < tm.pos.y) y = tm.pos.y;

    if (tm.size.y < h)
    {
      y += (tm.size.y - h)/2;
    }

    cam.x = -x;
    cam.y = -y;
  }

  global::ticks_elapsed++;
}

// ----

void key_callback (GLFWwindow * win, int key, int scancode, int action, int mods);

int main ()
{
  // glfw: initialize and configure
  // ------------------------------
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif


  // glfw window creation
  // --------------------
  window = glfwCreateWindow(window_width, window_height, "LearnOpenGL", NULL, NULL);
  if (window == NULL)
  {
    fmt::print("Failed to create GLFW window\n");
    glfwTerminate();
    return -1;
  }

  glfwSetInputMode (window, GLFW_STICKY_KEYS, GLFW_TRUE);
  glfwSetKeyCallback (window, key_callback);

  glfwMakeContextCurrent(window);
  glfwSwapInterval (1);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    fmt::print("Failed to initialize GLAD\n");
    return -1;
  }

  // build and compile our shader zprogram
  // ------------------------------------
  shader = Shader ("shaders/4.1.texture.vs", "shaders/4.1.texture.fs");
  simple_shader = Shader ("shaders/simple.vs", "shaders/simple.fs");


  // Each tile in the `lvl` file is either 0 if the tile is empty,
  // or 1+the index of the tileset to use, in the order of the `texs`
  // array below.
  //
  // The `load_tilemaps` function automatically applies autotiling rules
  // to figure out which individual tile to use from within the given tileset.
  //
  // We could do the autotiling in the render loop instead, but it's sort of expensive
  // and hopefully they won't change at runtime.
  //
  tilemaps = load_tilemaps ("lvl");
  fmt::print("got {} tilemaps!\n", tilemaps.size());
  fmt::print("x: {}, y: {}\n", tilemaps[0].size.x, tilemaps[0].size.y);

  Texture texs [] =
    { Texture ("res/tilesets/girder.png")
    , Texture ("res/tilesets/snow.png")
    , Texture ("res/tilesets/dirt.png")
    , Texture ("res/tilesets/cement.png")
    };

  const float tw = 8.0 / texs[0].width;
  const float th = 8.0 / texs[0].height;

  shader.use ();
  shader.setVec2("tile_size", tw, th);

  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  float vertices [20] =
  {
    // positions        // texture coords
    1.0f, 1.0f, 0.0f, (1.0f), (1.0f), // top right
    1.0f, 0.0f, 0.0f, (1.0f), (0.0f), // bottom right
    0.0f, 0.0f, 0.0f, (0.0f), (0.0f), // bottom left
    0.0f, 1.0f, 0.0f, (0.0f), (1.0f)  // top left
  };
  unsigned int indices [] =
  {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
  };

  {
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);

    GlBuf vbo (GL_ARRAY_BUFFER);
    GlBuf ebo (GL_ELEMENT_ARRAY_BUFFER);

    glBindVertexArray(VAO);

    vbo.fill (vertices);
    ebo.fill (indices);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // -------------------------

    while (!glfwWindowShouldClose(window))
    {
      main_tick ();

      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      glm::mat4 model = glm::mat4 (1.0f);
      model = glm::translate(model, glm::vec3(cam.x,cam.y, 0.f));
      {
        const float mult  = 0.1f;
        float scale = 1.f + cam.z * mult;

        model = glm::scale(model, glm::vec3(scale, scale, 1.f));
      }

      // tilemap
      if (1)
      {
        shader.use ();
        glBindVertexArray(VAO);

        auto tm = tilemaps[current_screen];
        /*for (auto & tm : tilemaps)*/
        {
          for (int y = 0; y < tm.size.y; y++)
          {
            for (int x = 0; x < tm.size.x; x++)
            {
              const int i = y*tm.size.x+x;
              const auto tile = tm.tiles[i];
              if (tile.is_empty()) continue;

              const auto & tex = texs[tile.get_tileset()];
              tex.use();

              auto model_ = glm::translate(model, {tm.pos.x+x, tm.pos.y+y, 0});
              shader.setMat4("model", model_);
              shader.setVec2("tile_pos", tile.tx, tile.ty);
              // shader.setVec2("tile_pos", 0, 14); // What it looks like without auto-tiling

              glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
          }
        }

      }

      // player
      {
        simple_shader.use ();
        glBindVertexArray (VAO);

        auto model_ = glm::translate(model, {player.pos.x, player.pos.y, 0.0});
        simple_shader.setMat4("model", model_);
        simple_shader.setVec2("size", player.size.x, player.size.y);

        if (player.dash_state == Player::DirectionPending)
          simple_shader.setVec3 ("color", 1.f, 1.f, 1.f);
        else if (player.n_dashes == 0)
          simple_shader.setVec3 ("color", 0.4f, 0.4f, 1.f);
        else
          simple_shader.setVec3 ("color", 1.f, 0.f, 0.f);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
      }

      glfwSwapBuffers(window);
      glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
  }

  glfwTerminate();

  return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);
  if (glfwGetKey(window, 'Q') == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, 'P') == GLFW_PRESS)
  {
    const auto & tm = tilemaps[current_screen];
    const auto p = player.pos - V2 <float> {(float) tm.pos.x, (float) tm.pos.y};
    fmt::print("player pos: {}, {}\n", (int) p.x, (int) p.y);
  }

  // const float zoom_speed = 0.1;
  // cam.z += get_move <float> ('O', 'I') * zoom_speed;

  // V2 <float> move { 0.0, 0.0 };
  // if (key_pressed (GLFW_KEY_LEFT )) move.x += 1.0;
  // if (key_pressed (GLFW_KEY_RIGHT)) move.x -= 1.0;
  // if (key_pressed (GLFW_KEY_UP )) move.y += 1.0;
  // if (key_pressed (GLFW_KEY_DOWN)) move.y -= 1.0;

  // float speed = 0.3;

  // if (cam.z > 0)
  // {
  //   speed *= 1.0 - cam.z/32.0;
  // }
  // else
  // {
  //   speed *= 1.0 + (-cam.z)/32.0;
  // }

  // cam.x += move.x * speed;
  // cam.y += move.y * speed;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow*, int width, int height)
{
  window_width = width;
  window_height = height;

  glViewport(0, 0, width, height);

  {
    glm::mat4 view  = glm::mat4(1.0f);
    glm::mat4 proj  = glm::mat4(1.0f);
    glm::mat4 model = glm::mat4(1.0f);

    proj = glm::ortho(0.f, (float) window_width, (float) window_height, 0.f, -1.f, 1.f);

    const float scale = (float) window_width / 40;
    view = glm::scale(view, glm::vec3(scale, scale, 1.f));

    shader.use();
    shader.setMat4("projection", proj );
    shader.setMat4("view",       view );
    shader.setMat4("model",      model);

    simple_shader.use();
    simple_shader.setMat4("projection", proj );
    simple_shader.setMat4("view",       view );
    simple_shader.setMat4("model",      model);
  }
}

void key_callback (GLFWwindow * win, int key, int scancode, int action, int mods)
{
  global::keymap.put (global::ticks_elapsed, key,scancode,action,mods);
}

