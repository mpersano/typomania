#ifndef SPECTRUM_BARS_H_
#define SPECTRUM_BARS_H_

#include "ogg_player.h"

struct spectrum_bars {
public:
	spectrum_bars(const ogg_player& player);

	void update(int cur_tic);
	void draw() const;

private:
	void update_spectrum_window(int cur_tic);

	void render_wave(const float *samples, int num_samples, float scale) const;
	void render_spectrum_bars(const float *samples, int num_samples, float scale) const;

	enum {
		WINDOW_SIZE = 4096,
		LOG2_WINDOW_SIZE = 12,
	};

	float sample_window[WINDOW_SIZE];
	float spectrum_window[WINDOW_SIZE/2];

	const ogg_player& player;
};

#endif // SPECTRUM_BARS_H_
