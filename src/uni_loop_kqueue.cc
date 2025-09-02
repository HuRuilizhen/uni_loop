#include <sys/event.h>
#include <unistd.h>

#include <stdexcept>

#include "uni_loop/uni_loop.h"

namespace UniLoop {

UniLoop::UniLoop(int max_events, timespec timeout)
    : max_events_(max_events), timeout_(timeout) {
  backend_file_desc_ = kqueue();
  if (backend_file_desc_ == -1) {
    throw std::runtime_error("Failed to create kqueue");
  }
  running_ = false;
}

UniLoop::~UniLoop() { close(backend_file_desc_); }

void UniLoop::addEvent(const Event& event, EventCallback callback) {
  int event_ident = next_event_ident_++;

  struct kevent ke;
  short filter;

  switch (event.type) {
    case EventType::Read:
      filter = EVFILT_READ;
      EV_SET(&ke, event.ident, filter, EV_ADD | EV_ENABLE, 0, 0,
             (void*)(intptr_t)event_ident);
      break;
    case EventType::Timer:
      filter = EVFILT_TIMER;
      EV_SET(&ke, event.ident, filter, EV_ADD | EV_ENABLE, 0, event.interval_ms,
             (void*)(intptr_t)event_ident);
      break;
    default:
      filter = EVFILT_WRITE;
      EV_SET(&ke, event.ident, filter, EV_ADD | EV_ENABLE, 0, 0,
             (void*)(intptr_t)event_ident);
  }

  if (kevent(backend_file_desc_, &ke, 1, nullptr, 0, nullptr) == -1) {
    throw std::runtime_error("kevent addFd failed");
  }

  event_ids_[event] = event_ident;
  id_events_[event_ident] = event;
  callbacks_[event_ident] = callback;
}

void UniLoop::modEvent(const Event& event, EventCallback callback) {
  delEvent(event);
  addEvent(event, callback);
}

void UniLoop::delEvent(const Event& event) {
  auto it = event_ids_.find(event);
  if (it == event_ids_.end()) return;

  struct kevent ke;
  short filter;
  switch (event.type) {
    case EventType::Read:
      filter = EVFILT_READ;
      break;
    case EventType::Timer:
      filter = EVFILT_TIMER;
      break;
    default:
      filter = EVFILT_WRITE;
  }

  EV_SET(&ke, event.ident, filter, EV_DELETE, 0, 0,
         (void*)(intptr_t)it->second);

  kevent(backend_file_desc_, &ke, 1, nullptr, 0, nullptr);

  event_ids_.erase(it);
  id_events_.erase(it->second);
  callbacks_.erase(it->second);
}

void UniLoop::run() {
  running_ = true;

  std::vector<struct kevent> events(max_events_);

  while (running_) {
    int n = kevent(backend_file_desc_, nullptr, 0, events.data(), max_events_,
                   &timeout_);
    if (n < 0) {
      perror("kevent wait error");
      break;
    }
    for (int i = 0; i < n; i++) {
      int event_ident = (intptr_t)events[i].udata;
      auto callback_it = callbacks_.find(event_ident);
      if (callbacks_.find(event_ident) == callbacks_.end()) continue;
      auto id_events_it = id_events_.find(event_ident);
      if (id_events_.find(event_ident) == id_events_.end()) continue;
      callback_it->second(id_events_it->second);
    }
  }
}

void UniLoop::asyncRun() {
  if (running_.load()) return;
  running_ = true;

  worker_thread_ = std::thread([this] { UniLoop::run(); });
}

void UniLoop::wait() {
  if (worker_thread_.joinable()) worker_thread_.join();
}

void UniLoop::stop() { running_ = false; }

}  // namespace UniLoop
