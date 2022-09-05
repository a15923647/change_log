#include "sqlite_storage.h"
using namespace std;
sqlite3 *db;
mutex *db_mutex = nullptr;
mutex *tx_mutex = nullptr;

string double_single_quote(string& original);

const std::vector<std::string> table_names = {
  "file_meta", "file_content", "history_meta", "history_opl"
};

const std::vector<std::string> SQLiteStorage::create_table_queries = {
  "CREATE TABLE file_meta (\
    hash             char(64),\
    path             TEXT PRIMARY KEY\
    );",
  "CREATE TABLE file_content (\
    path             TEXT PRIMARY KEY,\
    content          TEXT\
    );",
  "CREATE TABLE history_meta (\
    path             TEXT,\
    ev_type          INT,\
    trigger_time     INT,\
    id               INTEGER PRIMARY KEY AUTOINCREMENT\
    );",
  "CREATE TABLE history_opl (\
    path key         text,\
    new_hash         text,\
    opl_dump         text,\
    id               INTEGER PRIMARY KEY AUTOINCREMENT\
    );"
};

bool send_query(string& sql, void (*handler)(sqlite3_stmt* stmt, void *ret_struct), void *result_struct, bool silent=false) {
  db_mutex->lock();
  int rc = sqlite3_open_v2(config::db_path.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE, NULL);
  if (rc) {
    cerr << "fail to open sqlite db at " << config::db_path << endl;
    exit(1);
  }
  if (!silent)
    cout << "open db sucessfully\n";

  sqlite3_busy_timeout(db, 10000);

  sqlite3_stmt *stmt = NULL;
  if (!silent)
    cout << "send query: " << sql << endl;
  int chk = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
  if (chk != SQLITE_OK) {
    cerr << "encounter error on executing sql statement below\n" + sql << endl << sqlite3_errmsg(db) << endl;
    exit(2);
  }
  if (!silent)
    cout << "run sql: " << sql << " sucessfully!\n";
  bool ret = false;

  rc = sqlite3_step(stmt);
  while (rc == SQLITE_ROW) {
    handler(stmt, result_struct);
    ret = true;
    rc = sqlite3_step(stmt);
  }

  if (rc != SQLITE_DONE) {
    cerr << "db rejects: " << sql << sqlite3_errmsg(db) << endl;
    exit(2);
  }
  sqlite3_finalize(stmt);
  if (db) {
    sqlite3_close_v2(db);
    db = nullptr;
  }
  db_mutex->unlock();
  return ret;
}

void nop(sqlite3_stmt* stmt, void *res) {}

bool file_in_db(string& abss) {
  string sql = "SELECT path FROM file_meta WHERE path = '" + double_single_quote(abss) + "';";
  return send_query(sql, nop, NULL);
}

bool table_exist(string table_name) {
  string sql = "SELECT name FROM sqlite_master WHERE type = 'table' AND name = '" + table_name + "';";
  return send_query(sql, nop, NULL, true);
}
//config must be loaded
SQLiteStorage::SQLiteStorage() : SQLiteStorage(config::db_path) {}

static void create_tables_ifnexist(const vector<string>& create_table_queries, const vector<string>& table_names) {
  for (size_t i = 0; i < create_table_queries.size(); i++) {
    string table_name = table_names[i];
    string sql = create_table_queries[i];
    if (!table_exist(table_name))
      send_query(sql, nop, NULL);
  }
}

SQLiteStorage::SQLiteStorage(string& db_path) {
  if (db_path.empty()) { //imply that config is not loaded
    cerr << "db path is empty\n";
    exit(4);
  }
  config::db_path = db_path;
  if (!db_mutex) {
    db_mutex = new mutex();
  }
  if (!tx_mutex) {
    tx_mutex = new mutex();
  }
  create_tables_ifnexist(create_table_queries, table_names);
}

string double_single_quote(string& original) {
  string res;
  for (char c : original) {
    if (c == '\'')
      res += '\'';
    res += c;
  }
  return res;
}

static inline void db_new_file(string& abss, string& content) {
  //string content;
  //read_whole_file(abss, content);
  string hash = sha256(content);

  string sql = "INSERT INTO file_meta(hash, path) VALUES('" + hash + "', '" + double_single_quote(abss) +  "'); ";
  send_query(sql, nop, NULL);
  sql = "INSERT INTO file_content(path, content) VALUES('" + double_single_quote(abss) + "', '" + double_single_quote(content) + "'); ";
  send_query(sql, nop, NULL);
}

void get_first_text(struct sqlite3_stmt* stmt, void *s_ptr) {
  string *p = (string *)s_ptr;
  *p = std::string((const char *)sqlite3_column_text(stmt, 0));
}

static inline void db_modify_file(string& abss, Event& ev) {
  string sql = "SELECT hash FROM file_meta WHERE path = '" + double_single_quote(abss) + "' ;";
  string prev_hash;
  send_query(sql, get_first_text, &prev_hash);
  string content;
  read_whole_file(abss, content);
  string hash = sha256(content);
  if (hash == prev_hash) {
    cout << abss << " not changed\n";
    return;
  }
  //get prev content
  string prev_content;
  sql = "SELECT content FROM file_content WHERE path = '" + double_single_quote(abss) + "';";
  send_query(sql, get_first_text, &prev_content);
  LCS lcs(prev_content, content);
  string lcs_res;
  lcs.lcs(lcs_res);
  cout << "edit content length = " << lcs.op_list.content_length() << endl;
  stringstream ss;
  ss << lcs.op_list;
  std::istreambuf_iterator<char> eos;
  string opl_dump(std::istreambuf_iterator<char>(ss), eos);
  

  //change hash column in file_meta
  sql = "UPDATE file_meta SET hash = '" + hash + "' WHERE path = '" + double_single_quote(abss) + "';";
  send_query(sql, nop, NULL);
  //update file_content entry
  sql = "UPDATE file_content SET content = '" + double_single_quote(content) + "' WHERE path = '" + double_single_quote(abss) + "';";
  send_query(sql, nop, NULL);
  //push to history_meta
  sql = "INSERT INTO history_meta (path, ev_type, trigger_time) VALUES('" + double_single_quote(abss) + "', " + to_string(ev.ev_type) + ", " + to_string(ev.trigger_time) + ");";
  send_query(sql, nop, NULL);
  //push to history_opl
  sql = "INSERT INTO history_opl (path, new_hash, opl_dump) VALUES('" + double_single_quote(abss) + "', '" + hash + "', '" + double_single_quote(opl_dump) + "');";
  send_query(sql, nop, NULL);
}

void SQLiteStorage::update(Event ev) {
  create_tables_ifnexist(create_table_queries, table_names);
  fs::path abspath(fs::path(ev.path));
  string abss = abspath.u8string();
  bool isdir = fs::is_directory(abspath);
  bool isreg = fs::is_regular_file(abspath);
  if (!isdir && !isreg) return;
  bool in_db = file_in_db(abss);
  if (ev.ev_type == EventType::CREATE) {
    if (!in_db) {
      string fake_content;
      //update path with empty string to avoid triggering infinite open events, which becomes EventType::CREATE in inotify_watcher.cpp.
      db_new_file(abss, fake_content);
      ev.ev_type = EventType::MODIFY;
    } else {
      cout << abss << " has already in database\n";
      return;
    }
  }
  string content;
  if (isreg) {
    read_whole_file(ev.path, content);
    if (!validUtf8(content))
      return;
  }
  tx_mutex->lock();
  //create event is dealt before reading content.
  if (ev.ev_type == EventType::MODIFY) {
    db_modify_file(abss, ev);
  } else if (ev.ev_type == EventType::DELETE) {
    //db_delete_file(abss, ev);//ignore
    cout << "delete a file\n";
  }
  tx_mutex->unlock();
}

//write to stdout directly, no return value
void *SQLiteStorage::query(string& q) {
  bool res = send_query(q, nop, NULL);
  return NULL;
}
