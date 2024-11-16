#pragma once

#include <mutex>


struct Task
{
  std::mutex m1 {};
  std::mutex m2 {};
};

void testTask ()
{

}
