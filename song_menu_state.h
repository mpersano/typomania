#ifndef SONG_MENU_STATE_H_
#define SONG_MENU_STATE_H_

#include "game.h"

class kashi;

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
	void draw_song_title(const kashi *p) const;

	const kashi_cont& kashi_list;

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
