#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>

#include "panic.h"
#include "kashi.h"

static std::vector<char *>
split(char *str, const char *delim)
{
	std::vector<char *> tokens;

	for (char *p = strtok(str, delim); p; p = strtok(0, delim))
		tokens.push_back(p);

	return tokens;
}

kashi::kashi()
{ }

kashi::~kashi()
{ }

bool
kashi::load(const char *path)
{
	FILE *in;

	if ((in = fopen(path, "r")) == 0)
		return false;

	char line[512];

	if (!fgets(line, sizeof(line), in)) {
		fclose(in);
		return false;
	}

	std::vector<char *> tokens = split(line, "\t\n");
	if (tokens.size() != 4) {
		fclose(in);
		return false;
	}

	name = utf8_to_wchar(tokens[0]);
	artist = utf8_to_wchar(tokens[1]);
	genre = utf8_to_wchar(tokens[2]);

	stream = tokens[3];

	while (fgets(line, sizeof(line), in)) {
		std::vector<char *> tokens = split(line, "\t\n");
		if (tokens.size() != 3) {
			fclose(in);
			return false;
		}

		serifu_list.push_back(serifu(atoi(tokens[0]), utf8_to_wchar(tokens[1]), utf8_to_wchar(tokens[2])));
	}

	fclose(in);

	init_level();

	return true;
}

void
kashi::init_level()
{
	float top_kana_per_ms = 0;

	for (serifu_cont::const_iterator i = serifu_list.begin(); i != serifu_list.end(); i++) {
		float kana_per_ms = static_cast<float>(i->duration)/i->kana.size();

		if (kana_per_ms > top_kana_per_ms)
			top_kana_per_ms = kana_per_ms;
	}

	level = static_cast<int>(top_kana_per_ms/1000);

	if (level >= 100)
		level = 99;
}

kashi::serifu::serifu(int duration, const wstring& kanji, const wstring& kana)
: duration(duration), kanji(kanji), kana(kana)
{ }
