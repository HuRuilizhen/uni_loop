#include <gtest/gtest.h>
#include <unistd.h>

#include "uni_loop/uni_loop.h"

TEST(UniLoopTimerTest, SingleShotTimer) {
  UniLoop::UniLoop loop;
  bool fired = false;
  auto start = std::chrono::steady_clock::now();

  UniLoop::Event timer{UniLoop::EventType::Timer, 1, 100};

  loop.addEvent(timer, [&](const UniLoop::Event& ev) {
    EXPECT_EQ(ev.type, UniLoop::EventType::Timer);
    auto end = std::chrono::steady_clock::now();
    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();
    EXPECT_GE(elapsed, 100);
    fired = true;
    loop.stop();
  });

  loop.run();

  EXPECT_TRUE(fired);
}

TEST(UniLoopTimerTest, PeriodicTimer) {
  UniLoop::UniLoop loop;
  int count = 0;

  UniLoop::Event timer{UniLoop::EventType::Timer, 1, 50};

  loop.addEvent(timer, [&](const UniLoop::Event& ev) {
    EXPECT_EQ(ev.type, UniLoop::EventType::Timer);
    count++;
    if (count >= 3) {
      loop.stop();
    }
  });

  loop.run();

  EXPECT_EQ(count, 3);
}
