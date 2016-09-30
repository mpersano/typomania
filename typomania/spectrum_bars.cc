#include <cmath>
#include <cstring>

#include "common.h"
#include "render.h"
#include "fft.h"
#include "gl_texture.h"
#include "spectrum_bars.h"

spectrum_bars::spectrum_bars(const ogg_player& player, int x, int y, int w, int h, int num_bands)
: player(player)
, base_x(x), base_y(y), width(w), height(h), num_bands(num_bands)
, bar_texture(texture_cache["data/images/spectrum-bar.png"])
{ }

void
spectrum_bars::update(unsigned cur_ms)
{
	update_spectrum_window(cur_ms);
}

void
spectrum_bars::draw() const
{
	render::push_matrix();
	render::translate(base_x, base_y);
	render_spectrum_bars(spectrum_window, WINDOW_SIZE/4, 4.*height);
	render::pop_matrix();
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
	const int dx = width/num_bands;

	int x = 0;

	for (int i = 0; i < num_bands; i++) {
		float w = 0;

		for (int j = 0; j < samples_per_band; j++)
			w += samples[i*samples_per_band + j];

		w /= samples_per_band;

		w = sqrt(w)*scale;

		if (w > height)
			w = height;

		const float u = w/height;

		render::add_quad(
			bar_texture,
			{ { x, 0 }, { x, w }, { x + dx - 1, 0 }, { x + dx - 1, w } },
			{ { 0, 0 }, { 0, u }, { 1, 0 }, { 1, u } },
			-10);

		x += dx;
	}
}
