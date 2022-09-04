#include <sqlite3.h>
#include <iostream>
#include <vector>
#include <string>
#include "sqlite_storage.h"
using namespace std;
string double_single_quote(string& s) {
  string res;
  for (char c : s) {
    if (c == '\'') res += '\'';
    res += c;
  }
  return res;
}

bool send_query(string& db_loc, string& sql, void (*handler)(sqlite3_stmt *, void *ret_struct), void *ret_struct) {
  sqlite3 *db;
  int rc = sqlite3_open_v2(db_loc.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE, NULL);
  if (rc) {
    cerr << "fail to open sqlite db at " + db_loc + "\n";
    exit(1);
  }
  //cout << "open db successfully\n";
  
  sqlite3_busy_timeout(db, 10000);
  
  sqlite3_stmt *stmt = NULL;
  //cout << "send query: " << sql << endl;
  int chk = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
  if (chk != SQLITE_OK) {
    cerr << "sql error, sql statement: " + sql + "\n" << sqlite3_errmsg(db) << endl;
    exit(2);
  }
  //cout << "run sql: " + sql + " successfully\n";
  bool ret = false;
  rc = sqlite3_step(stmt);
  while (rc == SQLITE_ROW) {
    handler(stmt, ret_struct);
    ret = true;
    rc = sqlite3_step(stmt);
  }

  if (rc != SQLITE_DONE) {
    cerr << "db rejects: " + sql + " " << sqlite3_errmsg(db) << endl;
    cerr << "return code: " << rc << endl;
    cerr << "ret: " << ret << endl;
    exit(2);
  }
  sqlite3_finalize(stmt);
  if (db) {
    sqlite3_close_v2(db);
    db = nullptr;
  }
  return ret;
}

void nop(sqlite3_stmt *stmt, void *res) {}

bool table_exists(string db_loc, string table_name) {
  string sql = "SELECT name FROM sqlite_master WHERE type = 'table' AND name = '" + table_name + "';";
  return send_query(db_loc, sql, nop, NULL);
}

//s_ptr will contain the first column of last result row.
void get_first_text(struct sqlite3_stmt *stmt, void *s_ptr) {
  string *p = (string *)s_ptr;
  *p = string((const char *)sqlite3_column_text(stmt, 0));
}
//g++ -E -P sqlite_util.cpp -I ../include --std=c++17
#define TABLE_HANDLER(NAME) \
  void NAME ## _handler(struct sqlite3_stmt *stmt, void *v_ptr) {\
    vector<NAME ## _entry> *p = (vector<NAME ## _entry> *)v_ptr;\
    p->push_back(NAME ## _entry(stmt));\
  }
TABLE_HANDLER(file_meta)
TABLE_HANDLER(file_content)
TABLE_HANDLER(history_meta)
TABLE_HANDLER(history_opl)
TABLE_HANDLER(history)
