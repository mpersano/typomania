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
	virtual ~pattern_node() { if (next) delete next; }

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
	romaji_iterator(const pattern_node *pattern, const wchar_t *kana)
	: cur_pattern(pattern), kana(kana)
	{
		while (cur_pattern && cur_pattern->is_optional)
			next_pattern();
	}

	romaji_iterator& operator++()
	{
		if (cur_pattern) {
			next_pattern();
			while (cur_pattern && cur_pattern->is_optional)
				next_pattern();
		}

		return *this;
	}

	char operator*() const
	{ return cur_pattern ? cur_pattern->get_char() : 0; }

	void next_pattern()
	{
		if (!(cur_pattern = cur_pattern->next))
			cur_pattern = kana_to_pattern::find(*kana++);
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
	const wchar_t ch = kana[kana_index++];
	cur_pattern = ch == L'\0' ? 0 : kana_to_pattern::find(ch);
}

static kana_buffer input_buffer;

in_game_state::in_game_state(const kashi& cur_kashi)
: cur_kashi(cur_kashi)
, spectrum(player, 100, 50, 800, 128, 64)
, cur_tic(0)
, cur_serifu(cur_kashi.begin())
, cur_serifu_ms(0)
, total_ms(0)
, score(0)
, display_score(0)
, combo(0)
, max_combo(0)
, miss(0)
, total_strokes(0)
, tiny_font(font_cache["data/fonts/tiny_font.fnt"])
, small_font(font_cache["data/fonts/small_font.fnt"])
, medium_font(font_cache["data/fonts/medium_font.fnt"])
, big_az_font(font_cache["data/fonts/big_az_font.fnt"])
{
	std::ostringstream path;
	path << STREAM_DIR << '/' << cur_kashi.stream;

	player.open(path.str());

	song_duration = static_cast<int>(player.get_track_duration()*1000);

	player.start(.1);

	spectrum.update(0);

	input_buffer.set_kana(&cur_serifu->kana[0]);
}

in_game_state::~in_game_state()
{ }

void
in_game_state::redraw() const
{
	glColor4f(1, 1, 1, .1);
	spectrum.draw();

	draw_time_bars();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);

	if (input_buffer.finished()) {
		kashi::const_iterator next_serifu = cur_serifu + 1;

		if (next_serifu != cur_kashi.end())
			draw_serifu(*next_serifu, 0, .5);
	} else if (cur_serifu != cur_kashi.end()) {
		draw_serifu(*cur_serifu, input_buffer.get_num_consumed(), 1);
	}

	draw_input_buffer();
	draw_hud_counters();
	draw_song_info();
}

void
in_game_state::update()
{
	++cur_tic;

	player.update();
	spectrum.update(cur_tic);

	total_ms += 1000/TICS_PER_SECOND;

	if (cur_serifu != cur_kashi.end()) {
		cur_serifu_ms += 1000/TICS_PER_SECOND;

		const int duration = cur_serifu->duration;

		if (cur_serifu_ms >= duration) {
			cur_serifu_ms -= duration;

			if (++cur_serifu != cur_kashi.end())
				input_buffer.set_kana(&cur_serifu->kana[0]);
		}
	}

	display_score += (score - display_score)/2;
	if (abs(display_score - score) == 1)
		display_score = score;
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
		++total_strokes;

		if (!input_buffer.on_key_down(keysym)) {
			score -= 601;
			combo = 0;
			++miss;
		} else {
			score += 311;
			if (++combo > max_combo)
				max_combo = combo;
		}
	}
}

static float
draw_integer_r(gl_vertex_array_texuv& gv, const font *f, float x, float y, float w, bool zero_padded, int num_digits, int value)
{
	gv.add_glyph(f->find_glyph(value%10 + L'0'), x, y);

	if (value/10 || (zero_padded && num_digits > 1))
		return draw_integer_r(gv, f, x - w, y, w, zero_padded, num_digits - 1, value/10);
	else
		return x - w;
}

static void
draw_integer(const font *f, float base_x, float base_y, bool zero_padded, int num_digits, int value)
{
	static gl_vertex_array_texuv gv(256);
	gv.reset();

	const font::glyph *g = f->find_glyph(L'0');
	base_y += .5*g->height - g->top;

	const float x = draw_integer_r(gv, f, base_x, base_y, g->advance_x, zero_padded, num_digits, abs(value));

	if (value < 0)
		gv.add_glyph(f->find_glyph(L'-'), x, base_y);

	f->texture->bind();
	gv.draw(GL_QUADS);
}

static void
draw_string(const font *f, float x, float y, const wchar_t *str)
{
	static gl_vertex_array_texuv gv(256);
	gv.reset();

	const font::glyph *g = f->find_glyph(L'X');
	gv.add_string(f, str, x, y + .5*g->height - g->top);

	f->texture->bind();
	gv.draw(GL_QUADS);
}

void
in_game_state::draw_time_bar(float y, const wchar_t *label, int partial, int total) const
{
	const float x = 120;
	const float w = WINDOW_WIDTH - x - 8;
	const float h = 5;

	const float x0 = x, x1 = x + w, xm = x0 + (x1 - x0)*partial/total;
	const float y0 = y - .5*h, y1 = y + .5*h;

	glColor4f(1, 1, 1, 1);
	glEnable(GL_TEXTURE_2D);
	draw_string(tiny_font, x - tiny_font->get_string_width(label) - 8, y, label);

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

}

void
in_game_state::draw_time_bars() const
{
	if (cur_serifu == cur_kashi.end())
		return;

	const kashi::serifu& serifu = *cur_serifu;

	draw_time_bar(170, L"INTERVAL", cur_serifu_ms, serifu.duration);
	draw_time_bar(190, L"TOTAL TIME", total_ms, song_duration);
}

void
in_game_state::draw_serifu(const kashi::serifu& serifu, int num_consumed, float alpha) const
{
	const float base_x = 100;

	static gl_vertex_array_texuv gv(256);

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

	for (romaji_iterator iter = input_buffer.get_romaji_iterator(); *iter; ++iter) {
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

float 
in_game_state::draw_hud_counter(float x, float y, const wchar_t *label, bool zero_padded, int num_digits, int value) const
{
	draw_string(tiny_font, x, y, label);
	x += tiny_font->get_string_width(label) + small_font->find_glyph(L'0')->advance_x*num_digits;
	draw_integer(small_font, x, y, zero_padded, num_digits, value);

	return x;
}

float 
in_game_state::draw_hud_counter(float x, float y, const wchar_t *label, const wchar_t *value) const
{
	draw_string(tiny_font, x, y, label);
	x += tiny_font->get_string_width(label) + small_font->find_glyph(L'0')->advance_x;
	draw_string(small_font, x, y, value);

	return x;
}

void
in_game_state::draw_hud_counters() const
{
	const float base_y = 140;

	glColor4f(1, 1, 1, 1);

	if (combo) {
		draw_integer(small_font, 40, base_y, false, 0, combo);
		draw_string(tiny_font, 60, base_y, L"COMBO");
	}

	float x = 140;
	x = draw_hud_counter(x, base_y, L"SCORE", false, 7, display_score) + 50;
	x = draw_hud_counter(x, base_y, L"MAX COMBO", true, 3, max_combo) + 50;
	x = draw_hud_counter(x, base_y, L"MISS", true, 3, miss) + 50;

	wchar_t buf[30] = {0};
	if (total_strokes > 0) {
		if (miss == 0) {
			wcscpy(buf, L"100%");
		} else {
			const float f = 100.*static_cast<float>(total_strokes - miss)/total_strokes;
			swprintf(buf, sizeof buf/sizeof *buf, L"%.1f%%", f);
		}
	}
	draw_hud_counter(x, base_y, L"CORRECT", buf);
}

void
in_game_state::draw_song_info() const
{
	draw_string(medium_font, 30, 320, &cur_kashi.name[0]);
	draw_string(tiny_font, 36, 290, &cur_kashi.genre[0]);
	draw_string(tiny_font, 36, 346, &cur_kashi.artist[0]);
}
