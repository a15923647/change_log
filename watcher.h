#ifndef WATCHER_INCLUDED
#define WATCHER_INCLUDED
#include "event.h"
#include "storage.h"
#include <string>
class Watcher {
  public:
    /*
     * start to monitor file changes
     */
    virtual void start() = 0;
    /*
     * init storage
     * mkdir
     */
    virtual void init() = 0;
  //private:
    //virtual bool match(std::string& filename) const = 0;
    
};
#endif
