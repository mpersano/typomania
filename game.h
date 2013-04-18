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

	void start_in_game(const kashi& cur_kashi);
	void start_song_menu();

private:
	void load_song_list();

	std::auto_ptr<state> cur_state;

	typedef std::vector<kashi *> kashi_cont;
	kashi_cont kashi_list;

	game(const game&);
	game& operator=(const game&);
};

typedef std::auto_ptr<game> game_ptr;

#endif // GAME_H_
