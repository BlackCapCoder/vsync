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

// the input lists should have the same length!
bool match (bool x [], Trit pat [])
{
  for (int i = 0; i < sizeof (x); i++)
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


