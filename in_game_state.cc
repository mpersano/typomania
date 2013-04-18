#include <sstream>

#include <GL/gl.h>

#include "common.h"
#include "gl_vertex_array.h"
#include "in_game_state.h"

static const char *STREAM_DIR = "data/streams";

static const struct kana_to_romaji {
	const wchar_t kana;
	const char *romaji;
} kana_to_romaji[] = {
	{ L'あ', "a"  }, { L'い', "i"  }, { L'う', "u"  }, { L'え', "e"  }, { L'お', "o"  },
	{ L'か', "ka" }, { L'き', "ki" }, { L'く', "ku" }, { L'け', "ke" }, { L'こ', "ko" },
	{ L'さ', "sa" }, { L'し', "si" }, { L'す', "su" }, { L'せ', "se" }, { L'そ', "so" },
	{ L'た', "ta" }, { L'ち', "ti" }, { L'つ', "tu" }, { L'て', "te" }, { L'と', "to" },
	{ L'な', "na" }, { L'に', "ni" }, { L'ぬ', "nu" }, { L'ね', "ne" }, { L'の', "no" },
	{ L'は', "ha" }, { L'ひ', "hi" }, { L'ふ', "fu" }, { L'へ', "he" }, { L'ほ', "ho" },
	{ L'ま', "ma" }, { L'み', "mi" }, { L'む', "mu" }, { L'め', "me" }, { L'も', "mo" },
	{ L'や', "ya" }, { L'ゆ', "yu" }, { L'よ', "yo" },
	{ L'ら', "ra" }, { L'り', "ri" }, { L'る', "ru" }, { L'れ', "re" }, { L'ろ', "ro" },
	{ L'わ', "wa" }, { L'を', "wo" },
	{ L'が', "ga" }, { L'ぎ', "gi" }, { L'ぐ', "gu" }, { L'げ', "ge" }, { L'ご', "go" },
	{ L'ざ', "za" }, { L'じ', "ji" }, { L'ず', "zu" }, { L'ぜ', "ze" }, { L'ぞ', "zo" },
	{ L'だ', "da" }, { L'ぢ', "di" }, { L'づ', "du" }, { L'で', "de" }, { L'ど', "do" },
	{ L'ば', "ba" }, { L'び', "bi" }, { L'ぶ', "bu" }, { L'べ', "be" }, { L'ぼ', "bo" },
	{ L'ぱ', "pa" }, { L'ぴ', "pi" }, { L'ぷ', "pu" }, { L'ぺ', "pe" }, { L'ぽ', "po" },
	{ 0, 0 },
};

in_game_state::in_game_state(const kashi& cur_kashi)
: cur_kashi(cur_kashi)
, spectrum(player)
, cur_tic(0)
, cur_serifu(cur_kashi.begin())
, cur_serifu_ms(0)
, small_font(font_cache["data/fonts/small_font.fnt"])
, tiny_font(font_cache["data/fonts/tiny_font.fnt"])
{
	std::ostringstream path;
	path << STREAM_DIR << '/' << cur_kashi.stream;

	player.open(path.str());
	player.start(.2);

	spectrum.update(0);
}

in_game_state::~in_game_state()
{ }

void
in_game_state::redraw() const
{
	spectrum.draw();
	draw_serifu();
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
		}
	}
}

void
in_game_state::on_key_up(int keysym)
{ }

void
in_game_state::on_key_down(int keysym)
{ }

void
in_game_state::draw_serifu() const
{
	if (cur_serifu == cur_kashi.end())
		return;

	const kashi::serifu& serifu = *cur_serifu;

	const float x0 = 8, x1 = WINDOW_WIDTH - 8, xm = x0 + (x1 - x0)*cur_serifu_ms/serifu.duration;
	const float y0 = 60, y1 = 65;

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

	glColor3f(1, 1, 0);

	static gl_vertex_array_texuv gv(256);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);

	gv.reset();
	gv.add_string(tiny_font, &serifu.kana[0], 10, 32);
	tiny_font->texture->bind();
	gv.draw(GL_QUADS);

	gv.reset();
	gv.add_string(small_font, &serifu.kanji[0], 10, 10);
	small_font->texture->bind();
	gv.draw(GL_QUADS);
}
