#pragma once

#include <vector>
#include <string>
#include <memory>

#include "rgba.h"
#include "vector2.h"
#include "utf8.h"

struct font;
struct glyph_fx;

using fx_cont = std::vector<std::unique_ptr<glyph_fx>>;

struct serifu_part
{
	serifu_part() { }
	virtual ~serifu_part() { }

	virtual const wstring& get_kana() const = 0;

	virtual int get_width() const = 0;

	virtual int draw(int num_highlighted, const rgba color[2]) const = 0;

	virtual void get_kana_glyph_fx(size_t index, const vector2& offset, fx_cont& cont) const = 0;

	int draw_kana(const font *f, float x, float y, const wstring& kana, int num_highlighted, const rgba color[2]) const;
};

using serifu_part_ptr = std::unique_ptr<serifu_part>;

struct serifu_kana_part : serifu_part
{
	serifu_kana_part();

	const wstring& get_kana() const
	{ return kana; }

	int get_width() const;

	int draw(int num_highlighted, const rgba color[2]) const;

	void get_kana_glyph_fx(size_t index, const vector2& offset, fx_cont& cont) const;

	wstring kana;
	const font *kana_font;
};

struct serifu_furigana_part : serifu_part
{
	serifu_furigana_part();

	const wstring& get_kana() const
	{ return furigana; }

	int get_width() const;

	int draw(int num_highlighted, const rgba color[2]) const;

	void get_kana_glyph_fx(size_t index, const vector2& offset, fx_cont& cont) const;

	wstring kanji;
	wstring furigana;

	const font *kanji_font;
	const font *furigana_font;
};

struct serifu
{
	serifu(int duration)
	: duration(duration)
	{ }

	virtual ~serifu();

	bool parse(const wstring& text);

	void draw(int num_highlighted, const rgba color[2]) const;

	using section_cont = std::vector<serifu_part_ptr>;
	using iterator = section_cont::iterator;
	using const_iterator = section_cont::const_iterator;

	iterator begin() { return section_list.begin(); }
	iterator end() { return section_list.end(); }

	const_iterator begin() const { return section_list.begin(); }
	const_iterator end() const { return section_list.end(); }

	int duration;
	section_cont section_list;
};

using serifu_ptr = std::unique_ptr<serifu>;

class serifu_kana_iterator
{
public:
	serifu_kana_iterator() { }
	serifu_kana_iterator(const serifu *s);

	wchar_t operator*() const;
	wchar_t operator[](int index) const;

	serifu_kana_iterator& operator++();

	void get_glyph_fx(fx_cont& fx_list) const;

private:
	void next();
	void skip_non_kana();
	wchar_t cur_kana() const;

	serifu::const_iterator iter, end;
	size_t cur_part_index;
	float base_x;
};

struct pattern_node;

struct serifu_romaji_iterator
{
public:
	serifu_romaji_iterator() { }
	serifu_romaji_iterator(const serifu *s);
	serifu_romaji_iterator(const pattern_node *cur_pattern, const serifu_kana_iterator& kana);

	serifu_romaji_iterator& operator++();

	char operator*() const;

private:
	void next();
	void skip_optional_pattern();
	void consume_kana();

	serifu_kana_iterator kana;
	const pattern_node *cur_pattern;
};

class kashi
{
public:
	kashi();
	~kashi();

	bool load(const char *path);

	using serifu_cont = std::vector<serifu_ptr>;
	using iterator = serifu_cont::iterator;
	using const_iterator = serifu_cont::const_iterator;

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

using kashi_ptr = std::unique_ptr<kashi>;
