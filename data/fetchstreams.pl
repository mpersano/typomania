#!/usr/bin/perl

use strict;

mkdir 'streams';

for (<DATA>) {
	my ($title, $url) = split;

	my $ogg = "streams/$title.ogg";

	if (!-f $ogg) {
		my $mp4 = "$title.mp4";
		my $wav = "$title.wav";

		if (!-f $mp4) {
			system('youtubedown', '--no-mux', '--title', $title, $url) == 0
				or die "youtubedown failed: $?";
		}

		system('ffmpeg', '-i', $mp4, '-vn', $wav) == 0
			or die "ffmpeg failed: $?";

		system('sox', '--norm', $wav, $ogg) == 0
			or die "sox failed: $?";

		unlink $wav;
	}
}

__DATA__
aquarion	https://www.youtube.com/watch?v=ASquVYXBD7Q
shounenheart	https://www.youtube.com/watch?v=3jGlD-0sPvc
thankyou	https://www.youtube.com/watch?v=iuJ8xRYDTOA
shikinouta	https://www.youtube.com/watch?v=hhIiPySyRwg
konnanitikakude	https://www.youtube.com/watch?v=tEQTi3nY4Fs
kimijanakya	https://www.youtube.com/watch?v=S24pkA7mFF4
youthful	https://www.youtube.com/watch?v=rxebYxY9NXE
amenotihare	https://www.youtube.com/watch?v=3oYBBf9jTqU
nandodemo	https://www.youtube.com/watch?v=4HqnOzMlqu8
lion	https://www.youtube.com/watch?v=kexAkVkwYs0
allegro	https://www.youtube.com/watch?v=gfZh80kZm3g
