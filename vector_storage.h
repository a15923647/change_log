#ifndef VECTOR_STORAGE_INCLUDED
#define VECTOR_STORAGE_INCLUDED
#include <vector>
#include <queue>
#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
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
  friend std::ostream& operator << (std::ostream& output_stream, const Node& node) {
    output_stream << node.ev_type << " " << node.trigger_time << " " << node.path << std::endl;
    output_stream << node.op_list << std::endl;
    return output_stream;
  }
  friend std::istream& operator >> (std::istream& input_stream, Node& node) {
    int ev_num;
    input_stream >> ev_num >> node.trigger_time;
    node.ev_type = num2ev_type[ev_num];
    input_stream.get();//eat " "
    getline(input_stream, node.path);
    input_stream.get();//eat "\n"
    input_stream >> node.op_list;
    return input_stream;
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
