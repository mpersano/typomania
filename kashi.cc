#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <cassert>

#include "panic.h"
#include "common.h"
#include "gl_vertex_array.h"
#include "kashi.h"

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
	for (iterator i = serifu_list.begin(); i != serifu_list.end(); i++)
		delete *i;
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
	if (tokens.size() != 4) {
		fclose(in);
		return false;
	}

	name = utf8_to_wchar(tokens[0]);
	artist = utf8_to_wchar(tokens[1]);
	genre = utf8_to_wchar(tokens[2]);

	stream = tokens[3];

	while (fgets(line, sizeof(line), in)) {
		std::vector<char *> tokens = split(line, "\t\n");
		if (tokens.size() != 2) {
			fclose(in);
			return false;
		}

		serifu *p = new serifu(atoi(tokens[0]));
		p->parse(utf8_to_wchar(tokens[1]));
		serifu_list.push_back(p);
	}

	fclose(in);

	init_level();

	return true;
}

void
kashi::init_level()
{
#if 0
	float top_kana_per_ms = 0;

	for (serifu_cont::const_iterator i = serifu_list.begin(); i != serifu_list.end(); i++) {
		float kana_per_ms = static_cast<float>(i->kana.size())/i->duration;

		if (kana_per_ms > top_kana_per_ms)
			top_kana_per_ms = kana_per_ms;
	}

	level = static_cast<int>(top_kana_per_ms*5000.);

	if (level >= 100)
		level = 99;
#else
	level = 1;
#endif
}

serifu::~serifu()
{
	for (iterator i = section_list.begin(); i != section_list.end(); i++)
		delete *i;
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

	serifu_part *cur_part = 0;

	for (const wchar_t *p = &text[0]; *p; p++) {
		wchar_t ch = *p;

		if (ch == '(') {
			if (cur_state == KANJI || cur_state == FURIGANA)
				return false;
			else if (cur_state == KANA)
				section_list.push_back(cur_part);

			cur_part = new serifu_furigana_part;
			cur_state = KANJI;
		} else if (ch == '|') {
			if (cur_state != KANJI)
				return false;

			cur_state = FURIGANA;
		} else if (ch == ')') {
			if (cur_state != FURIGANA)
				return false;

			section_list.push_back(cur_part);
			cur_state = NONE;
		} else {
			switch (cur_state) {
				case NONE:
					cur_part = new serifu_kana_part;
					cur_state = KANA;
					// FALLTHROUGH

				case KANA:
					dynamic_cast<serifu_kana_part *>(cur_part)->kana.push_back(ch);
					break;

				case KANJI:
					dynamic_cast<serifu_furigana_part *>(cur_part)->kanji.push_back(ch);
					break;

				case FURIGANA:
					dynamic_cast<serifu_furigana_part *>(cur_part)->furigana.push_back(ch);
					break;
			}
		}
	}

	if (cur_state != NONE)
		section_list.push_back(cur_part);

	return true;
}

void
serifu::draw(float x, float y) const
{
	for (section_cont::const_iterator i = section_list.begin(); i != section_list.end(); i++) {
		const serifu_part *p = *i;
		p->draw(x, y);
		x += p->get_width();
	}
}

serifu_kana_part::serifu_kana_part()
: kana_font(font_cache["data/fonts/small_font.fnt"])
{ }

int
serifu_kana_part::get_width() const
{
	return kana_font->get_string_width(&kana[0], kana.size());
}

void
serifu_kana_part::draw(float x, float y) const
{
	static gl_vertex_array_texuv gv(256);

	gv.reset();
	gv.add_stringn(kana_font, &kana[0], kana.size(), x, y);

	kana_font->texture.bind();
	gv.draw(GL_QUADS);
}

serifu_furigana_part::serifu_furigana_part()
: kanji_font(font_cache["data/fonts/small_font.fnt"])
, furigana_font(font_cache["data/fonts/tiny_font.fnt"])
{ }

int
serifu_furigana_part::get_width() const
{
	return std::max(kanji_font->get_string_width(&kanji[0], kanji.size()),
	  furigana_font->get_string_width(&furigana[0], furigana.size()));
}

void
serifu_furigana_part::draw(float x, float y) const
{
	const int width = get_width();

	static gl_vertex_array_texuv gv(256);

	gv.reset();
	gv.add_stringn(kanji_font, &kanji[0], kanji.size(),
	  x + .5*width - .5*(kanji_font->get_string_width(&kanji[0], kanji.size())), y);
	kanji_font->texture.bind();
	gv.draw(GL_QUADS);

	gv.reset();
	gv.add_stringn(furigana_font, &furigana[0], furigana.size(),
	  x + .5*width - .5*(furigana_font->get_string_width(&furigana[0], furigana.size())), y + 26);
	furigana_font->texture.bind();
	gv.draw(GL_QUADS);
}

serifu_kana_iterator::serifu_kana_iterator(const serifu *s)
: iter(s->begin())
, end(s->end())
, cur_part_index(0)
{
	skip_non_kana();
}

wchar_t
serifu_kana_iterator::operator*() const
{
	return cur_kana();
}

serifu_kana_iterator&
serifu_kana_iterator::operator++()
{
	next();
	skip_non_kana();

	return *this;
}

void
serifu_kana_iterator::skip_non_kana()
{
	wchar_t ch;

	while ((ch = cur_kana())) {
		if (ch >= 12352 && ch <= 12447)
			break;
		operator++();
	}
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
			++iter;
			cur_part_index = 0;
		}
	}
}
