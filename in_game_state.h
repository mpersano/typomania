#ifndef IN_GAME_STATE_H_
#define IN_GAME_STATE_H_

#include "ogg_player.h"
#include "spectrum_bars.h"
#include "game.h"

class in_game_state : public state {
public:
	in_game_state(const kashi& cur_kashi);
	~in_game_state();

	void redraw() const;
	void update();
	void on_key_up(int keysym);
	void on_key_down(int keysym);

private:
	void draw_serifu() const;

	const kashi& cur_kashi;

	ogg_player player;
	spectrum_bars spectrum;

	int cur_tic;

	std::vector<kashi::serifu>::const_iterator cur_serifu;
	int cur_serifu_ms;
};

#endif // IN_GAME_STATE_H_
