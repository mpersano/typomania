#ifndef GAME_H_
#define GAME_H_

#include <memory>
#include <vector>

#include "kashi.h"

class state {
public:
	virtual ~state() { }

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

private:
	void load_song_list();

	void start_in_game_state(const kashi& cur_kashi);

	std::auto_ptr<state> cur_state;
	std::vector<kashi *> kashi_list;

	game(const game&);
	game& operator=(const game&);
};

#endif // GAME_H_
