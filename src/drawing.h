#pragma once

#define GLAD_GL_IMPLEMENTATION
#include <GL/gl.h>
#include <GL/glu.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../libs/stb_image.h"


using TexObj = unsigned int;


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

void draw_tile (float tw, float th, float s, float tx, float ty, float x, float y)
{
  const auto x1 = x;
  const auto x2 = x1+s;
  const auto y1 = y;
  const auto y2 = y1+s;

  const auto tx1 = tx*tw;
  const auto tx2 = tx1+tw;
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
