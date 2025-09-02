#include <gtest/gtest.h>
#include <unistd.h>

#include "uni_loop/uni_loop.h"

TEST(UniLoopBasicTest, MultiFileDescTriggered) {
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

TEST(UniLoopBasicTest, AsyncRunLoop) {
  UniLoop::UniLoop loop;

  loop.asyncRun();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  loop.stop();
  loop.wait();

  SUCCEED();
}
