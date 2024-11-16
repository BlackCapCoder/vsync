#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <cstdio>

#include "src/task.h"

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

void key_callback (GLFWwindow * win, int key, int scancode, int action, int mods);
void key_callback (GLFWwindow * win, int key, int scancode, int action, int mods)
{
  if (action != GLFW_PRESS)
    return;

  if (key == GLFW_KEY_ESCAPE || key == 'Q')
  {
    glfwSetWindowShouldClose (win, GLFW_TRUE);
    return;
  }

}


int main()
{
  testTask ();

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

  glfwSetKeyCallback (win, key_callback);

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

