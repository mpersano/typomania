#include <cmath>
#include <cstring>

#include "common.h"
#include "resources.h"
#include "render.h"
#include "fft.h"
#include "gl_texture.h"
#include "spectrum_bars.h"

spectrum_bars::spectrum_bars(const ogg_player& player, int w, int h, int num_bands)
: player(player)
, bar_width(w), max_height(h), num_bands(num_bands)
, bar_texture(get_texture("data/images/spectrum-bar.png"))
{ }

void
spectrum_bars::update(unsigned cur_ms)
{
	update_spectrum_window(cur_ms);
}

void
spectrum_bars::draw() const
{
	render_spectrum_bars(spectrum_window, WINDOW_SIZE/4, 4.*max_height);
}

void
spectrum_bars::update_spectrum_window(unsigned cur_ms)
{
	const int buffer_samples = player.get_num_buffer_samples();
	const int total_buffer_samples = buffer_samples*ogg_player::NUM_BUFFERS;

	// ogg_player::rate samples --> 1 second
	// y samples --> cur_ms msecs

	// 1 tic --> ogg_player::rate/30 samples

	unsigned sample_index = (static_cast<unsigned long long>(cur_ms)*player.rate/1000)%total_buffer_samples;

	for (int i = 0; i < WINDOW_SIZE; i++) {
		const ogg_player::buffer& buf = player.buffers[sample_index/buffer_samples];
		const int16_t *buffer_data = reinterpret_cast<const int16_t *>(buf.data);

		int j = sample_index%buffer_samples;

		switch (player.format) {
			case AL_FORMAT_MONO16:
				sample_window[i] = static_cast<float>(buffer_data[j])/(1 << 15);
				break;

			case AL_FORMAT_STEREO16:
				{
				int16_t l = buffer_data[j*2];
				int16_t r = buffer_data[j*2 + 1];
				sample_window[i] = (.5*(static_cast<float>(l) + static_cast<float>(r)))/(1 << 15);
				}
				break;
		}

		if (++sample_index == total_buffer_samples)
			sample_index = 0;
	}

	static float real[WINDOW_SIZE], imag[WINDOW_SIZE];
	memcpy(real, sample_window, sizeof real);
	memset(imag, 0, sizeof imag);

	fft(1, LOG2_WINDOW_SIZE, real, imag);

	for (int i = 0; i < WINDOW_SIZE/2; i++)
		spectrum_window[i] = sqrtf(real[i]*real[i] + imag[i]*imag[i]);
}

void
spectrum_bars::render_spectrum_bars(const float *samples, int num_samples, float scale) const
{
	const int samples_per_band = num_samples/num_bands;

	int x = 0;

	for (int i = 0; i < num_bands; i++) {
		int s0 = i*samples_per_band;
		int s1 = std::min(s0 + samples_per_band, num_samples);

		float h = 0;

		for (int j = s0; j < s1; j++)
			h += samples[j];

		h /= samples_per_band;

		h = sqrt(h)*scale;

		if (h > max_height)
			h = max_height;

		const float y0 = -.5f*bar_width;
		const float y1 = 0.f;
		const float y2 = h;
		const float y3 = h + .5f*bar_width;

		render::draw_quad(
			bar_texture,
			{ { x, y0 }, { x, y1 }, { x + bar_width, y0 }, { x + bar_width, y1 } },
			{ { 0, 0 }, { 0, .5f }, { 1, 0 }, { 1, .5f } },
			-10);

		render::draw_quad(
			bar_texture,
			{ { x, y1 }, { x, y2 }, { x + bar_width, y1 }, { x + bar_width, y2 } },
			{ { 0, .5f }, { 0, .5f }, { 1, .5f }, { 1, .5f } },
			-10);

		render::draw_quad(
			bar_texture,
			{ { x, y2 }, { x, y3 }, { x + bar_width, y2 }, { x + bar_width, y3 } },
			{ { 0, .5f }, { 0, 1 }, { 1, .5f }, { 1, 1 } },
			-10);

		x += bar_width;
	}
}
