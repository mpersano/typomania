#ifndef IN_GAME_STATE_H_
#define IN_GAME_STATE_H_

#include "ogg_player.h"
#include "spectrum_bars.h"
#include "game.h"

struct font;

class in_game_state : public state {
public:
	in_game_state(const kashi& cur_kashi);
	~in_game_state();

	void redraw() const;
	void update();
	void on_key_up(int keysym);
	void on_key_down(int keysym);

private:
	void draw_time_bars() const;
	void draw_time_bar(float y, const wchar_t *label, int partial, int total) const;

	void draw_serifu(const kashi::serifu& serifu, int num_consumed, float alpha) const;

	void draw_input_buffer() const;

	void draw_hud_counters() const;
	float draw_hud_counter(float x, float y, const wchar_t *label, bool zero_padded, int num_digits, int value) const;
	float draw_hud_counter(float x, float y, const wchar_t *label, const wchar_t *value) const;

	void draw_song_info() const;

	const kashi& cur_kashi;

	ogg_player player;
	spectrum_bars spectrum;

	int cur_tic;

	kashi::const_iterator cur_serifu;
	int cur_serifu_ms, total_ms;

	int song_duration; // in seconds

	int score, display_score;
	int combo;
	int max_combo;
	int miss;
	int total_strokes;

	font *tiny_font;
	font *small_font;
	font *medium_font;
	font *big_az_font;
};

#endif // IN_GAME_STATE_H_
