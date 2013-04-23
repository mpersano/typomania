#!/usr/bin/perl

use strict;
use utf8;

use constant {
	DUMPGLYPHS => './dumpglyphs',
	IMAGE_PATH => 'data/images/',
	FONT => 'font.ttf',
};

binmode STDOUT, ':utf8';

# my @glyphs = qw(32-126 12448-12543 12352-12447 12288 12293 65281-65374);
my @glyphs = qw(32-126 12448-12543 12352-12447 12288 12293);

my %kanji;

for my $kashi (<../data/lyrics/*kashi>) {
	open IN, $kashi;
	binmode IN, ':utf8';
	
	while (<IN>) {
		$kanji{$1} = 1 while /([\x{4e00}-\x{9fbf}])/g;
	}
	
	close IN;
}

push @glyphs, (map { ord } keys %kanji);

system DUMPGLYPHS, '-W', 512, '-H', 512, '-S', 10, '-I', 'tiny', '-p', IMAGE_PATH, FONT, @glyphs;
system DUMPGLYPHS, '-W', 512, '-H', 1024, '-S', 18, '-I', 'small', '-p', IMAGE_PATH, FONT, @glyphs;
system DUMPGLYPHS, '-W', 1024, '-H', 1024, '-S', 24, '-I', 'medium', '-p', IMAGE_PATH, FONT, @glyphs;
system DUMPGLYPHS, '-W', 256, '-H', 256, '-S', 36, '-I', 'big_az', '-p', IMAGE_PATH, FONT, ord('A').'-'.ord('Z'), ord('-');
