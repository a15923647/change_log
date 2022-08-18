#ifndef WATCHER_INCLUDED
#define WATCHER_INCLUDED
#include "event.h"
class Watcher {
  public:
    /*
     * start to monitor file changes
     */
    virtual void start();
  private:
    virtual bool match(std::string& ) const;
};
#endif
