#ifndef OPERATION_INCLUDED
#define OPERATION_INCLUDED
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <iostream>
#include <algorithm>
//EditOperation
enum EditOperation {Insert = 0, Delete = 1};
extern std::string operationstringify[2];
struct edit_operation {
  EditOperation op;
  size_t pos;
  std::string content;
  bool adjacent(size_t o_pos, EditOperation o_op) {
    return (o_op == op) && ((op == EditOperation::Insert && o_pos == pos) || (op == EditOperation::Delete && o_pos == pos-1));
  }
  void show() const {
    std::cout << operationstringify[op] << " " << content << " on " << pos << std::endl;
  }
  const bool operator < (edit_operation& o) const {
    return (pos != o.pos) ? (pos < o.pos) : (op == EditOperation::Delete);
  }
  size_t length() const {return content.length();}
  friend std::ostream& operator << (std::ostream&, const edit_operation&);
  friend std::istream& operator >> (std::istream&, edit_operation&);
};

class OperationList {
  public:
    OperationList();
    void update(edit_operation new_operation);
    std::vector<edit_operation> *diff_list();
    bool empty() const;
    size_t size() const;
    void clear();
    void show() const;
    auto begin() const;
    auto end() const;
    size_t content_length() const;
    friend std::ostream& operator <<(std::ostream& , const OperationList& );
    friend std::istream& operator >> (std::istream&, OperationList&);
  private:
    size_t _content_length;
    std::vector<edit_operation> ops;
};

#endif
