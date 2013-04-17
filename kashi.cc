#include <cstdio>
#include <cstring>
#include <cerrno>

#include "panic.h"
#include "kashi.h"

kashi::kashi(const wstring& name, const wstring& artist, const wstring& genre, const std::string& stream)
: name(name), artist(artist), genre(genre), stream(stream)
{ }

kashi::~kashi()
{ }

kashi *
kashi::load(const char *path)
{
	FILE *in;
	if ((in = fopen(path, "r")) == 0) {
		fprintf(stderr, "fopen failed on `%s': %s\n", path, strerror(errno));
		return 0;
	}

	char line[512];
	if (!fgets(line, sizeof(line), in)) {
		fclose(in);
		return 0;
	}

	char name[512], artist[512], genre[512], stream[512];
	if (sscanf(line, "%[^\t]\t%[^\t]\t%[^\t]\t%[^\n]", name, artist, genre, stream) != 4) {
		fprintf(stderr, "borked 1\n");
		fclose(in);
		return 0;
	}

	kashi *p = new kashi(utf8_to_wchar(name), utf8_to_wchar(artist), utf8_to_wchar(genre), stream);

	while (fgets(line, sizeof(line), in)) {
		int duration;
		char kanji[512], kana[512];

		if (sscanf(line, "%d\t%[^\t]\t%[^\n]", &duration, kanji, kana) != 3) {
			fprintf(stderr, "borked: 2\n");
			delete p;
			fclose(in);
			return 0;
		}

		p->serifu_list.push_back(serifu(duration, utf8_to_wchar(kanji), utf8_to_wchar(kana)));
	}

	fclose(in);

	p->init_level();

	return p;
}

kashi::serifu::serifu(int duration, const wstring& kanji, const wstring& kana)
: duration(duration), kanji(kanji), kana(kana)
{ }

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
