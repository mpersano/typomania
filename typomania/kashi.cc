#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <cassert>

#include "panic.h"
#include "resources.h"
#include "render.h"
#include "pattern.h"
#include "kana.h"
#include "glyph_fx.h"
#include "kashi.h"

static bool
is_kana(wchar_t ch)
{
	return
	  (ch >= 12353 && ch <= 12438) // hiragana
	  || (ch >= 12449 && ch <= 12538) // katakana
	  || ch == L'ー'
	  || (ch >= 'A' && ch <= 'Z') || (ch >= L'Ａ' && ch <= L'Ｚ')
	  || (ch >= 'a' && ch <= 'z') || (ch >= L'ａ' && ch <= L'ｚ')
	  || (ch >= '0' && ch <= '9') || (ch >= L'０' && ch <= L'９');
}

static std::vector<char *>
split(char *str, const char *delim)
{
	std::vector<char *> tokens;

	for (char *p = strtok(str, delim); p; p = strtok(0, delim))
		tokens.push_back(p);

	return tokens;
}

kashi::kashi()
{ }

kashi::~kashi()
{
}

bool
kashi::load(const char *path)
{
	FILE *in;

	if ((in = fopen(path, "r")) == 0)
		return false;

	char line[512];

	if (!fgets(line, sizeof(line), in)) {
		fclose(in);
		return false;
	}

	std::vector<char *> tokens = split(line, "\t\n");
	if (tokens.size() < 4) {
		fclose(in);
		return false;
	}

	name = utf8_to_wchar(tokens[0]);
	artist = utf8_to_wchar(tokens[1]);
	genre = utf8_to_wchar(tokens[2]);

	stream = tokens[3];

	if (tokens.size() > 4)
		background = tokens[4];

	while (fgets(line, sizeof(line), in)) {
		std::vector<char *> tokens = split(line, "\t\n");
		if (tokens.size() != 2) {
			fclose(in);
			return false;
		}

		serifu_ptr p(new serifu(atoi(tokens[0])));
		p->parse(utf8_to_wchar(tokens[1]));
		serifu_list.push_back(std::move(p));
	}

	fclose(in);

	init_level();

	return true;
}

void
kashi::init_level()
{
	float top_kana_per_ms = 0;

	for (auto& serifu : serifu_list) {
		int kana_count = 0;

		for (serifu_romaji_iterator j(serifu.get()); *j; ++j)
			++kana_count;

		float kana_per_ms = static_cast<float>(kana_count)/serifu->duration;

		if (kana_per_ms > top_kana_per_ms)
			top_kana_per_ms = kana_per_ms;
	}

	level = static_cast<int>(top_kana_per_ms*11000.);

	if (level >= 100)
		level = 99;
}

serifu::~serifu()
{
}

bool
serifu::parse(const wstring& text)
{
	enum state {
		NONE,
		KANA,
		KANJI,
		FURIGANA,
	};
	state cur_state = NONE;

	serifu_part_ptr cur_part;

	for (const wchar_t *p = &text[0]; *p; p++) {
		wchar_t ch = *p;

		if (ch == '(') {
			if (cur_state == KANJI || cur_state == FURIGANA)
				return false;
			else if (cur_state == KANA)
				section_list.push_back(std::move(cur_part));

			cur_part.reset(new serifu_furigana_part);
			cur_state = KANJI;
		} else if (ch == '|') {
			if (cur_state != KANJI)
				return false;

			cur_state = FURIGANA;
		} else if (ch == ')') {
			if (cur_state != FURIGANA)
				return false;

			section_list.push_back(std::move(cur_part));
			cur_state = NONE;
		} else {
			switch (cur_state) {
				case NONE:
					cur_part.reset(new serifu_kana_part);
					cur_state = KANA;
					// FALLTHROUGH

				case KANA:
					dynamic_cast<serifu_kana_part *>(cur_part.get())->kana.push_back(ch);
					break;

				case KANJI:
					dynamic_cast<serifu_furigana_part *>(cur_part.get())->kanji.push_back(ch);
					break;

				case FURIGANA:
					dynamic_cast<serifu_furigana_part *>(cur_part.get())->furigana.push_back(ch);
					break;
			}
		}
	}

	if (cur_state != NONE)
		section_list.push_back(std::move(cur_part));

	return true;
}

void
serifu::draw(int num_highlighted, const rgba color[2]) const
{
	for (auto& section : section_list) {
		num_highlighted = section->draw(num_highlighted, color);
		render::translate(section->get_width(), 0);
	}
}

int
serifu_part::draw_kana(const font *f, float x, float y, const wstring& kana, int num_highlighted, const rgba color[2]) const
{
	size_t len = 0;

	if (num_highlighted) {
		while (len < kana.size()) {
			if (is_kana(kana[len++])) {
				if (!--num_highlighted)
					break;
			}
		}
	}

	if (len) {
		render::set_color(color[0]);
		x = f->draw_stringn(&kana[0], len, x, y, 0);
	}

	if (len < kana.size()) {
		render::set_color(color[1]);
		f->draw_stringn(&kana[len], kana.size() - len, x, y, 0);
	}

	return num_highlighted;
}

serifu_kana_part::serifu_kana_part()
: kana_font(get_font("data/fonts/small_font.fnt"))
{ }

int
serifu_kana_part::get_width() const
{
	return kana_font->get_string_width(&kana[0], kana.size());
}

int
serifu_kana_part::draw(int num_highlighted, const rgba color[2]) const
{
	return draw_kana(kana_font, 0, 0, kana, num_highlighted, color);
}

void
serifu_kana_part::get_kana_glyph_fx(size_t index, const vec2f& offset, fx_cont& fx_list) const
{
	assert(index >= 0 && index < kana.size());

	float x = 0;
	for (size_t i = 0; i < index; i++)
		x += kana_font->find_glyph(kana[i])->advance_x;

	fx_list.emplace_back(new glyph_fx(kana_font, kana[index], vec2f(x, 0) + offset));
}

serifu_furigana_part::serifu_furigana_part()
: kanji_font(get_font("data/fonts/small_font.fnt"))
, furigana_font(get_font("data/fonts/tiny_font.fnt"))
{ }

int
serifu_furigana_part::get_width() const
{
	return std::max(kanji_font->get_string_width(&kanji[0], kanji.size()),
	  furigana_font->get_string_width(&furigana[0], furigana.size()));
}

int
serifu_furigana_part::draw(int num_highlighted, const rgba color[2]) const
{
	const int width = get_width();

	int num_kana = std::count_if(furigana.begin(), furigana.end(), is_kana);

	render::set_color(color[num_highlighted < num_kana]);

	kanji_font->draw_stringn(&kanji[0], kanji.size(),
	  .5*width - .5*(kanji_font->get_string_width(&kanji[0], kanji.size())), 0, 0);

	return draw_kana(furigana_font,
	  .5*width - .5*(furigana_font->get_string_width(&furigana[0], furigana.size())), 26,
	  furigana, num_highlighted, color);
}

void
serifu_furigana_part::get_kana_glyph_fx(size_t index, const vec2f& offset, fx_cont& fx_list) const
{
	assert(index >= 0 && index < furigana.size());

	float x = .5*get_width() - .5*(furigana_font->get_string_width(&furigana[0], furigana.size()));

	for (size_t i = 0; i < index; i++)
		x += furigana_font->find_glyph(furigana[i])->advance_x;

	fx_list.emplace_back(new glyph_fx(furigana_font, furigana[index], offset + vec2f(x, 26)));

	if (index == furigana.size() - 1) {
		float x = .5*get_width() - .5*(kanji_font->get_string_width(&kanji[0], kanji.size()));

		for (wchar_t ch : kanji) {
			fx_list.emplace_back(new glyph_fx(kanji_font, ch, offset + vec2f(x, 0)));
			x += kanji_font->find_glyph(ch)->advance_x;
		}
	}
}

serifu_kana_iterator::serifu_kana_iterator(const serifu *s)
: iter(s->begin())
, end(s->end())
, cur_part_index(0)
, base_x(0)
{
	skip_non_kana();
}

wchar_t
serifu_kana_iterator::operator*() const
{
	return cur_kana();
}

wchar_t
serifu_kana_iterator::operator[](int index) const
{
	serifu_kana_iterator i = *this;

	while (index) {
		++i;
		--index;
	}

	return *i;
}

serifu_kana_iterator&
serifu_kana_iterator::operator++()
{
	if (iter != end) {
		next();
		skip_non_kana();
	}

	return *this;
}

void
serifu_kana_iterator::skip_non_kana()
{
	wchar_t ch;

	while ((ch = cur_kana()) && !is_kana(ch))
		next();
}

wchar_t
serifu_kana_iterator::cur_kana() const
{
	if (iter != end) {
		const wstring& cur_kana = (*iter)->get_kana();
		return cur_kana[cur_part_index];
	} else {
		return L'\0';
	}
}

void
serifu_kana_iterator::next()
{
	if (iter != end) {
		if (++cur_part_index == (*iter)->get_kana().size()) {
			base_x += (*iter)->get_width();
			++iter;
			cur_part_index = 0;
		}
	}
}

void
serifu_kana_iterator::get_glyph_fx(fx_cont& fx_list) const
{
	return (*iter)->get_kana_glyph_fx(cur_part_index, vec2f(base_x, 0), fx_list);
}

serifu_romaji_iterator::serifu_romaji_iterator(const serifu *s)
: kana(s)
{
	consume_kana();
	skip_optional_pattern();
}

serifu_romaji_iterator::serifu_romaji_iterator(const pattern_node *cur_pattern, const serifu_kana_iterator& kana)
: kana(kana), cur_pattern(cur_pattern)
{
	skip_optional_pattern();
}

serifu_romaji_iterator&
serifu_romaji_iterator::operator++()
{
	if (cur_pattern) {
		next();
		skip_optional_pattern();
	}

	return *this;
}

char
serifu_romaji_iterator::operator*() const
{
	return cur_pattern ? cur_pattern->get_char() : 0;
}

void
serifu_romaji_iterator::next()
{
	if (!(cur_pattern = cur_pattern->next))
		consume_kana();
}

void
serifu_romaji_iterator::consume_kana()
{
	if ((cur_pattern = kana_to_pattern::find_pair(kana[0], kana[1]))) {
		++kana;
		++kana;
	} else if ((cur_pattern = kana_to_pattern::find_single(*kana))) {
		++kana;
	}
}

void
serifu_romaji_iterator::skip_optional_pattern()
{
	while (cur_pattern && cur_pattern->is_optional)
		next();
}
