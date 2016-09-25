#!/usr/bin/perl

use strict;
use utf8;

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

print join ';', sort { $a <=> $b } keys %glyphs;
