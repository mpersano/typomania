#!/usr/bin/perl

use strict;
use utf8;

use constant {
	DUMPGLYPHS => './dumpglyphs'
};

binmode STDOUT, ':utf8';

my @glyphs = qw(32-126 12448-12543 12352-12447);

my %kanji;

for my $kashi (<../data/lyrics/*kashi>) {
	open IN, $kashi;
	binmode IN, ':utf8';
	
	while (<IN>) {
		$kanji{$1} = 1 while /([\x{4e00}-\x{9fbf}])/g;
	}
	
	close IN;
}

push @glyphs, (12288, (map { ord } keys %kanji));

system DUMPGLYPHS, '-W', 512, '-H', 512, '-S', 12, '-I', 'tiny', 'font.ttf', @glyphs;
system DUMPGLYPHS, '-W', 512, '-H', 1024, '-S', 18, '-I', 'small', 'font.ttf', @glyphs;
