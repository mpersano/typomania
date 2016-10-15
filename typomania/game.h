#pragma once

#include <memory>
#include <vector>
#include <stack>

#include "noncopyable.h"
#include "kashi.h"

class game;

class game_state
{
public:
	game_state(game *parent);
	virtual ~game_state() { }

	virtual void redraw() const = 0;
	virtual void update() = 0;
	virtual void on_key_up(int keysym) = 0;
	virtual void on_key_down(int keysym) = 0;

protected:
	game *parent_;
};

class game : private noncopyable
{
public:
	game(int window_width, int window_height);

	void redraw() const;
	void update();
	void on_key_up(int keysym);
	void on_key_down(int keysym);

	void enter_in_game_state(const kashi& cur_kashi);
	void leave_state();

	int get_window_width() const
	{ return window_width_; }

	int get_window_height() const
	{ return window_height_; }

private:
	void push_state(game_state *new_state);
	void pop_state();

	void load_song_list();

	game_state *cur_state();
	const game_state *cur_state() const;

	int window_width_;
	int window_height_;

	std::stack<std::unique_ptr<game_state>> state_stack_;
	std::vector<kashi_ptr> kashi_list_;
};
