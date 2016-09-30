#!/usr/bin/perl

use strict;

mkdir 'streams';

for (<DATA>) {
	my ($title, $url) = split;

	my $ogg = "streams/$title.ogg";

	if (!-f $ogg) {
		my $mp4 = "$title.mp4";

		if (!-f $mp4) {
			system('youtubedown', '--no-mux', '--title', $title, $url) == 0
				or die "youtubedown failed: $?";
		}

		system('ffmpeg', '-i', $mp4, '-vn', $ogg) == 0
			or die "ffmpeg failed: $?";
	}
}

__DATA__
aquarion	https://www.youtube.com/watch?v=ASquVYXBD7Q
shounenheart	https://www.youtube.com/watch?v=3jGlD-0sPvc
thankyou	https://www.youtube.com/watch?v=iuJ8xRYDTOA
shikinouta	https://www.youtube.com/watch?v=hhIiPySyRwg
konnanitikakude	https://www.youtube.com/watch?v=tEQTi3nY4Fs
