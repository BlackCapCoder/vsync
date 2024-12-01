#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec2 size;

void main()
{
  gl_Position = projection * view * model *
    vec4
    ( aPos * size
    , 0
    , 1.0);
}
