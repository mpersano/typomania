#include <cmath>
#include <cstring>

#include <GL/gl.h>

#include "fft.h"
#include "common.h"
#include "spectrum_bars.h"

spectrum_bars::spectrum_bars(const ogg_player& player)
: player(player)
{ }

void
spectrum_bars::update(int cur_tic)
{
	update_spectrum_window(cur_tic);
}

void
spectrum_bars::draw() const
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);

	glPushMatrix();
	glTranslatef(0, .25*WINDOW_HEIGHT, 0);
	glColor4f(.5, .5, .5, 1);
	render_wave(sample_window, WINDOW_SIZE, .25*WINDOW_HEIGHT);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, .5*WINDOW_HEIGHT, 0);
	glColor4f(1, 1, 1, 1);
	render_spectrum_bars(spectrum_window, WINDOW_SIZE/2, WINDOW_HEIGHT);
	glPopMatrix();
}

void
spectrum_bars::update_spectrum_window(int cur_tic)
{
	const int buffer_samples = player.get_num_buffer_samples();
	const int total_buffer_samples = buffer_samples*ogg_player::NUM_BUFFERS;

	// 30 tics/second
	// ogg_player::rate samples/second

	// 1 tic --> ogg_player::rate/30 samples

	int sample_index = (cur_tic*player.rate/TICS_PER_SECOND)%total_buffer_samples;

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
spectrum_bars::render_wave(const float *samples, int num_samples, float scale) const
{
	const float dx = static_cast<float>(WINDOW_WIDTH)/num_samples;

	float x = 0;

	glBegin(GL_LINES);

	for (int i = 0; i < num_samples - 1; i++) {
		const float s0 = samples[i];
		const float s1 = samples[i + 1];

		glVertex2f(x, s0*scale);
		glVertex2f(x + dx, s1*scale);

		x += dx;
	}

	glEnd();
}

void
spectrum_bars::render_spectrum_bars(const float *samples, int num_samples, float scale) const
{
	const int NUM_BANDS = 64;
	const int SAMPLES_PER_BAND = num_samples/NUM_BANDS;

	float x = 0;
	const float dx = static_cast<float>(WINDOW_WIDTH)/NUM_BANDS;

	glBegin(GL_QUADS);

	for (int i = 0; i < NUM_BANDS; i++) {
		float w = 0;

		for (int j = 0; j < SAMPLES_PER_BAND; j++)
			w += samples[i*SAMPLES_PER_BAND + j];

		w *= scale*log(i + 2);

		w++;

		glVertex2f(x, 0);
		glVertex2f(x, w);
		glVertex2f(x + dx - 1, w);
		glVertex2f(x + dx - 1, 0);

		x += dx;
	}

	glEnd();
}
