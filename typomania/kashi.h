#pragma once

#include <vector>
#include <string>
#include <memory>

#include "noncopyable.h"
#include "rgba.h"
#include "vec2.h"

struct font;
struct glyph_fx;

namespace gl {
class texture;
}

using fx_cont = std::vector<std::unique_ptr<glyph_fx>>;

struct serifu_part
{
	serifu_part() { }
	virtual ~serifu_part() { }

	virtual const std::wstring& get_kana() const = 0;
	virtual int get_width() const = 0;
	virtual int draw(int num_highlighted, const rgba color[2]) const = 0;
	virtual void get_kana_glyph_fx(size_t index, const vec2f& offset, fx_cont& cont) const = 0;

	int draw_kana(const font *f, float x, float y, const std::wstring& kana, int num_highlighted, const rgba color[2]) const;
};

using serifu_part_ptr = std::unique_ptr<serifu_part>;

struct serifu_kana_part : serifu_part
{
	serifu_kana_part();

	const std::wstring& get_kana() const override
	{ return kana; }

	int get_width() const override;

	int draw(int num_highlighted, const rgba color[2]) const override;

	void get_kana_glyph_fx(size_t index, const vec2f& offset, fx_cont& cont) const override;

	std::wstring kana;
	const font *kana_font;
};

struct serifu_furigana_part : serifu_part
{
	serifu_furigana_part();

	const std::wstring& get_kana() const override
	{ return furigana; }

	int get_width() const override;

	int draw(int num_highlighted, const rgba color[2]) const override;

	void get_kana_glyph_fx(size_t index, const vec2f& offset, fx_cont& cont) const override;

	std::wstring kanji;
	std::wstring furigana;

	const font *kanji_font;
	const font *furigana_font;
};

struct pattern_node;

struct serifu
{
public:
	serifu(int duration);

	bool parse(const std::wstring& text);

	void draw(int num_highlighted, const rgba color[2]) const;

	int duration() const;

	class kana_iterator : public std::iterator<std::forward_iterator_tag, wchar_t>
	{
	public:
		kana_iterator();
		kana_iterator(const std::vector<serifu_part_ptr>::const_iterator& it, const std::vector<serifu_part_ptr>::const_iterator& end);

		wchar_t operator*() const;

		bool operator!=(const kana_iterator& other) const;

		kana_iterator& operator++();

		void get_glyph_fx(fx_cont& fx_list) const;

	private:
		void next();
		void skip_non_kana();
		wchar_t cur_kana() const;

		std::vector<serifu_part_ptr>::const_iterator iter_, end_;
		size_t cur_part_index_;
		float base_x_;
	};

	kana_iterator kana_begin() const;
	kana_iterator kana_end() const;

	struct romaji_iterator : public std::iterator<std::forward_iterator_tag, char>
	{
	public:
		romaji_iterator(const kana_iterator& kana_it);
		romaji_iterator(const kana_iterator& kana_it, const pattern_node *cur_pattern);

		char operator*() const;

		bool operator!=(const romaji_iterator& other) const;

		romaji_iterator& operator++();

	private:
		void next();
		void skip_optional_pattern();
		void consume_kana();

		serifu::kana_iterator kana_it_;
		const pattern_node *cur_pattern_;
	};

	romaji_iterator romaji_begin() const;
	romaji_iterator romaji_end() const;

private:
	int duration_;
	std::vector<serifu_part_ptr> section_list;

	friend class kana_iterator;
};

using serifu_ptr = std::unique_ptr<serifu>;


class kashi : private noncopyable
{
public:
	kashi();
	~kashi();

	bool load(const std::string& path);

	using serifu_cont = std::vector<serifu_ptr>;
	using iterator = serifu_cont::iterator;
	using const_iterator = serifu_cont::const_iterator;

	iterator begin() { return serifu_list.begin(); }
	iterator end() { return serifu_list.end(); }

	const_iterator begin() const { return serifu_list.begin(); }
	const_iterator end() const { return serifu_list.end(); }

	std::wstring name;
	std::wstring artist;
	std::wstring genre;

	int level;

	std::string stream;
	const gl::texture *background;

private:
	void init_level();

	serifu_cont serifu_list;
};

using kashi_ptr = std::unique_ptr<kashi>;
