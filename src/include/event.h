#ifndef EVENT_INCLUDED
#define EVENT_INCLUDED
#include <string>
#include <time.h>
enum EventType {MODIFY = 0, CREATE = 1, DELETE = 2};
const EventType num2ev_type[3] = {EventType::MODIFY, EventType::CREATE, EventType::DELETE};
const std::string evTypeStringify[3] = {
  "modify", "create", "delete"
};
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
