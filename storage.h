#ifndef STORAGE_INCLUDED
#define STORAGE_INCLUDED
#include "event.h"
#include <queue>
class Storage {
  public:
    /* 
     * Write to somewhat database
     * Compute diff
     * Update the temp file
     * Update hash in File
     */
    virtual void update(Event&) = 0;
    /*
     * Query for something
     */
    virtual void *query(std::string& query) = 0;
};
#endif
