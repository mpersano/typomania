#pragma once

#include "game.h"

class kashi;
class menu_item;

namespace gl {
class texture;
}

using menu_item_ptr = std::unique_ptr<menu_item>;

class song_menu_state : public game_state
{
public:
	song_menu_state(game *parent, const std::vector<kashi_ptr>& kashi_list);
	~song_menu_state();

	void redraw() const override;
	void update() override;
	void on_key_up(int keysym) override;
	void on_key_down(int keysym) override;

private:
	void draw_background() const;

	std::vector<menu_item_ptr> item_list_;

	enum class state {
		IDLE,
		MOVING_UP,
		MOVING_DOWN,
	};

	void set_cur_state(state s);

	state cur_state_;
	int state_tics_;

	int cur_selection_;
	float cur_displayed_position_;

	int move_tics_;

	const gl::texture *arrow_texture_;
	const gl::texture *bg_texture_;
};
