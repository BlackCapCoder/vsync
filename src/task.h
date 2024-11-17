#pragma once

#include <mutex>
#include <iostream>


struct Task
{
  std::mutex m1 {};
  std::mutex m2 {};

  void foo ()
  {
    m2.lock();
    /*m2.lock();*/
  }
};

void testTask ()
{
  Task t {};
  t.foo();
}
