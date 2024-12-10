#version 330 core
out vec4 FragColor;

uniform vec4 color;

void main()
{
  FragColor = color; // vec4(color.x, color.y, color.z, 1.0f);
} 
