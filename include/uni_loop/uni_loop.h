#pragma once
#include <functional>
#include <unordered_map>

namespace UniLoop {

enum class EventType { Read, Write, Error, Close };

using EventCallback = std::function<void(int file_desc, EventType)>;

class UniLoop {
 public:
  UniLoop(int max_events = 64);
  ~UniLoop();

  void addFd(int file_desc, EventType event_type, EventCallback callback);
  void modFd(int file_desc, EventType event_type);
  void delFd(int file_desc);

  void run();
  void stop();

 private:
  int max_events_;
  int backend_file_desc_;
  bool running_;

  std::unordered_map<int, EventCallback> callbacks_;
  std::unordered_map<int, EventType> interests_;
};

}  // namespace UniLoop
