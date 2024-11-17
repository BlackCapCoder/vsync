#pragma once

#include <cstdio>
#include <cstdlib>
#include "V2.h"
#include <optional>
#include <iostream>
#include <vector>

// ----

using tile_t =
  unsigned char;

struct TileMap
{
  V2 <int> pos;
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

TileMap pop_tilemap (FILE * s)
{
  int x = pop_int (s);
  int y = pop_int (s);
  int w = pop_int (s);
  int h = pop_int (s);

  // I don't know why I need the +1, is C 0-terminating? What the crap!
  void * tiles = malloc (w*h+1);
  fgets(reinterpret_cast<char *>(tiles), w*h+1, s);
  return TileMap
    { .pos   = {x, y}
    , .size  = {w, h}
    , .tiles = reinterpret_cast<tile_t *>(tiles)
    };
}

std::vector<TileMap> load_tilemaps (const char * pth)
{
  FILE * f = fopen (pth, "r");
  std::vector<TileMap> v {};
  for (int i=0; i < 20; i++)
  /*while (! feof(f))*/
  {
    v.push_back (pop_tilemap (f));
    /*fgetc(f);*/
  }
  fclose(f);
  return v;
}

