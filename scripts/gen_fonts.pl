#!/usr/bin/perl

use strict;
use utf8;

use constant {
	DUMPGLYPHS => './dumpglyphs',
	IMAGE_PATH => 'data/images/',
	FONT => 'font.ttf',
};

binmode STDOUT, ':utf8';

my %glyphs;

$glyphs{$_} = 1 for 32 .. 126;

for my $kashi (<../data/lyrics/*kashi>) {
	open IN, $kashi;
	binmode IN, ':utf8';
	
	while (<IN>) {
		for (split //) {
			if ($_ !~ /[\t\n\r]/) {
				$glyphs{ord($_)} = 1;
			}
		}
	}
	
	close IN;
}

my @glyphs = keys %glyphs;

system DUMPGLYPHS, '-W', 512, '-H', 256, '-S', 10, '-I', 'tiny', '-p', IMAGE_PATH, FONT, @glyphs;
system DUMPGLYPHS, '-W', 512, '-H', 1024, '-S', 18, '-I', 'small', '-p', IMAGE_PATH, FONT, @glyphs;
system DUMPGLYPHS, '-W', 512, '-H', 1024, '-S', 24, '-I', 'medium', '-p', IMAGE_PATH, FONT, @glyphs;
system DUMPGLYPHS, '-W', 256, '-H', 256, '-S', 36, '-I', 'big_az', '-p', IMAGE_PATH, FONT,
  ord('A').'-'.ord('Z'), ord('0').'-'.ord('9'), ord('-');
