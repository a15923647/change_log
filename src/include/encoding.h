#ifndef ENCODING_INCLUDED
#define ENCODING_INCLUDED
//https://leetcode.com/problems/utf-8-validation
#include<string>
#define IB 0b0
#define IIB 0b110
#define IIIB 0b1110
#define IVB 0b11110
#define CB 0b10
using namespace std;
static inline bool check(size_t n, std::string& data, int& i) {
  const int end = i + n;
  for (; i < end && i < data.size(); i++) {
    int cur_byte = data[i];
    if ((cur_byte >> 6) ^ CB)
      return false;
  }
  return i == end;
}

bool validUtf8(std::string& data) {
  const size_t size = data.size();
  for (int i = 0; i < size; ) {
    int cur_byte = data[i];
    if (!((cur_byte >> 3) ^ IVB)) {
      if (!check(3, data, ++i)) 
        return false;
    } else if (!((cur_byte >> 4) ^ IIIB)) {
      if (!check(2, data, ++i)) 
        return false;
    } else if (!((cur_byte >> 5) ^ IIB)) {
      if (!check(1, data, ++i)) 
        return false;
    } else if (!((cur_byte >> 7) ^ IB)) {
      i++;
      continue;
    } else {
      return false;
    }
  }
  return true;
}
#endif
