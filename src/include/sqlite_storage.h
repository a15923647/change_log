#ifndef SQLITE_STORAGE_INCLUDED
#define SQLITE_STORAGE_INCLUDED
#include <vector>
#include <string>
#include <sstream>
#include <sqlite3.h>
#include <iostream>
#include <unistd.h>
#include <mutex>
#include "event.h"
#include "encoding.h"
#include "storage.h"
#include "lcs.h"
#include "file.h"
extern std::string db_loc;
extern sqlite3* db;
/*
 * sqlite will complain use too many memory if we have multiple query at the same time.
*/
extern std::mutex *db_mutex;
class SQLiteStorage : public Storage {
  public:
    SQLiteStorage(std::string& db_path);
    static void update(Event);
    static void *query(std::string& query);
  private:
    static const std::vector<std::string> create_table_queries;
};
struct file_meta_entry {
  std::string hash;
  std::string path;
  file_meta_entry(sqlite3_stmt *stmt) {
    hash = std::string((const char *)sqlite3_column_text(stmt, 0));
    path = std::string((const char *)sqlite3_column_text(stmt, 1));
  }
};
struct file_content_entry {
  std::string path;
  std::string content;
  file_content_entry(sqlite3_stmt *stmt) {
    path = std::string((const char *)sqlite3_column_text(stmt, 0));
    content = std::string((const char *)sqlite3_column_text(stmt, 1));
  }
};
struct history_meta_entry {
  std::string path;
  int ev_type;
  int trigger_time;
  int id;
  history_meta_entry(sqlite3_stmt *stmt) {
    path = std::string((const char *)sqlite3_column_text(stmt, 0));
    ev_type = sqlite3_column_int(stmt, 1);
    trigger_time = sqlite3_column_int(stmt, 2);
    id = sqlite3_column_int(stmt, 3);
  }
};
struct history_opl_entry {
  std::string path;
  std::string new_hash;
  std::string opl_dump;
  int id;
  history_opl_entry(sqlite3_stmt *stmt) {
    path = std::string((const char *)sqlite3_column_text(stmt, 0));
    new_hash = std::string((const char *)sqlite3_column_text(stmt, 1));
    opl_dump = std::string((const char *)sqlite3_column_text(stmt, 2));
    id = sqlite3_column_int(stmt, 3);
  }
};

struct history_entry {
  std::string path;
  int ev_type;
  int trigger_time;
  int id;
  std::string new_hash;
  std::string opl_dump;
  //select * from history_meta join history_opl on history_meta.id = history_opl.id;
  history_entry(sqlite3_stmt *stmt) {
    path = std::string((const char *)sqlite3_column_text(stmt, 0));
    ev_type = sqlite3_column_int(stmt, 1);
    trigger_time = sqlite3_column_int(stmt, 2);
    id = sqlite3_column_int(stmt, 3);
    //path at 4 again
    new_hash = std::string((const char *)sqlite3_column_text(stmt, 5));
    opl_dump = std::string((const char *)sqlite3_column_text(stmt, 6));
  }
};
#endif
