#include <cstring>

#include "utf8.h"
#include "panic.h"

wstring
utf8_to_wchar(const char *utf8_data)
{
	return utf8_to_wchar(utf8_data, strlen(utf8_data));
}

wstring
utf8_to_wchar(const char *utf8_data, int utf8_data_size)
{
	wstring result;

	wchar_t cur_wchar = 0;
	int extra_bytes = 0;

	for (const char *p = utf8_data; p != &utf8_data[utf8_data_size]; p++) {
		unsigned char ch = *p;

		if (extra_bytes == 0) {
			if ((ch & 0x80) == 0) {
				result.push_back(ch);
			} else if ((ch & ~0x1f) == 0xc0) {
				cur_wchar = ch & 0x1f;
				extra_bytes = 1;
			} else if ((ch & ~0x0f) == 0xe0) {
				cur_wchar = ch & 0x0f;
				extra_bytes = 2;
			} else if ((ch & ~0x07) == 0xf0) {
				cur_wchar = ch & 0x07;
				extra_bytes = 3;
			} else if ((ch & ~0x03) == 0xf8) {
				cur_wchar = ch & 0x03;
				extra_bytes = 4;
			} else if ((ch & ~0x01) == 0xfc) {
				cur_wchar = ch & 0x01;
				extra_bytes = 5;
			} else {
				goto failed;
			}
		} else {
			if ((ch & ~0x3f) != 0x80)
				goto failed;

			cur_wchar = (cur_wchar << 6) | (ch & 0x3f);

			if (--extra_bytes == 0) {
				result.push_back(cur_wchar);
				cur_wchar = 0;
			}
		}
	}

	if (extra_bytes != 0)
		goto failed;

	result.push_back(L'\0');

	return result;

failed:
	panic("invalid UTF-8 character sequence");
	// NOTREACHED
	return wstring();
}
