#pragma once

#include <cstdlib>
#include "V2.h"

// ----

using tile_t =
  unsigned char;

struct TileMap
{
  V2 <int> size;
  tile_t * tiles; 
};


TileMap new_tile_map (int w, int h)
{
  tile_t * tiles = reinterpret_cast<tile_t *>(calloc (w*h, 1));
  return TileMap
    { .size  = V2 <int> { .x = w, .y = h }
    , .tiles = tiles
    };
}


// ----

enum Trit { X, O, I };

bool trit_bool (Trit a, bool b)
{
  switch (a)
  {
    case X: return true;
    case O: return !b;
    case I: return b;
  }
}

/*bool match */
