#pragma once

#include "V2.h"
#include <vector>

// ----

struct RoomMeta
{
  std::vector <V2 <int>> respawn_points;
};

std::vector <RoomMeta> make_room_metas ()
{
  std::vector <RoomMeta> vec (20, RoomMeta {});

  vec[ 0].respawn_points = {V2<int>{32, 5}, V2<int>{2, 16}};
  vec[ 1].respawn_points = {V2<int>{34, 13}, V2<int>{7, 18}};
  vec[ 2].respawn_points = {V2<int>{32, 6}, V2<int>{6, 19}};
  vec[ 3].respawn_points = {V2<int>{7, 18}};
  vec[ 4].respawn_points = {V2<int>{24, 3}, V2<int>{8, 19}};
  vec[ 5].respawn_points = {V2<int>{39, 32}, V2<int>{7, 32}};
  vec[ 6].respawn_points = {V2<int>{5, 19}, V2<int>{3, 4}, V2<int>{38, 8}};
  vec[ 7].respawn_points = {V2<int>{1, 8}, V2<int>{31, 6}};
  vec[ 8].respawn_points = {V2<int>{21, 28}};
  vec[ 9].respawn_points = {V2<int>{0, 7}};
  vec[10].respawn_points = {V2<int>{4, 10},V2<int>{37, 9}};
  vec[11].respawn_points = {V2<int>{28, 10},V2<int>{5, 7}};
  vec[12].respawn_points = {V2<int>{39, 19},V2<int>{7, 6}};
  vec[13].respawn_points = {V2<int>{1, 10},V2<int>{1, 7},V2<int>{36, 8}};
  vec[14].respawn_points = {V2<int>{1, 17},V2<int>{1, 3},V2<int>{33, 3}};
  vec[15].respawn_points = {V2<int>{5, 22}};
  vec[16].respawn_points = {V2<int>{5, 17}, V2<int>{37, 4}};
  vec[17].respawn_points = {V2<int>{0, 10}, V2<int>{34, 2}};
  vec[18].respawn_points = {V2<int>{8, 14}, V2<int>{31, 4}};
  vec[19].respawn_points = {V2<int>{1, 17}, V2<int>{5, 0}, V2<int>{38, 6}};

  return vec;
}

