# uni_loop

A lightweight, cross-platform cpp event loop library based on **epoll (Linux)** and **kqueue (macOS)**. Future support for **IOCP (Windows)** is planned.

## Features
- Unified `EventLoop` API across platforms
- Support for Read / Write / Error / Close events
- Timer support (via kqueue EVFILT_TIMER / Linux timerfd planned)
- Minimal and dependency-free (only system headers + C++ STL)
- Examples: TCP echo server, echo client, timer demo
- Unit tests with GoogleTest
