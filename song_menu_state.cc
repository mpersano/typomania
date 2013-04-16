#include <GL/gl.h>

#include "common.h"
#include "kashi.h"
#include "gl_util.h"
#include "song_menu_state.h"

song_menu_state::song_menu_state(const kashi_cont& kashi_list)
: kashi_list(kashi_list)
{ }

song_menu_state::~song_menu_state()
{ }

void
song_menu_state::redraw() const
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);

	float y = 100;

	for (kashi_cont::const_iterator i = kashi_list.begin(); i != kashi_list.end(); i++) {
		draw_song_title(*i, 0, y);
		y += 80;
	}
}

void
song_menu_state::update()
{
}

void
song_menu_state::on_key_up(int keysym)
{
}

void
song_menu_state::on_key_down(int keysym)
{
}

void
song_menu_state::draw_song_title(const kashi *p, float x, float y) const
{
	static gl_vertex_array_texuv gv(256);

	gv.reset();
	gv.add_string(tiny_font, &p->artist[0], x + 1, y + 24);
	gv.add_string(tiny_font, &p->genre[0], x + 1, y - 20);
	glBindTexture(GL_TEXTURE_2D, tiny_font->texture_id);
	gv.draw(GL_QUADS);

	gv.reset();
	gv.add_string(small_font, &p->name[0], x, y);
	glBindTexture(GL_TEXTURE_2D, small_font->texture_id);
	gv.draw(GL_QUADS);
}
