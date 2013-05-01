#ifndef IN_GAME_STATE_H_
#define IN_GAME_STATE_H_

#include "ogg_player.h"
#include "spectrum_bars.h"
#include "game.h"

class in_game_state : public game_state {
public:
	in_game_state(const kashi& cur_kashi);
	~in_game_state();

	void redraw() const;
	void update();
	void on_key_up(int keysym);
	void on_key_down(int keysym);

private:
	void draw_song_info() const;

	void draw_hud(float alpha) const;
	void draw_time_bars(float alpha) const;
	void draw_time_bar(float y, const wchar_t *label, int partial, int total, float alpha) const;
	void draw_timers(float alpha) const;
	void draw_serifu(float alpha) const;
	void draw_input_buffer(float alpha) const;
	void draw_hud_counters(float alpha) const;
	float draw_hud_counter(float x, float y, const wchar_t *label, bool zero_padded, int num_digits, int value) const;
	float draw_hud_counter(float x, float y, const wchar_t *label, const wchar_t *value) const;

	const wchar_t *get_correct_percent() const;
	const wchar_t *get_class() const;

	void draw_results(int tic) const;

	const kashi& cur_kashi;

	enum state {
		PLAYING,
		OUTRO,
	};

	state cur_state;
	void set_state(state next_state);

	int state_tics;

#ifndef MUTE
	ogg_player player;
	spectrum_bars spectrum;
#endif

	kashi::const_iterator cur_serifu;

	unsigned start_ms, start_serifu_ms;
	unsigned total_ms, serifu_ms;

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
