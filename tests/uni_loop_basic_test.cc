#include <gtest/gtest.h>
#include <unistd.h>

#include "uni_loop/uni_loop.h"

TEST(UniLoopBasicTest, AsyncRunLoop) {
  UniLoop::UniLoop loop;

  loop.asyncRun();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  loop.stop();
  loop.wait();

  SUCCEED();
}
