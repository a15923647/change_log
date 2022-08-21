#ifndef LCS_INCLUDED
#define LCS_INCLUDED

#include <string>
#include <vector>
#include "operation.h"
class LCS {
  public:
    LCS(std::string& s, std::string& t);
    size_t length();
    void lcs(std::string& ret);
    std::vector<edit_operation> *diff_list();
    void show_arr();
    OperationList op_list;
  private:
    void construct_arr();
    std::vector<std::vector<size_t>> common_len;
    std::string s;
    std::string t;
};

#endif
