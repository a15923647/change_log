#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <regex>
#include <chrono>
#include <unordered_map>
#include <filesystem>
#include <sqlite3.h>
#include <unordered_map>
#include "sqlite_util.h"
#include "sqlite_storage.h"
#include "event.h"
#include "patch.h"
#include "file.h"
#include "operation.h"

using namespace std;

void list(string db_loc, regex& pat) {
  string sql = "SELECT * FROM history_meta;";
  vector<history_meta_entry> res;
  send_query(db_loc, sql, history_meta_handler, (void *)&res);
  unordered_map<string, vector<history_meta_entry>> path_history;

  for (auto& e : res) {
    if (!regex_match(e.path, pat)) continue;
    if (!path_history.count(e.path)) {
      path_history[e.path] = vector<history_meta_entry>();
    }
    path_history[e.path].push_back(e);
  }
  for (auto& [k, v] : path_history) {
    cout << k << " " << v.size() << " time modify\n";
  }
}

void ll(string db_loc, regex& pat) {
  string sql = "SELECT * FROM history_meta;";
  vector<history_meta_entry> res;
  send_query(db_loc, sql, history_meta_handler, (void *)&res);
  unordered_map<string, vector<history_meta_entry>> path_history;

  for (auto& e : res) {
    if (!regex_match(e.path, pat)) continue;
    if (!path_history.count(e.path)) {
      path_history[e.path] = vector<history_meta_entry>();
    }
    path_history[e.path].push_back(e);
  }

  for (auto& [k, v] : path_history) {
    cout << k << endl;
    for (auto& e : v) {
      cout << "  " + evTypeStringify[e.ev_type] + " at " << e.trigger_time << endl;
    }
  }
}

void get_cur_content(string db_loc, string& path, string& cur_content) {
  string sql = "SELECT content FROM file_content WHERE path = '" + double_single_quote(path) + "';";
  send_query(db_loc, sql, get_first_text, (void *)&cur_content);
}

void str2opl(string& s, OperationList& opl) {
  stringstream ss;
  ss << s;
  ss >> opl;
}

void back(string db_loc, regex& pat, time_t til) {
  auto patch = Patch();
  string sql = "SELECT * FROM history_meta JOIN history_opl ON history_meta.id = history_opl.id WHERE trigger_time >= " + to_string(max((time_t)0, til)) +";";
  vector<history_entry> qres;
  send_query(db_loc, sql, history_handler, (void *)&qres);

  unordered_map<string, vector<history_entry>> path_history;
  for (auto& e : qres) {
    if (!regex_match(e.path, pat)) continue;
    if (!path_history.count(e.path)) {
      path_history[e.path] = vector<history_entry>();
    }
    path_history[e.path].push_back(e);
  }
  string res;
  for (auto& [k, v] : path_history) {
    string cur_content;
    string path_str(k);
    std::filesystem::path path(path_str);
    get_cur_content(db_loc, path_str, cur_content);
    reverse(v.begin(), v.end());//patch back from back
    history_entry& last = v[0];
    int patch_times = (til < 0) ? -til : 0x7fffffff;
    for(auto& ent : v) {
      if (patch_times-- < 0) break;
      OperationList op_list;
      str2opl(ent.opl_dump, op_list);
      vector<edit_operation> *eop = op_list.diff_list();
      patch.patchBack(cur_content, *eop, res);
      delete eop;
      cur_content = res;
      res = "";
      last = ent;
    }
    cout << "recover " + k + " to version back to " << last.trigger_time << endl;
    cout << cur_content;
    //currently just list them
    //DO SOMETHING HERE!
  }
}

void print_usage(const char *argv[]) {
  cerr << argv[0] << " db_path command [path_pattern]\n";
  cerr << "supported command:\nll\nlist\nback\n";
}

int main(int argc, const char *argv[]) {
  if (argc < 3) {
    print_usage(argv);
    exit(1);
  }
  string db_loc(argv[1]);
  regex pat(".*");
  if (argc > 3)
    pat = regex(argv[3]);
  if (strcmp(argv[2], "back") == 0) {
    time_t til = 0;
    if (argc > 4)
      til = stoi(argv[4]);
    back(db_loc, pat, til);
  } else if (strcmp(argv[2], "list") == 0) {
    list(db_loc, pat);
  } else if (strcmp(argv[2], "ll") == 0) {
    ll(db_loc, pat);
  } else {
    print_usage(argv);
    exit(1);
  }
  return 0;
}
