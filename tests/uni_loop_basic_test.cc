#include <gtest/gtest.h>
#include <unistd.h>

#include <string>

#include "uni_loop/uni_loop.h"

TEST(UniLoopTest, PipeReadEvent) {
  int fds[2];
  ASSERT_EQ(pipe(fds), 0);

  UniLoop::UniLoop loop;
  bool called = false;

  loop.addFd(fds[0], UniLoop::EventType::Read,
             [&](int fd, UniLoop::EventType event_type) {
               char buf[16];
               int n = read(fd, buf, sizeof(buf));
               EXPECT_GT(n, 0);
               std::string msg(buf, n);
               EXPECT_EQ(msg, "hello");
               called = true;
               loop.stop();
             });

  write(fds[1], "hello", 5);

  loop.run();

  EXPECT_TRUE(called);

  close(fds[0]);
  close(fds[1]);
}
