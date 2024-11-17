#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>

#include "src/texture.h"
#include "src/shader.h"
#include "src/V2.h"
#include "src/tilemap.h"

// ----------

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

unsigned int window_width  = 800;
unsigned int window_height = 600;

GLFWwindow * window;
Shader shader;
glm::vec3 cam { -20, -10, 0.0 };

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
T get_move (char l, char r)
{
  T move = 0;
  if (key_pressed (l)) move -= 1;
  if (key_pressed (r)) move += 1;
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

// ----

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
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval (1);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // build and compile our shader zprogram
  // ------------------------------------
  shader = Shader ("shaders/4.1.texture.vs", "shaders/4.1.texture.fs");


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
  auto tms = load_tilemaps ("lvl");
  std::cout << "got " << tms.size() << " tilemaps!" << std::endl;

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
     0.5f,  0.5f, 0.0f, (1.0f), (1.0f), // top right
     0.5f, -0.5f, 0.0f, (1.0f), (0.0f), // bottom right
    -0.5f, -0.5f, 0.0f, (0.0f), (0.0f), // bottom left
    -0.5f,  0.5f, 0.0f, (0.0f), (1.0f)  // top left 
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
      processInput(window);

      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      /*tex.use ();*/
      shader.use ();

      glBindVertexArray(VAO);

      glm::mat4 model = glm::mat4 (1.0f);
      model = glm::translate(model, cam);

      for (auto & tm : tms)
      {
        /*std::cout << tm.pos.x << ", " << tm.pos.y << std::endl;*/

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

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
          }
        }
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

  const float zoom_speed = 0.5;
  cam.z += get_move <float> ('O', 'I') * zoom_speed;

  const auto move = get_movement <float> ('E','F','D','S');
  float speed = 0.3;

  if (cam.z > 0)
  {
    speed *= 1.0 - cam.z/32.0;
  }
  else
  {
    speed *= 1.0 + (-cam.z)/32.0;
  }

  cam.x += move.x * speed;
  cam.y += move.y * speed;

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  window_width = width;
  window_height = height;

  glViewport(0, 0, width, height);

  {
    glm::mat4 view  = glm::mat4(1.0f);
    glm::mat4 proj  = glm::mat4(1.0f);
    glm::mat4 model = glm::mat4(1.0f);

    proj = glm::perspective(glm::radians(45.0f), (float)window_width / (float)window_height, 0.1f, 100.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -64.0f * 0.5f));
    view[1] *= -1.0;

    shader.setMat4("projection", proj );
    shader.setMat4("view",       view );
    shader.setMat4("model",      model);
  }
}
