#ifndef EVENT_INCLUDED
#define EVENT_INCLUDED
#include <string>
#include <time.h>
enum EventType {MODIFY = 0, CREATE = 1, DELETE = 2};
struct Event {
  EventType ev_type;
  time_t trigger_time;
  std::string path;
  Event() {}
  Event(EventType ev_type, std::string& path) {
    this->ev_type = ev_type;
    this->path = path;
    time(&(this->trigger_time));
  }
};
#endif
