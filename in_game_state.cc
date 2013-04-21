#include <cassert>

#include <sstream>

#include <algorithm>

#include <SDL.h>
#include <GL/gl.h>

#include "common.h"
#include "gl_vertex_array.h"
#include "in_game_state.h"

static const char *STREAM_DIR = "data/streams";

struct pattern_node {
	pattern_node() : is_optional(false), next(0) { }
	virtual ~pattern_node() { }

	virtual bool match(int keysym) const = 0;
	virtual int get_char() const = 0;

	bool is_optional;
	pattern_node *next;
};

struct single_char_node : pattern_node {
	single_char_node(int ch) : ch(ch) { }

	bool match(int keysym) const
	{ return ch == keysym; }

	int get_char() const
	{ return ch; }

	int ch;
};

struct multi_char_node : pattern_node {
	bool match(int keysym) const
	{ return std::find(char_list.begin(), char_list.end(), keysym) != char_list.end(); }

	int get_char() const
	{ return char_list[0]; }

	std::vector<int> char_list;
};

static pattern_node *
parse_pattern(const char *pattern_str)
{
	pattern_node *head = 0, **node_ptr = &head;

	while (*pattern_str) {
		pattern_node *node;

		int ch = *pattern_str++;

		if (ch == '[') {
			node = new multi_char_node;

			while ((ch = *pattern_str++) != ']' && ch != '\0')
				dynamic_cast<multi_char_node *>(node)->char_list.push_back(ch);
		} else {
			node = new single_char_node(ch);
		}

		if (*pattern_str == '?') {
			node->is_optional = true;
			++pattern_str;
		}

		*node_ptr = node;
		node_ptr = &node->next;
	}

	return head;
}

class kana_to_pattern {
public:
	static pattern_node *find(const wchar_t ch);

private:
	kana_to_pattern();
	~kana_to_pattern();

	typedef std::map<wchar_t, pattern_node *> map;
	map kana_to_pattern_map;
};

kana_to_pattern::kana_to_pattern()
{
	static const struct kana_to_romaji {
		const wchar_t kana;
		const char *romaji;
	} kana_to_romaji_table[] = {
		{ L'あ', "A"  }, { L'い', "I"  }, { L'う', "U"  }, { L'え', "E"  }, { L'お', "O"  },
		{ L'か', "KA" }, { L'き', "KI" }, { L'く', "KU" }, { L'け', "KE" }, { L'こ', "KO" },
		{ L'さ', "SA" }, { L'し', "SH?I" }, { L'す', "SU" }, { L'せ', "SE" }, { L'そ', "SO" },
		{ L'た', "TA" }, { L'ち', "[TC]H?I" }, { L'つ', "TS?U" }, { L'て', "TE" }, { L'と', "TO" },
		{ L'な', "NA" }, { L'に', "NI" }, { L'ぬ', "NU" }, { L'ね', "NE" }, { L'の', "NO" },
		{ L'は', "HA" }, { L'ひ', "HI" }, { L'ふ', "[HF]U" }, { L'へ', "HE" }, { L'ほ', "HO" },
		{ L'ま', "MA" }, { L'み', "MI" }, { L'む', "MU" }, { L'め', "ME" }, { L'も', "MO" },
		{ L'や', "YA" }, { L'ゆ', "YU" }, { L'よ', "YO" },
		{ L'ら', "RA" }, { L'り', "RI" }, { L'る', "RU" }, { L'れ', "RE" }, { L'ろ', "RO" },
		{ L'わ', "WA" }, { L'を', "WO" },
		{ L'ん', "N"  }, { L'っ', "T"  },
		{ L'が', "GA" }, { L'ぎ', "GI" }, { L'ぐ', "GU" }, { L'げ', "GE" }, { L'ご', "GO" },
		{ L'ざ', "ZA" }, { L'じ', "[ZJ]I" }, { L'ず', "ZU" }, { L'ぜ', "ZE" }, { L'ぞ', "ZO" },
		{ L'だ', "DA" }, { L'ぢ', "DI" }, { L'づ', "[DZ]U" }, { L'で', "DE" }, { L'ど', "DO" },
		{ L'ば', "BA" }, { L'び', "BI" }, { L'ぶ', "BU" }, { L'べ', "BE" }, { L'ぼ', "BO" },
		{ L'ぱ', "PA" }, { L'ぴ', "PI" }, { L'ぷ', "PU" }, { L'ぺ', "PE" }, { L'ぽ', "PO" },
		{ 0, 0 },
	};

	for (const kana_to_romaji *p = kana_to_romaji_table; p->kana; p++)
		kana_to_pattern_map[p->kana] = parse_pattern(p->romaji);
}

kana_to_pattern::~kana_to_pattern()
{
	for (map::iterator i = kana_to_pattern_map.begin(); i != kana_to_pattern_map.end(); i++)
		delete i->second;
}

pattern_node *
kana_to_pattern::find(const wchar_t ch)
{
	static kana_to_pattern instance;

	map::iterator i = instance.kana_to_pattern_map.find(ch);

	return i == instance.kana_to_pattern_map.end() ? 0 : i->second;
}

struct romaji_iterator {
	romaji_iterator(const pattern_node *cur_pattern, const wchar_t *kana)
	: cur_pattern(cur_pattern), kana(kana)
	{ skip_optional(); }

	bool operator!() const
	{
		return !!cur_pattern;
	}

	operator bool() const
	{ return cur_pattern; }

	romaji_iterator& operator++()
	{
		if (cur_pattern) {
			if (!(cur_pattern = cur_pattern->next))
				cur_pattern = kana_to_pattern::find(*kana++);

			skip_optional();
		}

		return *this;
	}

	char operator*() const
	{ return cur_pattern ? cur_pattern->get_char() : 0; }

	void skip_optional()
	{
		while (cur_pattern && cur_pattern->is_optional) {
			if (!(cur_pattern = cur_pattern->next))
				cur_pattern = kana_to_pattern::find(*kana++);
		}
	}

	const pattern_node *cur_pattern;
	const wchar_t *kana;
};

class kana_buffer {
public:
	kana_buffer()
	: kana(0), kana_index(0), cur_pattern(0)
	{ }

	void set_kana(const wchar_t *p);

	bool on_key_down(int keysym);

	bool finished() const
	{ return !cur_pattern; }

	int get_num_consumed() const
	{ return kana_index - 1; }

	romaji_iterator get_romaji_iterator() const
	{ return romaji_iterator(cur_pattern, &kana[kana_index]); }

	void draw(const font *big_font, const font *small_font) const;

private:
	void consume_kana();

	const wchar_t *kana;
	int kana_index;
	const pattern_node *cur_pattern;
};

bool
kana_buffer::on_key_down(int keysym)
{
	assert(cur_pattern);

	if (keysym >= 'a' && keysym <= 'z')
		keysym += 'A' - 'a';

	if (!cur_pattern->match(keysym)) {
		if (!cur_pattern->is_optional) {
			return false;
		} else {
			// failed to match but was optional, so try next one

			if (cur_pattern->next && cur_pattern->next->match(keysym)) {
				cur_pattern = cur_pattern->next;
			} else {
				return false;
			}
		}
	}

	if (!(cur_pattern = cur_pattern->next))
		consume_kana();

	return true;
}

void
kana_buffer::set_kana(const wchar_t *p)
{
	kana = p;
	kana_index = 0;
	consume_kana();
}

void
kana_buffer::consume_kana()
{
retry:
	const wchar_t ch = kana[kana_index++];

	if (ch == L'\0') {
		cur_pattern = 0;
	} else {
		if (!(cur_pattern = kana_to_pattern::find(ch)))
			goto retry;
	}
}

void
kana_buffer::draw(const font *big_font, const font *small_font) const
{
	if (!cur_pattern)
		return;

	const float base_y = 30;
	const float base_x = 20;

	const font::glyph *big_glyph = big_font->find_glyph(L'X');
	const float big_y = base_y + .5*big_glyph->height - big_glyph->top;

	const font::glyph *small_glyph = small_font->find_glyph(L'X');
	const float small_y = base_y + .5*small_glyph->height - small_glyph->top;

	static gl_vertex_array_texuv gv(256);

	bool is_first = true;

	const pattern_node *p = cur_pattern;
	const wchar_t *q = &kana[kana_index];

	float x = base_x;

	for (;;) {
		const int ch = p->get_char();

		if (is_first) {
			gv.reset();
			gv.add_glyph(big_font->find_glyph(ch), x, big_y);
			big_font->texture->bind();
			gv.draw(GL_QUADS);

			x += big_glyph->advance_x;

			is_first = false;
		} else {
			const font::glyph *g = small_font->find_glyph(ch);

			gv.reset();
			gv.add_glyph(small_font->find_glyph(ch), x, small_y);
			small_font->texture->bind();
			gv.draw(GL_QUADS);

			x += g->advance_x;
		}

		if (!(p = p->next)) {
			const wchar_t kana = *q++;

			if (!kana)
				break;

			p = kana_to_pattern::find(kana);
		}
	}
}

static kana_buffer input_buffer;

in_game_state::in_game_state(const kashi& cur_kashi)
: cur_kashi(cur_kashi)
, spectrum(player)
, cur_tic(0)
, cur_serifu(cur_kashi.begin())
, cur_serifu_ms(0)
, small_font(font_cache["data/fonts/small_font.fnt"])
, tiny_font(font_cache["data/fonts/tiny_font.fnt"])
, big_az_font(font_cache["data/fonts/big_az_font.fnt"])
{
	std::ostringstream path;
	path << STREAM_DIR << '/' << cur_kashi.stream;

	player.open(path.str());
	player.start(.1);

	spectrum.update(0);

	input_buffer.set_kana(&cur_serifu->kana[0]);
}

in_game_state::~in_game_state()
{ }

void
in_game_state::redraw() const
{
	spectrum.draw();

	draw_time_bars();

	if (input_buffer.finished()) {
		kashi::const_iterator next_serifu = cur_serifu + 1;

		if (next_serifu != cur_kashi.end())
			draw_serifu(*next_serifu, 0, .5);
	} else if (cur_serifu != cur_kashi.end()) {
		draw_serifu(*cur_serifu, input_buffer.get_num_consumed(), 1);
	}

	draw_input_buffer();
}

void
in_game_state::update()
{
	++cur_tic;

	player.update();
	spectrum.update(cur_tic);

	if (cur_serifu != cur_kashi.end()) {
		cur_serifu_ms += 1000/TICS_PER_SECOND;

		const int duration = cur_serifu->duration;

		if (cur_serifu_ms >= duration) {
			cur_serifu_ms -= duration;
			++cur_serifu;

			input_buffer.set_kana(&cur_serifu->kana[0]);
		}
	}
}

void
in_game_state::on_key_up(int keysym)
{ }

void
in_game_state::on_key_down(int keysym)
{
	if (keysym == SDLK_ESCAPE) {
		the_game->start_song_menu();
		return;
	}

	if (!input_buffer.finished()) {
		if (!input_buffer.on_key_down(keysym))
			fprintf(stderr, "miss!\n");
	}
}

void
in_game_state::draw_time_bars() const
{
	if (cur_serifu == cur_kashi.end())
		return;

	const kashi::serifu& serifu = *cur_serifu;

	const float x0 = 8, x1 = WINDOW_WIDTH - 8, xm = x0 + (x1 - x0)*cur_serifu_ms/serifu.duration;
	const float y0 = 120, y1 = 125;

	glDisable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);

	glColor3f(1, 1, 1);
	glVertex2f(x0, y0);
	glVertex2f(x0, y1);
	glVertex2f(xm, y1);
	glVertex2f(xm, y0);

	glColor3f(.5, .5, .5);
	glVertex2f(xm, y0);
	glVertex2f(xm, y1);
	glVertex2f(x1, y1);
	glVertex2f(x1, y0);

	glEnd();

	// TODO: total time bar
}

void
in_game_state::draw_serifu(const kashi::serifu& serifu, int num_consumed, float alpha) const
{
	const float base_x = 20;

	static gl_vertex_array_texuv gv(256);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);

	// kana

	tiny_font->texture->bind();

	glColor4f(1, 1, 0, alpha);
	gv.reset();
	float next_x = gv.add_stringn(tiny_font, &serifu.kana[0], num_consumed, base_x, 96);
	gv.draw(GL_QUADS);

	if (serifu.kana[num_consumed]) {
		glColor4f(1, 1, 1, alpha);
		gv.reset();
		gv.add_string(tiny_font, &serifu.kana[num_consumed], next_x, 96);
		gv.draw(GL_QUADS);
	}

	// kanji

	small_font->texture->bind();

	glColor4f(1, 1, 1, alpha);
	gv.reset();
	gv.add_string(small_font, &serifu.kanji[0], base_x, 70);
	gv.draw(GL_QUADS);
}

void
in_game_state::draw_input_buffer() const
{
	const float base_y = 30;
	const float base_x = 20;

	const font::glyph *big_glyph = big_az_font->find_glyph(L'X');
	const float big_y = base_y + .5*big_glyph->height - big_glyph->top;

	const font::glyph *small_glyph = small_font->find_glyph(L'X');
	const float small_y = base_y + .5*small_glyph->height - small_glyph->top;

	static gl_vertex_array_texuv gv(256);

	bool is_first = true;

	float x = base_x;

	for (romaji_iterator iter = input_buffer.get_romaji_iterator(); iter; ++iter) {
		const int ch = *iter;

		if (is_first) {
			gv.reset();
			gv.add_glyph(big_az_font->find_glyph(ch), x, big_y);
			big_az_font->texture->bind();
			gv.draw(GL_QUADS);

			x += big_glyph->advance_x;

			is_first = false;

			gv.reset();
		} else {
			const font::glyph *g = small_font->find_glyph(ch);
			gv.add_glyph(small_font->find_glyph(ch), x, small_y);

			x += g->advance_x;
		}
	}

	small_font->texture->bind();
	gv.draw(GL_QUADS);
}
