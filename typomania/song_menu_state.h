#pragma once

#include "game.h"

class kashi;
class menu_item;
class gl_texture;

using menu_item_ptr = std::unique_ptr<menu_item>;

class song_menu_state : public game_state
{
public:
	song_menu_state(const std::vector<kashi_ptr>& kashi_list);
	~song_menu_state();

	void redraw() const;
	void update();
	void on_key_up(int keysym);
	void on_key_down(int keysym);

private:
	void draw_background() const;

	using item_cont = std::vector<menu_item_ptr>;
	item_cont item_list;

	enum state {
		STATE_IDLE,
		STATE_MOVING_UP,
		STATE_MOVING_DOWN,
	};

	void set_cur_state(state s);

	state cur_state;
	int state_tics;

	int cur_selection;
	float cur_displayed_position;

	int move_tics;

	gl_texture *arrow_texture;
	gl_texture *bg_texture;
};
