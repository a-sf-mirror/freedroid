#!/usr/bin/perl
#
# Script inspired by pkgApp.pl from http://www.stellarium.org/
# under GPL General Public License
#

use strict;
use Cwd;

my $objdump = qq{objdump -x \%s | grep "DLL Name" | cut -d: -f2};

my $windir = shift(@ARGV);
chdir $windir;
my $main_executable = shift(@ARGV);
my $dll_dir = shift(@ARGV);

&recurse( $main_executable, $dll_dir );

sub recurse {
  my($main_executable, $dll_dir) = @_;

  my $cmd1 = sprintf($objdump, $main_executable);
  my(@names) = `$cmd1`;

  my $name;

  NAME_LOOP:
  foreach $name ( @names ) {
    chomp($name);
    $name =~ s,^\s*,,;
    my $absName = "$dll_dir/$name";

    if ( ! -e $absName ) {
      next NAME_LOOP;
    }

    my $winPath = "$name";
    
    my $not_existed = 1;
    if ( ! -e $winPath ) {
      #print STDOUT "$absName\n";
      my $c = "cp $absName $winPath";
      `$c`;
    } else {
      $not_existed = 0;
    }

    if ( $not_existed ) {
      &recurse($winPath, $dll_dir);
    }
  }
}
