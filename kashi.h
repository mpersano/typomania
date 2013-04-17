#ifndef KASHI_H_
#define KASHI_H_

#include <vector>
#include <string>

#include "utf8.h"

class kashi {
public:
	static kashi *load(const char *path);

	kashi(const wstring& name, const wstring& artist, const wstring& genre, const std::string& stream);
	~kashi();

	struct serifu {
		serifu(int duration, const wstring& kanji, const wstring& kana);

		int duration;
		wstring kanji;
		wstring kana;
	};

	typedef std::vector<serifu> serifu_cont;
	typedef serifu_cont::iterator iterator;
	typedef serifu_cont::const_iterator const_iterator;

	iterator begin() { return serifu_list.begin(); }
	iterator end() { return serifu_list.end(); }

	const_iterator begin() const { return serifu_list.begin(); }
	const_iterator end() const { return serifu_list.end(); }

	wstring name;
	wstring artist;
	wstring genre;

	int level;

	std::string stream;

private:
	void init_level();

	serifu_cont serifu_list;

	kashi(const kashi&);
	kashi& operator=(const kashi&);
};

#endif // KASHI_H_
