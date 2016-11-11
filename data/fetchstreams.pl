#!/usr/bin/perl

use strict;

my $fmt = 18;

mkdir 'streams';

for (<DATA>) {
	my ($youtube_id, $seek, $title) = split;

	my $ogg = "streams/$title.ogg";

	if (!-f $ogg) {
		my $video = "$title.mp4";
		my $wav = "$title.wav";

		if (!-f $video) {
			my $url = "https://www.youtube.com/watch?v=$youtube_id";

			# download the crappiest format available to save bandwidth, we only want the audio
			system('youtubedown', '--no-mux', '--fmt', $fmt, '--title', $title, $url) == 0
				or die "youtubedown failed: $?";
		}

		system('ffmpeg', '-i', $video, '-ss', $seek, '-vn', $wav) == 0
			or die "ffmpeg failed: $?";

		system('sox', '--norm', $wav, $ogg) == 0
			or die "sox failed: $?";

		unlink $wav;
	}
}

__DATA__
ASquVYXBD7Q	0	aquarion
3jGlD-0sPvc	0	shounenheart
iuJ8xRYDTOA	0	thankyou
hhIiPySyRwg	0	shikinouta
tEQTi3nY4Fs	0	konnanitikakude
S24pkA7mFF4	0	kimijanakya
rxebYxY9NXE	0	youthful
3oYBBf9jTqU	0	amenotihare
4HqnOzMlqu8	0	nandodemo
kexAkVkwYs0	0	lion
gfZh80kZm3g	0	allegro
zneEthgiDls	3.828	sugarrush
ystEbjp-yzM	0	takeoff
ONto8dpB9Us	0	corepride
ABaGWVxeZ6g	0	northerncross
W1iDx9kQMn8	0	hametunojunjou
2MarNcvavM8	0	triangle
1l6qKUTfsWo	0	seikanhikou
WK2KHPmACug	0	castingdice
