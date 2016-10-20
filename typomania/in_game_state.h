#pragma once

#include <list>

#include "ogg_player.h"
#include "spectrum_bars.h"
#include "game.h"

namespace gl {
class texture;
class framebuffer;
}

class kana_buffer;

class in_game_state : public game_state
{
public:
	in_game_state(game *parent, const kashi& cur_kashi);
	~in_game_state();

	void redraw() const override;
	void update() override;
	void on_key_up(int keysym) override;
	void on_key_down(int keysym) override;

	void add_glyph_fx(std::unique_ptr<glyph_fx> p);

private:
	void set_cur_serifu(const serifu *s, bool is_last);

	void draw_background(float alpha) const;
	void draw_song_info(float alpha) const;
	void draw_song_info_text(float alpha) const;

	void draw_hud(float alpha) const;
	void draw_time_bars(float alpha) const;
	void draw_time_bar(float y, const wchar_t *label, int partial, int total, float alpha) const;
	void draw_timers(float alpha) const;
	void draw_serifu(float alpha) const;
	void draw_input_buffer(float alpha) const;
	void draw_hud_counters(float alpha) const;
	float draw_hud_counter(float x, float y, const wchar_t *label, bool zero_padded, int num_digits, int value) const;
	float draw_hud_counter(float x, float y, const wchar_t *label, const wchar_t *value) const;

	void draw_glyph_fxs() const;
	void update_glyph_fxs();

	const wchar_t *get_correct_percent() const;
	const wchar_t *get_class() const;

	void draw_results(int tic) const;

	const kashi& cur_kashi;

	enum state {
		INTRO,
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

	unsigned song_duration, cur_serifu_duration; // in ms

	int score, display_score;
	int combo;
	int max_combo;
	int miss;
	int total_strokes;

	const font *tiny_font;
	const font *small_font;
	const font *medium_font;
	const font *big_az_font;

	std::unique_ptr<kana_buffer> input_buffer_;
	std::list<std::unique_ptr<glyph_fx>> glyph_fxs_;

	const gl::texture *bg_overlay_texture_;
	const gl::program *blur_program_;
	std::array<std::unique_ptr<gl::framebuffer>, 2> title_framebuffers_;
};
