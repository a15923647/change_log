#include "inotify_watcher.h"
#include "sqlite_storage.h"
#include "config.h"
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <thread>

using namespace std;

void qthread() {
  string sql;
  while (1) {
    time_t now; time(&now);
    struct tm * yesterday = localtime(&now);
    yesterday->tm_hour = yesterday->tm_min = yesterday->tm_sec = 0;
    yesterday->tm_mday--;
    
    string dump_name = to_string(yesterday->tm_year + 1900) + "_" + to_string(yesterday->tm_mon+1) + "_" + to_string(yesterday->tm_mday) + ".csv";
    fs::path dump_path = fs::path(config::temp_dir) / fs::path(dump_name);

    sql = "sqlite3 -header -csv " + config::db_path + " 'select * from history_opl join history_meta on history_meta.id = history_opl.id where trigger_time >= " + to_string(mktime(yesterday)) + ";' > " + dump_path.u8string();
    system(sql.c_str());
    //SQLiteStorage::query(sql);
    yesterday->tm_mday += 2;//tomorrow
    double seconds = difftime(mktime(yesterday), time(NULL));
    cout << "sleep " << seconds << " seconds\n";
    sleep(seconds);
  }
}

int main(int argc, const char* argv[]) {
  if (argc < 2) {
    cerr << "usage: " << argv[0] << " config.json_path\n";
    exit(1);
  }
  string config_path(argv[1]);
  cout << "init config from " + config_path << endl;
  config::load_from_json(config_path);
  //SQLiteStorage *sql_st_p = new SQLiteStorage(config::db_path);
  SQLiteStorage *sql_st_p = new SQLiteStorage();//just ensure config is loaded
  thread t(qthread);
  t.detach();
  //InotifyWatcher watchd(config_path);
  InotifyWatcher watchd;//config must be loaded first
  watchd.start();
}
