#include <sys/event.h>
#include <unistd.h>

#include <stdexcept>

#include "uni_loop/uni_loop.h"

namespace UniLoop {

UniLoop::UniLoop(int max_events) : max_events_(max_events) {
  backend_file_desc_ = kqueue();
  if (backend_file_desc_ == -1) {
    throw std::runtime_error("Failed to create kqueue");
  }
  running_ = false;
}

UniLoop::~UniLoop() { close(backend_file_desc_); }

void UniLoop::addFd(int file_desc, EventType event_type,
                    EventCallback callback) {
  struct kevent ke;
  short filter = (event_type == EventType::Read) ? EVFILT_READ : EVFILT_WRITE;
  EV_SET(&ke, file_desc, filter, EV_ADD | EV_ENABLE, 0, 0, NULL);

  if (kevent(backend_file_desc_, &ke, 1, nullptr, 0, nullptr) == -1) {
    throw std::runtime_error("kevent addFd failed");
  }
  callbacks_[file_desc] = callback;
  interests_[file_desc] = event_type;
}

void UniLoop::modFd(int file_desc, EventType event_type) {
  delFd(file_desc);
  addFd(file_desc, event_type, callbacks_[file_desc]);
}

void UniLoop::delFd(int file_desc) {
  auto it = interests_.find(file_desc);
  if (it == interests_.end()) return;

  struct kevent ke;
  short filter = (it->second == EventType::Read) ? EVFILT_READ : EVFILT_WRITE;
  EV_SET(&ke, file_desc, filter, EV_DELETE, 0, 0, NULL);

  kevent(backend_file_desc_, &ke, 1, nullptr, 0, nullptr);
  callbacks_.erase(file_desc);
  interests_.erase(file_desc);
}

void UniLoop::run() {
  running_ = true;
  std::vector<struct kevent> events(max_events_);

  while (running_) {
    int n = kevent(backend_file_desc_, nullptr, 0, events.data(), max_events_,
                   nullptr);
    if (n < 0) {
      perror("kevent wait error");
      break;
    }
    for (int i = 0; i < n; i++) {
      int file_desc = static_cast<int>(events[i].ident);
      auto callback_it = callbacks_.find(file_desc);
      if (callback_it == callbacks_.end()) continue;

      EventType ev = interests_[file_desc];
      if (events[i].flags & EV_EOF) {
        callback_it->second(file_desc, EventType::Close);
      } else if (events[i].flags & EV_ERROR) {
        callback_it->second(file_desc, EventType::Error);
      } else {
        callback_it->second(file_desc, ev);
      }
    }
  }
}

void UniLoop::stop() { running_ = false; }

}  // namespace UniLoop
