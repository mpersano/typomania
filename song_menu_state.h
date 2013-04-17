#ifndef SONG_MENU_STATE_H_
#define SONG_MENU_STATE_H_

#include "game.h"

class kashi;

struct menu_item;

class song_menu_state : public state {
public:
	typedef std::vector<kashi *> kashi_cont;

	song_menu_state(const kashi_cont& kashi_list);
	~song_menu_state();

	void redraw() const;
	void update();
	void on_key_up(int keysym);
	void on_key_down(int keysym);

private:
	typedef std::vector<menu_item *> item_cont;
	item_cont item_list;

	enum state {
		STATE_IDLE,
		STATE_MOVING_UP,
		STATE_MOVING_DOWN,
	};

	state cur_state;
	int state_tics;

	int cur_selection;
};

#endif // SONG_SELECTION_STATE_H_
