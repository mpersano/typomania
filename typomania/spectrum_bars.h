#ifndef SPECTRUM_BARS_H_
#define SPECTRUM_BARS_H_

#include "ogg_player.h"

namespace gl {
class texture;
}

class spectrum_bars {
public:
	spectrum_bars(const ogg_player& player, int w, int h, int num_bars);

	void update(unsigned cur_ms);
	void draw() const;

private:
	void update_spectrum_window(unsigned cur_ms);

	void render_spectrum_bars(const float *samples, int num_samples, float scale) const;

	enum {
		WINDOW_SIZE = 4096,
		LOG2_WINDOW_SIZE = 12,
	};

	float sample_window[WINDOW_SIZE];
	float spectrum_window[WINDOW_SIZE/2];

	const ogg_player& player;

	int bar_width;
	int max_height;
	int num_bands;

	const gl::texture *bar_texture;
};

#endif // SPECTRUM_BARS_H_
