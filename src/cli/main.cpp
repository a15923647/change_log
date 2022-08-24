#include <iostream>
#include <fstream>
#include <string.h>
#include <regex>
#include <chrono>
#include <unordered_map>
#include <filesystem>
#include "vector_storage.h"
#include "event.h"
#include "patch.h"
#include "file.h"
#include "operation.h"

using namespace std;

void read_from_dump(ifstream& fin, unordered_map<string, vector<Node>>& nodem, regex& pat) {
  size_t size; fin >> size; fin.get(); //eat endl at ../daemon/main.cpp:23
  while (size--) {
    Node node(0);
    fin >> node; fin.get(); //eat endl at ../daemon/main.cpp:25
    if (regex_match(node.path, pat)) {
      if (!nodem.count(node.path)) {
        nodem[node.path] = vector<Node>();
      }
      nodem[node.path].push_back(node);
    }
  }
}

void ll(ifstream& fin, regex& pat) {
  unordered_map<string, vector<Node>> nodem;
  read_from_dump(fin, nodem, pat);
  for (auto& [k, v] : nodem) {
    cout << k << endl;
    reverse(v.begin(), v.end());
    cout << "  time length\n";
    for (auto& node: v)
      cout << "  " << node.trigger_time << " " << node.op_list.content_length() << endl;
  }
}

void list(ifstream& fin, regex& pat) {
  unordered_map<string, vector<Node>> nodem;
  read_from_dump(fin, nodem, pat);
  for (auto& [k, v] : nodem) {
    size_t total_op_cnt = 0;
    for (auto& node : v)
      total_op_cnt += node.op_list.content_length();
    cout << k << " " << total_op_cnt << " edits\n";
  }
}

void back(ifstream& fin, regex& pat, time_t til) {
  puts("back");
  auto patch = Patch();
  unordered_map<string, vector<Node>> nodem;
  read_from_dump(fin, nodem, pat);
  string res;
  for (auto& [k, v] : nodem) {
    string cur_content;
    string path_str(k);
    std::filesystem::path path(path_str);
    read_whole_file(path_str, cur_content);//file content may not match the operation context
    reverse(v.begin(), v.end());//patch back from back
    //future work: check modify time or hash
    /*
    auto ftime = fs::last_write_time(path);
    time_t mtime = decltype(ftime)::clock::to_time_t(ftime);
    if (abs(mtime - v[0].trigger_time) > 0.5) {
      cerr << "mtime mismatch!\n";
      exit(3);
    }
    */
    for(auto& node : v) {
      if (node.trigger_time < til) break;
      vector<edit_operation> *eop = node.op_list.diff_list();
      patch.patchBack(cur_content, *eop, res);
      delete eop;
      puts("----------------------------------------");
      cout << res;
      cur_content = res;
      res = "";
    }
    cout << "previous version: \n";
    cout << cur_content;
    //DO SOMETHING HERE!
  }
}

ifstream get_if(char *path) {
  ifstream f(path);
  if (!f.good()) {
    cerr << "file is not good\n";
    exit(2);
  }
  return f;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    cerr << argv[0] << " command\n";
    exit(1);
  }
  regex all(".*");
  if (strcmp(argv[1], "back") == 0) {
    if (argc < 3) {
      cout << argv[0] << " back patch_file [path_pattern]\n";
      return 1;
    }
    ifstream fin = get_if(argv[2]);

    regex pat(".*");
    if (argc > 3)
      pat = regex(argv[3]);

    time_t til = 0;
    if (argc > 4)
      til = stoi(argv[4]);

    back(fin, pat, til);
    fin.close();
  } else if (strcmp(argv[1], "list")) {
    if (argc < 3) {
      cout << argv[0] << " list patch_file [path_pattern]\n";
      return 1;
    }
    ifstream fin = get_if(argv[2]);
    list(fin, all);
  } else if (strcmp(argv[1], "ll")) {
    if (argc < 3) {
      cout << argv[0] << " ll patch_file [path_pattern]\n";
      return 1;
    }
    ifstream fin = get_if(argv[2]);
    ll(fin, all);
  }
  return 0;
}
