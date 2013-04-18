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
	{ L'さ', "SA" }, { L'し', "SI" }, { L'す', "SU" }, { L'せ', "SE" }, { L'そ', "SO" },
	{ L'た', "TA" }, { L'ち', "TI" }, { L'つ', "TU" }, { L'て', "TE" }, { L'と', "TO" },
	{ L'な', "NA" }, { L'に', "NI" }, { L'ぬ', "NU" }, { L'ね', "NE" }, { L'の', "NO" },
	{ L'は', "HA" }, { L'ひ', "HI" }, { L'ふ', "HU" }, { L'へ', "HE" }, { L'ほ', "HO" },
	{ L'ま', "MA" }, { L'み', "MI" }, { L'む', "MU" }, { L'め', "ME" }, { L'も', "MO" },
	{ L'や', "YA" }, { L'ゆ', "YU" }, { L'よ', "YO" },
	{ L'ら', "RA" }, { L'り', "RI" }, { L'る', "RU" }, { L'れ', "RE" }, { L'ろ', "RO" },
	{ L'わ', "WA" }, { L'を', "WO" },
	{ L'ん', "N"  }, { L'っ', "T"  },
	{ L'が', "GA" }, { L'ぎ', "GI" }, { L'ぐ', "GU" }, { L'げ', "GE" }, { L'ご', "GO" },
	{ L'ざ', "ZA" }, { L'じ', "ZI" }, { L'ず', "ZU" }, { L'ぜ', "ZE" }, { L'ぞ', "ZO" },
	{ L'だ', "DA" }, { L'ぢ', "DI" }, { L'づ', "DU" }, { L'で', "DE" }, { L'ど', "DO" },
	{ L'ば', "BA" }, { L'び', "BI" }, { L'ぶ', "BU" }, { L'べ', "BE" }, { L'ぼ', "BO" },
	{ L'ぱ', "PA" }, { L'ぴ', "PI" }, { L'ぷ', "PU" }, { L'ぺ', "PE" }, { L'ぽ', "PO" },
	{ 0, 0 },
};

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
, cur_input_index(0)
, cur_input_part_index(0)
, small_font(font_cache["data/fonts/small_font.fnt"])
, tiny_font(font_cache["data/fonts/tiny_font.fnt"])
, big_az_font(font_cache["data/fonts/big_az_font.fnt"])
{
	std::ostringstream path;
	path << STREAM_DIR << '/' << cur_kashi.stream;

	player.open(path.str());
	player.start(.1);

	spectrum.update(0);
}

in_game_state::~in_game_state()
{ }

void
in_game_state::redraw() const
{
	spectrum.draw();

	draw_time_bars();
	draw_serifu();
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

			cur_input_index = cur_input_part_index = 0;

			// TODO: increase completed
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

	if (keysym >= 'a' && keysym <= 'z')
		keysym += 'A' - 'a';

	const wchar_t *kana = &cur_serifu->kana[cur_input_index];
	const char *input = get_input_for(*kana);

	if (input) {
		if (keysym == input[cur_input_part_index]) {
			if (!input[++cur_input_part_index]) {
				++cur_input_index;
				cur_input_part_index = 0;
			}

			// TODO: increase score, combo
		} else {
			// TODO: increase miss

			fprintf(stderr, "miss!\n");
		}
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
in_game_state::draw_serifu() const
{
	if (cur_serifu == cur_kashi.end())
		return;

	const float base_x = 20;

	static gl_vertex_array_texuv gv(256);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);

	tiny_font->texture->bind();

	glColor3f(1, 1, 0);
	gv.reset();
	float next_x = gv.add_stringn(tiny_font, &cur_serifu->kana[0], cur_input_index, base_x, 96);
	gv.draw(GL_QUADS);

	glColor3f(1, 1, 1);
	gv.reset();
	gv.add_string(tiny_font, &cur_serifu->kana[cur_input_index], next_x, 96);
	gv.draw(GL_QUADS);

	small_font->texture->bind();

	glColor3f(1, 1, 1);
	gv.reset();
	gv.add_string(small_font, &cur_serifu->kanji[0], base_x, 70);
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

	const wchar_t *kana = &cur_serifu->kana[cur_input_index];
	const char *input = get_input_for(*kana);

	if (input) {
		float x = base_x;

		input += cur_input_part_index;

		gv.reset();

		gv.add_glyph(big_az_font->find_glyph(*input++), x, big_y);
		big_az_font->texture->bind();
		gv.draw(GL_QUADS);

		x += big_glyph->advance_x;

		gv.reset();

		for (;;) {
			if (!*input) {
				if (!*++kana)
					break;

				input = get_input_for(*kana);
			}

			const font::glyph *g = small_font->find_glyph(*input++);
			gv.add_glyph(g, x, small_y);
			x += g->advance_x;
		}

		small_font->texture->bind();
		gv.draw(GL_QUADS);
	}
}
