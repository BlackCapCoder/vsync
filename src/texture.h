#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include "../libs/stb_image.h"
#include "../glad/glad.h"


struct Texture
{
  unsigned int ID;
  int width, height, nrChannels;

  Texture ()
  {
  }

  Texture (const char * pth)
  {
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    unsigned char * img =
      stbi_load (pth, &width, &height, &nrChannels, 0);

    if (!img)
    {
      throw "Failed to load image";
    }

    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
    glGenerateMipmap (GL_TEXTURE_2D);

    stbi_image_free (img);
  }

  void use () const
  {
    glBindTexture (GL_TEXTURE_2D, ID);
  }

};

