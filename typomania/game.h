#pragma once

#include <memory>
#include <vector>
#include <stack>

#include "kashi.h"

class game_state
{
public:
	virtual ~game_state() { }

	virtual void redraw() const = 0;
	virtual void update() = 0;
	virtual void on_key_up(int keysym) = 0;
	virtual void on_key_down(int keysym) = 0;
};

class game {
public:
	game();

	void redraw() const;
	void update();
	void on_key_up(int keysym);
	void on_key_down(int keysym);

	void push_state(game_state *new_state);
	void pop_state();

private:
	void load_song_list();

	game_state *cur_state();
	const game_state *cur_state() const;

	std::stack<std::unique_ptr<game_state>> state_stack_;
	std::vector<kashi_ptr> kashi_list_;

	game(const game&);
	game& operator=(const game&);
};

typedef std::unique_ptr<game> game_ptr;
