#pragma once
#include <fmt/core.h>

template <class T>
union V2
{
  struct { T x, y; };
  struct { T w, h; };
  T vals [2];

  constexpr V2 () : x{}, y{} {};
  constexpr V2 (T x, T y) : x {x}, y {y} {}
  constexpr V2 (T a) : x {a}, y {a} {}

  constexpr V2 operator + (const V2 that) const { return V2 { x + that.x, y + that.y }; }
  constexpr V2 operator - (const V2 that) const { return V2 { x - that.x, y - that.y }; }
  constexpr V2 operator * (const V2 that) const { return V2 { x * that.x, y * that.y }; }
  constexpr V2 operator / (const V2 that) const { return V2 { x / that.x, y / that.y }; }

  void operator += (V2 that) { x += that.x; y += that.y; }
  void operator -= (V2 that) { x -= that.x; y -= that.y; }
  void operator *= (V2 that) { x *= that.x; y *= that.y; }
  void operator /= (V2 that) { x /= that.x; y /= that.y; }
};

template <class T>
union V4
{
  struct { T x, y, w, h; };
  struct { V2 <T> pos, size; };
  T vals [4];
  V2 <V2 <T>> v2v2;

  constexpr V4 operator + (V4 that) { return V4 { v2v2 + that.v2v2 }; }
  constexpr V4 operator - (V4 that) { return V4 { v2v2 - that.v2v2 }; }
  constexpr V4 operator * (V4 that) { return V4 { v2v2 * that.v2v2 }; }
  constexpr V4 operator / (V4 that) { return V4 { v2v2 / that.v2v2 }; }

  void operator += (V4 that) { v2v2 += that.v2v2; }
  void operator -= (V4 that) { v2v2 -= that.v2v2; }
  void operator *= (V4 that) { v2v2 *= that.v2v2; }
  void operator /= (V4 that) { v2v2 /= that.v2v2; }
};

