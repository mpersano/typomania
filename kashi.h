#ifndef KASHI_H_
#define KASHI_H_

#include <vector>
#include <string>

#include "utf8.h"

struct kashi {
	static kashi *load(const char *path);

	kashi(const wstring& name, const wstring& artist, const wstring& genre, const std::string& stream);
	~kashi();

	wstring name;
	wstring artist;
	wstring genre;
	std::string stream;

	struct serifu {
		serifu(int duration, const wstring& kanji, const wstring& kana);

		int duration;
		wstring kanji;
		wstring kana;
	};

	std::vector<serifu> serifu_list;
};

#endif // KASHI_H_
