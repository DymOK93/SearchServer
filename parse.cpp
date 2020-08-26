#include "parse.h"

string_view Strip(string_view s) {
  s.remove_prefix(s.find_first_not_of(' '));
  s.remove_suffix(s.length() - s.find_last_not_of(' ') - 1);
  return s;
}

vector<string_view> SplitBy(string_view s, char sep) {
  vector<string_view> result;
  while (!s.empty()) {
    size_t pos = s.find(sep);
    result.push_back(s.substr(0, pos));
    s.remove_prefix(pos != s.npos ? pos + 1 : s.size());
  }
  return result;
}

