#pragma once
#include <functional>
#include <thread>
#include <unordered_map>

namespace UniLoop {

enum class EventType { Read, Write, Error, Close };

using EventCallback = std::function<void(int file_desc, EventType)>;

class UniLoop {
 public:
  UniLoop(int max_events = 64, timespec timeout_ = {1, 0});
  ~UniLoop();

  void addFd(int file_desc, EventType event_type, EventCallback callback);
  void modFd(int file_desc, EventType event_type);
  void delFd(int file_desc);

  void run();
  void asyncRun();
  void stop();
  void wait();

 private:
  int max_events_;
  int backend_file_desc_;
  timespec timeout_;
  std::atomic<bool> running_;
  std::thread worker_thread_;

  std::unordered_map<int, EventCallback> callbacks_;
  std::unordered_map<int, EventType> interests_;
};

}  // namespace UniLoop
