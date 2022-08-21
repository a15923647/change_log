#ifndef PATCH_INCLUDED
#define PATCH_INCLUDED
#include <string>
#include <vector>
#include "operation.h"
class Patch {
  public:
    void patch(std::string& x, std::vector<edit_operation>& opv, std::string& res);
    void patchBack(std::string& y, std::vector<edit_operation>& opv, std::string& res);
};

#endif
