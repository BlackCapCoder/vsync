#pragma once

#include <cstdio>
#include <cstdlib>
#include "V2.h"
#include <cstring>
#include <optional>
#include <stdexcept>
#include <vector>
#include <utility>

// ----

using tile_t =
  unsigned char;

struct TileMap
{
  V2 <int> pos{};
  V2 <int> size;
  tile_t * tiles;

  tile_t & operator [] (V2<int> pos)
  {
    return tiles [pos.x + pos.y * size.x];
  }
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
    default: throw std::runtime_error("Implement me");
  }
}

bool match (const bool x [8], const Trit pat [8])
{
  for (int i = 0; i < 8; i++)
  {
    if (!trit_bool(pat[i], x[i])) return false;
  }
  return true;
}

template <class T>
struct Rule
{
  Trit pat [8];
  T res;
};


Rule <V2 <int>> rules [] =
  { Rule <V2 <int>> {
      { I, I, I
      , I,    I
      , I, I, I }, V2 <int> {5, 0} }

  , Rule <V2 <int>> {
      { X, O, X
      , I,    I
      , X, I, X }, V2 <int> {0, 0} }
  , Rule <V2 <int>> {
      { X, I, X
      , I,    I
      , X, O, X }, V2 <int> {0, 1} }
  , Rule <V2 <int>> {
      { X, I, X
      , O,    I
      , X, I, X }, V2 <int> {0, 2} }
  , Rule <V2 <int>> {
      { X, I, X
      , I,    O
      , X, I, X }, V2 <int> {0, 3} }

  , Rule <V2 <int>> {
      { X, O, X
      , I,    I
      , X, O, X }, V2 <int> {0, 4} }
  , Rule <V2 <int>> {
      { X, I, X
      , O,    O
      , X, I, X }, V2 <int> {0, 5} }

  , Rule <V2 <int>> {
      { X, O, X
      , O,    O
      , X, I, X }, V2 <int> {0, 6} }
  , Rule <V2 <int>> {
      { X, I, X
      , O,    O
      , X, O, X }, V2 <int> {0, 7} }

  , Rule <V2 <int>> {
      { X, O, X
      , O,    I
      , X, O, X }, V2 <int> {0, 8} }
  , Rule <V2 <int>> {
      { X, O, X
      , I,    O
      , X, O, X }, V2 <int> {0, 9} }

  , Rule <V2 <int>> {
      { X, O, X
      , O,    O
      , X, O, X }, V2 <int> {0, 10} }

  , Rule <V2 <int>> {
      { X, O, X
      , O,    I
      , X, I, X }, V2 <int> {0, 11} }

  , Rule <V2 <int>> {
      { X, O, X
      , I,    O
      , X, I, X }, V2 <int> {0, 12} }

  , Rule <V2 <int>> {
      { X, I, X
      , O,    I
      , X, O, X }, V2 <int> {0, 13} }
  , Rule <V2 <int>> {
      { X, I, X
      , I,    O
      , X, O, X }, V2 <int> {0, 14} }

  , Rule <V2 <int>> {
      { I, I, I
      , I,    I
      , I, I, O }, V2 <int> {4, 0} }
  , Rule <V2 <int>> {
      { I, I, O
      , I,    I
      , I, I, I }, V2 <int> {4, 1} }
  , Rule <V2 <int>> {
      { I, I, I
      , I,    I
      , O, I, I }, V2 <int> {4, 2} }
  , Rule <V2 <int>> {
      { O, I, I
      , I,    I
      , I, I, I }, V2 <int> {4, 3} }

  , Rule <V2 <int>> {
      { I, I, O
      , I,    I
      , I, I, O }, V2 <int> {4, 4} }
  , Rule <V2 <int>> {
      { O, I, O
      , I,    I
      , I, I, I }, V2 <int> {4, 5} }
  , Rule <V2 <int>> {
      { O, I, I
      , I,    I
      , O, I, I }, V2 <int> {4, 6} }
  , Rule <V2 <int>> {
      { I, I, I
      , I,    I
      , O, I, O }, V2 <int> {4, 7} }

  , Rule <V2 <int>> {
      { O, I, O
      , I,    I
      , I, I, O }, V2 <int> {4, 8} }
  , Rule <V2 <int>> {
      { O, I, O
      , I,    I
      , O, I, I }, V2 <int> {4, 9} }
  , Rule <V2 <int>> {
      { O, I, I
      , I,    I
      , O, I, O }, V2 <int> {4, 10} }
  , Rule <V2 <int>> {
      { I, I, O
      , I,    I
      , O, I, O }, V2 <int> {4, 11} }

  , Rule <V2 <int>> {
      { O, I, O
      , I,    I
      , O, I, O }, V2 <int> {4, 12} }
  , Rule <V2 <int>> {
      { I, I, O
      , I,    I
      , O, I, I }, V2 <int> {4, 13} }
  , Rule <V2 <int>> {
      { O, I, I
      , I,    I
      , I, I, O }, V2 <int> {4, 14} }
  };

std::optional<V2<int>> doRules (const bool x [8])
{
  for (const auto rule : rules)
  {
    if (match (x, rule.pat)) return std::optional<V2<int>> {rule.res};
  }
  return std::nullopt;
}

bool solid (TileMap tm, V2<int> pos)
{
  if (pos.x < 0 || pos.x >= tm.size.x || pos.y < 0 || pos.y >= tm.size.y) return true;
  return tm[pos] != 0;
}

std::optional<V2<int>> auto_tile (TileMap tm, V2<int> pos)
{
  const auto q = solid (tm, pos);
  if (!q) return std::nullopt;

  const V2<int> pts [8] =
    { V2<int> {-1, -1}
    , V2<int> {0,-1}
    , V2<int> {1,-1}
    , V2<int> {-1,0}
    , V2<int> {1,0}
    , V2<int> {-1,1}
    , V2<int> {0,1}
    , V2<int> {1,1}
    };

  bool ns [8];
  for (int i = 0; i < 8; i++)
  {
    const auto off = pts[i];
    ns[i] = solid (tm, V2 <int> {pos.x + off.x, pos.y + off.y});
  }
  /*std::cout << ns[0] << ns[1] << ns[2] << '\n' << ns[3] << ' ' << ns[4] << '\n' << ns[5] << ns[6] << ns[7] << std::endl;*/

  return doRules (ns);
}

// -----

// Each tile has a few variations or "flavors". This function
// psuedo-randomly chooses a flavor for each tile.
//
// The fill tiles (tiles that does not border with air)
// come in different "intensities", and we want to reduce the intensity
// the further the tile is from air.
//
char * flavor (const TileMap & tm)
{
  const int size = tm.size.x * tm.size.y;

  char * buf = reinterpret_cast<char *>(malloc(size));
  for (int i = 0; i < size; i++)
  {
    buf[i] = (tm.tiles[i] == 0) ? 0 : 14;
  }

  char * tmp = reinterpret_cast<char *>(malloc(size));
  for (int j = 0; j < 2; j++)
  {
    memcpy(tmp, buf, size);
    for (int y=0; y < tm.size.y; y++)
    {
      for (int x=0; x < tm.size.x; x++)
      {
        const int i = y*tm.size.x+x;
        const char a = buf[i];
        if (! (a > 1)) continue;

        const V2<int> offs [4] =
          { V2<int> {0, -1}
          , V2<int> {-1, 0}
          , V2<int> {1,0}
          , V2<int> {0,1}
          };

        int b = 14;
        for (const auto [ox, oy] : offs)
        {
          const int x2 = x+ox;
          const int y2 = y+oy;
          const int i2 = y2*tm.size.x+x2;
          const int b2 =
            (x2 < 0 || x2 >= tm.size.x || y2 < 0 || y2 >= tm.size.y)
            ? 14
            : tmp[i2];
          if (b > b2) b = b2;
        }
        b++;

        if (! (a > b)) continue;
        buf[i] = b;
      }
    }
  }
  free(tmp);

  for (int y=0; y < tm.size.y; y++)
  {
    for (int x=0; x < tm.size.x; x++)
    {
      const int i = y*tm.size.x+x;
      const char a = buf[i];
      switch (a)
      {
        case 1: buf[i] = (x+y) % 4; break;  // "psuedo-random" selection.
        case 2: buf[i] = (x+y) % 10; break;
        default: break;
      }
    }
  }

  return buf;
}

// ----

struct TileInfo
{
  unsigned char
  tileset, // 0=empty
  tx, ty;  // position of the tile within the tileset

  bool is_nonempty () const
  {
    return (bool) (tileset);
  }
  bool is_empty () const
  {
    return !(is_nonempty ());
  }
  int get_tileset () const
  {
    return tileset - 1;
  }
};

struct TileMapEx : TileMap
{
  V2 <int> pos, size;
  TileInfo * tiles;

  TileMapEx (const TileMap & tm)
  {
    pos  = tm.pos;
    size = tm.size;

    const int sz = size.x * size.y;
    tiles = reinterpret_cast<TileInfo*>(malloc(sizeof(TileInfo) * sz));

    char * flv = flavor(tm);

    for (int y=0; y < tm.size.y; y++)
    {
      for (int x=0; x < tm.size.x; x++)
      {
        const int i = y*tm.size.x+x;
        const auto q = auto_tile (tm, {x, y});
        if (! q.has_value ())
        {
          tiles[i].tileset = 0;
          continue;
        }
        auto [tx, ty] = q.value();
        const auto fl = flv[i];

        tiles[i].tileset = tm.tiles[i];
        tiles[i].tx = (tx != 0) ? tx : fl;
        tiles[i].ty = (tx != 5) ? ty : fl;
      }
    }

    free (flv);
  }

  TileInfo & operator [] (V2<int> pos)
  {
    return tiles [pos.x + pos.y * size.x];
  }
};

// ----


int pop_int (FILE * s)
{
  int n = 0;
  int sign = 1;
  for (;;)
  {
    const char c = fgetc(s);
    if (c == ';') break;
    if (c == '-') { sign *= -1; continue; }
    n = n * 10 + c - '0';
  }
  return n * sign;
}

V2<int> pop_int2 (FILE * s)
{
  return V2 <int>
    { .x = pop_int (s)
    , .y = pop_int (s)
    };
}

TileMap pop_tilemap (FILE * s)
{
  auto pos  = pop_int2 (s);
  auto size = pop_int2 (s);

  // I don't know why I need the +1, is C 0-terminating? What the crap!
  // I ask for N, but it gives me N-1 and a 0..?
  const int sz = size.x * size.y + 1;
  void * tiles = malloc (sz);
  fgets(reinterpret_cast<char *>(tiles), sz, s);
  return TileMap
    { .pos   = pos
    , .size  = size
    , .tiles = reinterpret_cast<tile_t *>(tiles)
    };
}

TileMapEx pop_tilemap_ex (FILE * s)
{
  TileMap tm = pop_tilemap (s);
  TileMapEx tmx (tm);
  free (tm.tiles);
  return tmx;
}

std::vector<TileMapEx> load_tilemaps (const char * pth)
{
  FILE * f = fopen (pth, "r");
  std::vector<TileMapEx> v {};
  for (int i=0; i < 20; i++)
  /*while (! feof(f))*/
  {
    v.push_back (pop_tilemap_ex (f));
    /*fgetc(f);*/
  }
  fclose(f);
  return v;
}

