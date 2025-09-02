#include <gtest/gtest.h>
#include <unistd.h>

#include <string>

#include "uni_loop/uni_loop.h"

TEST(UniLoopPipeTest, PipeReadEvent) {
  int file_descs[2];
  ASSERT_EQ(pipe(file_descs), 0);

  UniLoop::UniLoop loop;
  bool called = false;

  UniLoop::Event event;
  event.ident = file_descs[0];
  event.type = UniLoop::EventType::Read;

  loop.addEvent(event, [&](UniLoop::Event event) {
    char buf[16];
    int n = read(event.ident, buf, sizeof(buf));
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

  UniLoop::Event event;
  event.ident = file_descs[1];
  event.type = UniLoop::EventType::Write;

  loop.addEvent(event, [&](UniLoop::Event event) {
    EXPECT_EQ(event.type, UniLoop::EventType::Write);
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

  UniLoop::Event event;
  event.ident = file_descs[0];
  event.type = UniLoop::EventType::Close;

  loop.addEvent(event, [&](UniLoop::Event event) {
    EXPECT_EQ(event.type, UniLoop::EventType::Close);
    called = true;
    loop.stop();
  });

  close(file_descs[1]);

  loop.run();

  close(file_descs[0]);

  EXPECT_TRUE(called);
}

TEST(UniLoopPipeTest, MultiFileDescTriggered) {
  int file_desc_1[2], file_desc_2[2];
  ASSERT_EQ(pipe(file_desc_1), 0);
  ASSERT_EQ(pipe(file_desc_2), 0);

  UniLoop::UniLoop loop;
  int count = 0;

  UniLoop::Event event_1;
  event_1.ident = file_desc_1[0];
  event_1.type = UniLoop::EventType::Read;
  UniLoop::Event event_2;
  event_2.ident = file_desc_2[0];
  event_2.type = UniLoop::EventType::Read;

  loop.addEvent(event_1, [&](UniLoop::Event event) {
    char buf[4];
    read(event_1.ident, buf, sizeof(buf));
    count++;
    if (count == 2) loop.stop();
  });
  loop.addEvent(event_2, [&](UniLoop::Event event) {
    char buf[4];
    read(event_2.ident, buf, sizeof(buf));
    count++;
    if (count == 2) loop.stop();
  });

  write(file_desc_1[1], "a", 1);
  write(file_desc_2[1], "b", 1);

  loop.run();
  EXPECT_EQ(count, 2);

  close(file_desc_1[0]);
  close(file_desc_2[1]);
  close(file_desc_1[0]);
  close(file_desc_2[1]);
}
