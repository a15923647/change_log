#include "operation.h"
char operation_notation[2] = {'>', '<'};
std::string operationstringify[2] = {"add", "delete"};
std::ostream& operator << (std::ostream& output_stream, const edit_operation& eop) {
  output_stream << eop.pos << operationstringify[eop.op][0] << eop.content.length() << std::endl;
  output_stream << operation_notation[eop.op] << " " << eop.content << std::endl;
  return output_stream;
}
static size_t get_num(std::istream& input_stream) {
  size_t ret = 0;
  while (input_stream.peek() >= '0' && input_stream.peek() <= '9') {
    ret *= 10;
    ret += input_stream.get() - '0';
  }
  return ret;
}
std::istream& operator >> (std::istream& input_stream, edit_operation& eop) {
  eop.pos = get_num(input_stream);
  char prefix = input_stream.get();
  if (prefix == operationstringify[EditOperation::Insert][0]) {
    eop.op = EditOperation::Insert;
  } else if (prefix == operationstringify[EditOperation::Delete][0]) {
    eop.op = EditOperation::Delete;
  } else {
    std::cerr << "malform input, see " << (int)prefix << std::endl;
    throw "malform input";
  }
  const size_t len = get_num(input_stream);
  if (input_stream.peek() == '\n') input_stream.get();
  if (input_stream.get() != operation_notation[eop.op]) {
    throw "notation mismatch";
  }
  input_stream.get();//eat " "
  char buf[len+1];
  for (size_t i = 0; i < len; i++)
    buf[i] = input_stream.get();
  buf[len] = '\0';
  eop.content = std::string(buf);
  input_stream.get();//eat std::endl
  return input_stream;
}

void OperationList::show() const {
  for (const auto& eop : ops)
    eop.show();
}

std::vector<edit_operation> *OperationList::diff_list() {
  std::vector<edit_operation> *dup = new std::vector<edit_operation>(ops);
  std::reverse(dup->begin(), dup->end());
  return dup;
}

void OperationList::update(edit_operation new_operation) {
  this->_content_length += new_operation.length();
  if (!ops.empty() && ops.back().adjacent(new_operation.pos, new_operation.op)) {
    ops.back().content = new_operation.content + ops.back().content;
    ops.back().pos = new_operation.pos;
  } else {
    ops.push_back(new_operation);
  }
}

OperationList::OperationList() {this->clear();}
bool OperationList::empty() const {return ops.empty();}
size_t OperationList::size() const {return this->ops.size();}
size_t OperationList::content_length() const {return this->_content_length;}
void OperationList::clear() {this->ops.clear(); this->_content_length = 0;}
auto OperationList::begin() const {return this->ops.begin();}
auto OperationList::end() const {return this->ops.end();}

std::ostream& operator << (std::ostream& output_stream, const OperationList& o_list) {
  if (o_list.empty()) return output_stream;
  output_stream << *(o_list.end()-1);
  for (auto it = o_list.end()-2; it >= o_list.begin(); it--) {
    output_stream << "---\n" << *it;
  }
  return output_stream;
}

std::istream& operator >> (std::istream& input_stream, OperationList& o_list) {
  std::vector<edit_operation> buf;
  edit_operation tmp;
  if (input_stream.peek() != EOF) {
    input_stream >> tmp;
    buf.push_back(tmp);
  }
  while (input_stream.peek() != EOF) {
    for (int i = 0; i < 4; i++) input_stream.get();
    input_stream >> tmp;
    buf.push_back(tmp);
  }
  reverse(buf.begin(), buf.end());
  for (auto& eop : buf) {
    o_list.update(eop);
  }
  return input_stream;
}

