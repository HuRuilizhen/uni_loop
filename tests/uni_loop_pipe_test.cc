#include <gtest/gtest.h>
#include <unistd.h>

#include <string>

#include "uni_loop/uni_loop.h"

TEST(UniLoopTest, PipeReadEvent) {
  int file_descs[2];
  ASSERT_EQ(pipe(file_descs), 0);

  UniLoop::UniLoop loop;
  bool called = false;

  loop.addFd(file_descs[0], UniLoop::EventType::Read,
             [&](int file_desc, UniLoop::EventType event_type) {
               char buf[16];
               int n = read(file_desc, buf, sizeof(buf));
               EXPECT_GT(n, 0);
               std::string msg(buf, n);
               EXPECT_NE(msg.find("hello"), std::string::npos);
               called = true;
               loop.stop();
             });

  write(file_descs[1], "hello", sizeof("hello"));

  loop.run();

  EXPECT_TRUE(called);

  close(file_descs[0]);
  close(file_descs[1]);
}

TEST(UniLoopPipeTest, PipeWriteEvent) {
  int file_descs[2];
  ASSERT_EQ(pipe(file_descs), 0);

  UniLoop::UniLoop loop;
  bool called = false;

  loop.addFd(file_descs[1], UniLoop::EventType::Write,
             [&](int file_desc, UniLoop::EventType event_type) {
               EXPECT_EQ(event_type, UniLoop::EventType::Write);
               called = true;
               loop.stop();
             });

  loop.run();

  EXPECT_TRUE(called);

  close(file_descs[0]);
  close(file_descs[1]);
}

TEST(UniLoopPipeTest, PipeCloseEvent) {
  int file_descs[2];
  ASSERT_EQ(pipe(file_descs), 0);

  UniLoop::UniLoop loop;
  bool called = false;

  loop.addFd(file_descs[0], UniLoop::EventType::Close,
             [&](int file_desc, UniLoop::EventType event_type) {
               EXPECT_EQ(event_type, UniLoop::EventType::Close);
               called = true;
               loop.stop();
             });

  close(file_descs[1]);

  loop.run();

  close(file_descs[0]);

  EXPECT_TRUE(called);
}
