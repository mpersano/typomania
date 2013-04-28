#ifndef KASHI_H_
#define KASHI_H_

#include <vector>
#include <string>

#include "utf8.h"

struct font;

struct serifu_part {
	virtual const wstring& get_kana() const = 0;

	virtual int get_width() const = 0;
	virtual int draw(float x, float y, int num_highlighted, float alpha) const = 0;

	int draw_kana(const font *f, float x, float y, const wstring& kana, int num_highlighted, float alpha) const;
};

struct serifu_kana_part : serifu_part {
	serifu_kana_part();

	const wstring& get_kana() const
	{ return kana; }

	int get_width() const;

	int draw(float x, float y, int num_highlighted, float alpha) const;

	wstring kana;
	font *kana_font;
};

struct serifu_furigana_part : serifu_part {
	serifu_furigana_part();

	const wstring& get_kana() const
	{ return furigana; }

	int get_width() const;
	int draw(float x, float y, int num_highlighted, float alpha) const;

	wstring kanji;
	wstring furigana;

	font *kanji_font;
	font *furigana_font;
};

struct serifu {
	serifu(int duration)
	: duration(duration)
	{ }

	virtual ~serifu();

	bool parse(const wstring& text);

	void draw(float x, float y, int num_highlighted, float alpha) const;

	typedef std::vector<serifu_part *> section_cont;
	typedef section_cont::iterator iterator;
	typedef section_cont::const_iterator const_iterator;

	iterator begin() { return section_list.begin(); }
	iterator end() { return section_list.end(); }

	const_iterator begin() const { return section_list.begin(); }
	const_iterator end() const { return section_list.end(); }

	int duration;
	section_cont section_list;
};

class serifu_kana_iterator {
public:
	serifu_kana_iterator() { }
	serifu_kana_iterator(const serifu *s);

	wchar_t operator*() const;
	serifu_kana_iterator& operator++();

private:
	void next();
	void skip_non_kana();
	wchar_t cur_kana() const;

	serifu::const_iterator iter, end;
	size_t cur_part_index;
};

struct pattern_node;

struct serifu_romaji_iterator {
public:
	serifu_romaji_iterator() { }
	serifu_romaji_iterator(const serifu *s);
	serifu_romaji_iterator(const pattern_node *cur_pattern, const serifu_kana_iterator& kana);

	serifu_romaji_iterator& operator++();

	char operator*() const;

private:
	void next();
	void skip_optional_pattern();

	serifu_kana_iterator kana;
	const pattern_node *cur_pattern;
};

class kashi {
public:
	kashi();
	~kashi();

	bool load(const char *path);

	typedef std::vector<serifu *> serifu_cont;
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
