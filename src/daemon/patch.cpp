#include "patch.h"

static inline void insert(std::vector<edit_operation>& opv, size_t& idx, std::string& res) {
  res += opv[idx].content;
}

static inline void skip(size_t& x_idx, std::vector<edit_operation>& opv, size_t& idx) {
  x_idx += opv[idx].content.length();
}

void Patch::patch(std::string& x, std::vector<edit_operation>& opv, std::string& res) {
  size_t idx = 0;
  for (size_t x_idx = 0; idx < opv.size() || x_idx < x.length(); x_idx++) {
    while (idx < opv.size() && x_idx == opv[idx].pos) {
      //opv[idx].show();
      if (opv[idx].op == EditOperation::Insert) {
        insert(opv, idx, res);
      } else if (opv[idx].op == EditOperation::Delete) { //Delete
        skip(x_idx, opv, idx);
      }
      idx++;
    }
    if (x_idx < x.length()) res += x[x_idx];
  }
}

void Patch::patchBack(std::string& y, std::vector<edit_operation>& opv, std::string& res) {
  long long old_idx_offset = 0;
  for (auto& e : opv) {
    if (e.op == EditOperation::Delete) {
      e.op = EditOperation::Insert;
      e.pos += old_idx_offset;
      old_idx_offset -= e.content.length();
    } else {
      e.op = EditOperation::Delete;
      e.pos += old_idx_offset;
      old_idx_offset += e.content.length();
    }
  }
  this->patch(y, opv, res);
}
