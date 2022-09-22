#ifndef NOO_UTF8_H
#define NOO_UTF8_H

#include "Nooskewl_Shim/main.h"

namespace noo {

namespace util {

int utf8_len(std::string text);
int utf8_len_bytes(std::string text, int char_count);
Uint32 utf8_char_next(std::string text, int &offset);
Uint32 utf8_char_offset(std::string text, int o);
Uint32 utf8_char(std::string text, int i);
std::string utf8_char_to_string(Uint32 ch);
std::string utf8_substr(std::string s, int start, int count = -1);

} // End namespace util

}

#endif // NOO_UTF8_H
