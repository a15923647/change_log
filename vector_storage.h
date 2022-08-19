#ifndef VECTOR_STORAGE_INCLUDED
#define VECTOR_STORAGE_INCLUDED
#include <vector>
#include <queue>
#include <map>
#include <sstream>
#include "operation.h"
#include "storage.h"
#include "event.h"
#include "file.h"
struct Node : Event {
  OperationList op_list;
  Node(time_t t) {trigger_time = t;}
  Node(Event& ev, OperationList& op_list) : Event(ev), op_list(op_list) {}
  const bool operator < (const Node& o) const {
    return trigger_time < o.trigger_time;
  }
};

class VectorStorage : public Storage {
  public:
    static void update(Event);
    static void *query(std::string& query);
  //protected:
    //static std::vector<Node> record;
    //static std::map<std::string, std::map<std::string, File>> watch_struct;
    
};
extern std::vector<Node> record;
extern std::map<std::string, std::map<std::string, File>> watch_struct;
#endif
