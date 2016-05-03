#!/usr/bin/perl 
# args format is fileName&pattern1&...

use strict;

my $file = shift or die "Missing file name\n";

my %map;
my $max = 0;

foreach my $arg (@ARGV) {
   my $count = `grep -o $arg $file | wc -l`; # -o flag only prints out the regex part of the string only, wc -l counts how many lines were printed
   $map{$arg} = $count;

   if ($count > $max) {
      $max = $count;
   }
}

if ($max == 0) { # ensure count is not 0 for when we plot the histogram
   $max = 1;
}

my $data = "";
foreach my $search (sort { $map{$b} <=> $map{$a} or $a cmp $b } keys %map) { # sorts based on values, if values are equal sorts on key
   $data = $data . "$search $map{$search}" 
}

$max = $max + 1;
chomp $max;

my $gp_cmds = <<END;
set terminal png
set output "histogram.png"
unset key
set xlabel "Search Patterns"
set ylabel "Number of Occurences"
set yrange [0:$max]
set xrange[-0.5:5.0]
set boxwidth 0.75
set tics scale 1.0
set border linewidth 1.5
set xtics 1
set style fill solid
set style data histogram
plot '-' using 2:xticlabels(1) with boxes
$data
END

open (GNUPLOT, "|gnuplot");
print GNUPLOT $gp_cmds;
