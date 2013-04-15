#!/usr/bin/perl

# converts a Typingmania XML file into a typomania kashi script.

use strict;
use XML::Simple;
use Data::Dumper;

binmode STDIN, ':utf8';
binmode STDOUT, ':utf8';

if (@ARGV == 0) {
	print STDERR "Usage: $0 file.xml\n";
	exit 1;
}

my $parser = XML::Simple->new;

my $muzak = $parser->XMLin($ARGV[0]);

my $interval = $muzak->{interval};
my $nihongoword = $muzak->{nihongoword};
my $word = $muzak->{word};

my $num_lines = scalar @{$muzak->{nihongoword}};

for my $i (0 .. @{$interval} - 1) {
	print "$interval->[$i]\t$nihongoword->[$i]\t$word->[$i]\n";
}
