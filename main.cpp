#include "inotify_watcher.h"
#include "vector_storage.h"
#include <thread>
#include <unistd.h>
#include <iostream>
using namespace std;

void qthread() {
  while (1) {
    puts("querying...");
      string q("1 day");
      std::vector<Node> *res = (std::vector<Node> *)VectorStorage::query(q);
      cout << res->size() << endl;
      for (Node node : *res) {
        cout << node << endl;
      }
      delete res;
      time_t now; time(&now);
      struct tm * timeinfo = localtime (&now);
      timeinfo->tm_hour = timeinfo->tm_min = timeinfo->tm_sec = 0;
      timeinfo->tm_mday++;
      double seconds = difftime(mktime(timeinfo), time(NULL));
      cout << "sleep " << seconds << " seconds\n";
      //sleep(seconds);
      sleep(10);
  }
}

int main(void) {
  VectorStorage *vstorage_p = new VectorStorage();
  thread t(qthread);
  t.detach();
  InotifyWatcher watchd(vstorage_p);
  watchd.start();
  return 0;
}
