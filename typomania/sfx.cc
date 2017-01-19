#include <vector>
#include <string>
#include <memory>

#include <boost/noncopyable.hpp>

#include <SDL.h>

#include <AL/alc.h>
#include <AL/al.h>

#include "panic.h"
#include "sfx.h"

namespace {

class player : public boost::noncopyable
{
public:
	player();

	int add_effect(const std::string& source, int max_sources);

	void play(int effect_id);

private:
	class effect
	{
	public:
		effect(const std::string& source, int max_sources);

		void play();

	private:
		ALuint buffer_;
		int cur_source_;
		std::vector<ALuint> sources_;
	};

	std::vector<std::unique_ptr<effect>> effects_;
} *g_player;

player::effect::effect(const std::string& source, int max_sources)
{
	SDL_AudioSpec spec;
	uint32_t len;
	uint8_t *buf;

	if (!SDL_LoadWAV(source.c_str(), &spec, &buf, &len))
		panic("failed to load %s", source.c_str());

	ALenum format;

	switch (spec.format) {
		case AUDIO_U8:
		case AUDIO_S8:
			format = spec.channels == 2 ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
			break;

		case AUDIO_U16:
		case AUDIO_S16:
			format = spec.channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
			break;

		default:
			panic("unrecognized wav format in %s", source.c_str());
	}

	// load buffer

	alGenBuffers(1, &buffer_);
	if (alGetError() != AL_NO_ERROR)
		panic("alGenBuffers failed");

	alBufferData(buffer_, format, buf, len, spec.freq);
	if (alGetError() != AL_NO_ERROR)
		panic("alBufferData failed");

	// create sources

	sources_.resize(max_sources);

	alGenSources(max_sources, &sources_[0]);
	if (alGetError() != AL_NO_ERROR)
		panic("alGenSources failed");

	// attach buffer to sources

	for (auto source : sources_) {
		alSourcei(source, AL_BUFFER, buffer_);
		if (alGetError() != AL_NO_ERROR)
			panic("alSourcei failed");
	}

	cur_source_ = 0;
}

void player::effect::play()
{
	auto s = sources_[cur_source_++];
	if (cur_source_ == sources_.size())
		cur_source_ = 0;

	alSourceStop(s);
	alSourceRewind(s);
	alSourcePlay(s);
}

player::player()
{
}

int player::add_effect(const std::string& source, int max_sources)
{
	effects_.emplace_back(new effect(source, max_sources));
	return effects_.size() - 1;
}

void player::play(int effect_id)
{
	if (effect_id >= 0 && effect_id < effects_.size())
		effects_[effect_id]->play();
}

}

namespace sfx
{

void init()
{
	g_player = new player;
}

int add_effect(const std::string& source, int max_sources)
{
	return g_player->add_effect(source, max_sources);
}

void play(int effect_id)
{
	g_player->play(effect_id);
}

};
