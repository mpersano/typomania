#ifndef GAME_H_
#define GAME_H_

#include <memory>
#include <vector>
#include <stack>

#include "kashi.h"

class game_state {
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
	~game();

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

	std::stack<game_state *> state_stack;

	typedef std::vector<kashi *> kashi_cont;
	kashi_cont kashi_list;

	game(const game&);
	game& operator=(const game&);
};

typedef std::unique_ptr<game> game_ptr;

#endif // GAME_H_
