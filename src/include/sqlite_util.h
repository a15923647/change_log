#include <sqlite3.h>
#include <string>
#include <vector>
#include <iostream>
#include "sqlite_storage.h"
std::string double_single_quote(std::string& s);
bool send_query(std::string& db_loc, std::string& sql, void (*handler)(sqlite3_stmt *, void *ret_struct), void *ret_struct);
void nop(sqlite3_stmt *stmt, void *res);
bool table_exists(std::string db_loc, std::string table_name);
void get_first_text(struct sqlite3_stmt *stmt, void *s_ptr);
#define TABLE_HANDLER_DECL(NAME) \
  void NAME ## _handler(struct sqlite3_stmt *stmt, void *v_ptr);
TABLE_HANDLER_DECL(file_meta)
TABLE_HANDLER_DECL(file_content)
TABLE_HANDLER_DECL(history_meta)
TABLE_HANDLER_DECL(history_opl)
TABLE_HANDLER_DECL(history)
