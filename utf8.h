#ifndef UTF8_H_
#define UTF8_H_

#include <vector>

typedef std::vector<wchar_t> wstring;

wstring
utf8_to_wchar(const char *utf8_data);

wstring
utf8_to_wchar(const char *utf8_data, int utf8_data_size);

#endif // UTF8_H_
