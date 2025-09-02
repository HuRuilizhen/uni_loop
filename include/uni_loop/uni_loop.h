#pragma once
#include <functional>
#include <thread>
#include <unordered_map>

namespace UniLoop {

enum class EventType { Read, Write, Error, Close, Timer };

struct Event {
  EventType type;
  int ident = -1;       // fd / timerfd / signalfd
  int interval_ms = 0;  // Timer

  bool operator==(const Event& other) const {
    return type == other.type && ident == other.ident;
  }
};

struct EventHash {
  std::size_t operator()(const Event& event) const {
    return std::hash<int>()(event.ident) << 16 |
           std::hash<int>()(static_cast<int>(event.type));
  }
};

using EventCallback = std::function<void(Event event)>;

class UniLoop {
 public:
  UniLoop(int max_events = 64, timespec timeout_ = {1, 0});
  ~UniLoop();

  void addEvent(const Event& event, EventCallback callback);
  void modEvent(const Event& event, EventCallback callback);
  void delEvent(const Event& event);

  void run();
  void asyncRun();
  void stop();
  void wait();

 private:
  std::atomic<int> next_event_ident_{1};

  int max_events_;
  int backend_file_desc_;
  timespec timeout_;

  std::atomic<bool> running_;
  std::thread worker_thread_;

  std::unordered_map<int, EventCallback> callbacks_;
  std::unordered_map<Event, int, EventHash> event_ids_;
  std::unordered_map<int, Event> id_events_;
};

}  // namespace UniLoop
