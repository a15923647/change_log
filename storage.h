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
    static void update(Event&);
    /*
     * Query for something
     */
    static void *query(std::string& query);
};
#endif
