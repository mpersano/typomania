#include <cassert>

#include <sstream>

#include <SDL.h>
#include <GL/gl.h>

#include "common.h"
#include "gl_vertex_array.h"
#include "in_game_state.h"

static const char *STREAM_DIR = "data/streams";

static const struct kana_to_romaji {
	const wchar_t kana;
	const char *romaji;
} kana_to_romaji_table[] = {
	{ L'あ', "A"  }, { L'い', "I"  }, { L'う', "U"  }, { L'え', "E"  }, { L'お', "O"  },
	{ L'か', "KA" }, { L'き', "KI" }, { L'く', "KU" }, { L'け', "KE" }, { L'こ', "KO" },
	{ L'さ', "SA" }, { L'し', "SI|SHI" }, { L'す', "SU" }, { L'せ', "SE" }, { L'そ', "SO" },
	{ L'た', "TA" }, { L'ち', "TI|CHI" }, { L'つ', "TU|TSU" }, { L'て', "TE" }, { L'と', "TO" },
	{ L'な', "NA" }, { L'に', "NI" }, { L'ぬ', "NU" }, { L'ね', "NE" }, { L'の', "NO" },
	{ L'は', "HA" }, { L'ひ', "HI" }, { L'ふ', "HU|FU" }, { L'へ', "HE" }, { L'ほ', "HO" },
	{ L'ま', "MA" }, { L'み', "MI" }, { L'む', "MU" }, { L'め', "ME" }, { L'も', "MO" },
	{ L'や', "YA" }, { L'ゆ', "YU" }, { L'よ', "YO" },
	{ L'ら', "RA" }, { L'り', "RI" }, { L'る', "RU" }, { L'れ', "RE" }, { L'ろ', "RO" },
	{ L'わ', "WA" }, { L'を', "WO" },
	{ L'ん', "N"  }, { L'っ', "T"  },
	{ L'が', "GA" }, { L'ぎ', "GI" }, { L'ぐ', "GU" }, { L'げ', "GE" }, { L'ご', "GO" },
	{ L'ざ', "ZA" }, { L'じ', "ZI|JI" }, { L'ず', "ZU" }, { L'ぜ', "ZE" }, { L'ぞ', "ZO" },
	{ L'だ', "DA" }, { L'ぢ', "DI" }, { L'づ', "DU|ZU" }, { L'で', "DE" }, { L'ど', "DO" },
	{ L'ば', "BA" }, { L'び', "BI" }, { L'ぶ', "BU" }, { L'べ', "BE" }, { L'ぼ', "BO" },
	{ L'ぱ', "PA" }, { L'ぴ', "PI" }, { L'ぷ', "PU" }, { L'ぺ', "PE" }, { L'ぽ', "PO" },
	{ 0, 0 },
};

struct trie {
	trie();
	~trie();

	static trie *initialize_from(const char *str);

	bool is_terminator;
	trie *next['Z' - 'A' + 1];
};

trie::trie()
: is_terminator(false)
{
	memset(next, 0, sizeof next);
}

trie::~trie()
{
	for (size_t i = 0; i < 'Z' - 'A' + 1; i++) {
		if (next[i])
			delete next[i];
	}
}

trie *
trie::initialize_from(const char *str)
{
	trie *root = 0, **trie_ptr = &root;

	for (;;) {
		const char ch = *str++;
		assert(ch == '\0' || ch == '|' || (ch >= 'A' || ch <= 'Z'));

		trie *q = *trie_ptr;
		if (!q)
			q = *trie_ptr = new trie;

		q->is_terminator = (ch == '\0' || ch == '|');
		trie_ptr = !q->is_terminator ? &q->next[ch - 'A'] : &root;

		if (ch == '\0')
			break;
	}

	return root;
}

class kana_consumer {
public:
	kana_consumer();
	~kana_consumer();

	void set_cur_serifu(const wchar_t *serifu);
	bool on_key_down(int keysym);

	bool finished() const
	{ return !cur_trie; }

	int get_num_consumed() const
	{ return cur_serifu_index - 1; }

private:
	void consume_kana();

	const wchar_t *cur_serifu;
	const trie *cur_trie;

	int cur_serifu_index;

	typedef std::map<wchar_t, trie *> kana_trie_cont;
	kana_trie_cont kana_trie_map;
};

kana_consumer::kana_consumer()
: cur_serifu(0)
, cur_trie(0)
, cur_serifu_index(0)
{
	for (const kana_to_romaji *p = kana_to_romaji_table; p->kana; p++)
		kana_trie_map[p->kana] = trie::initialize_from(p->romaji);
}

kana_consumer::~kana_consumer()
{
	for (kana_trie_cont::iterator i = kana_trie_map.begin(); i != kana_trie_map.end(); i++)
		delete i->second;
}

bool
kana_consumer::on_key_down(int keysym)
{
	assert(cur_trie);

	if (keysym >= 'a' && keysym <= 'z')
		keysym += 'A' - 'a';

	if (keysym < 'A' || keysym > 'Z')
		return false;

	const trie *next = cur_trie->next[keysym - 'A'];

	if (!next)
		return false;

	if (next->is_terminator)
		consume_kana();
	else
		cur_trie = next;

	return true;
}

void
kana_consumer::set_cur_serifu(const wchar_t *serifu)
{
	cur_serifu = serifu;
	cur_serifu_index = 0;
	consume_kana();
}

void
kana_consumer::consume_kana()
{
retry:
	const wchar_t ch = cur_serifu[cur_serifu_index++];

	if (ch == L'\0') {
		cur_trie = 0;
	} else {
		kana_trie_cont::const_iterator i = kana_trie_map.find(ch);

		if (i == kana_trie_map.end())
			goto retry;

		cur_trie = i->second;
	}
}

static kana_consumer input_consumer;

static const char *
get_input_for(const wchar_t kana)
{
	// TODO: replace this linear search with map

	// kana?

	for (const kana_to_romaji *p = kana_to_romaji_table; p->kana; p++) {
		if (p->kana == kana)
			return p->romaji;
	}

	// TODO: ascii

	return 0;
}

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

	input_consumer.set_cur_serifu(&cur_serifu->kana[0]);
}

in_game_state::~in_game_state()
{ }

void
in_game_state::redraw() const
{
	spectrum.draw();

	draw_time_bars();

	if (input_consumer.finished()) {
		kashi::const_iterator next_serifu = cur_serifu + 1;

		if (next_serifu != cur_kashi.end())
			draw_serifu(*next_serifu, 0, .5);
	} else if (cur_serifu != cur_kashi.end()) {
		draw_serifu(*cur_serifu, input_consumer.get_num_consumed(), 1);
	}

	draw_input_queue();
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

			input_consumer.set_cur_serifu(&cur_serifu->kana[0]);
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

	if (!input_consumer.finished()) {
		if (!input_consumer.on_key_down(keysym))
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

	if (serifu.kana[cur_input_index]) {
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
in_game_state::draw_input_queue() const
{
	if (cur_serifu == cur_kashi.end())
		return;

	const float base_y = 30;
	const float base_x = 20;

	const font::glyph *big_glyph = big_az_font->find_glyph(L'X');
	const float big_y = base_y + .5*big_glyph->height - big_glyph->top;

	const font::glyph *small_glyph = small_font->find_glyph(L'X');
	const float small_y = base_y + .5*small_glyph->height - small_glyph->top;

	static gl_vertex_array_texuv gv(256);

	const int cur_input_index = input_consumer.get_num_consumed();

	const wchar_t *kana = &cur_serifu->kana[cur_input_index];
	const char *input = get_input_for(*kana);

	if (input) {
		float x = base_x;

		gv.reset();

		gv.add_glyph(big_az_font->find_glyph(*input++), x, big_y);
		big_az_font->texture->bind();
		gv.draw(GL_QUADS);

		x += big_glyph->advance_x;

		gv.reset();

		for (;;) {
			if (*input == '\0' || *input == '|') {
				if (!*++kana)
					break;

				input = get_input_for(*kana);
			}

			if (!input)
				break;

			const font::glyph *g = small_font->find_glyph(*input++);
			gv.add_glyph(g, x, small_y);
			x += g->advance_x;
		}

		small_font->texture->bind();
		gv.draw(GL_QUADS);
	}
}
