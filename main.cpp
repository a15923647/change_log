#include "inotify_watcher.h"
#include "vector_storage.h"
using namespace std;

int main(void) {
  VectorStorage *vstorage_p = new VectorStorage();
  InotifyWatcher watchd(vstorage_p);
  watchd.start();
  return 0;
}
