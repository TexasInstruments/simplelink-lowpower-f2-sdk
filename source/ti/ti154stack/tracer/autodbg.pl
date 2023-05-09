#! /usr/bin/perl

#------------------------------------------------------------------------------
# TI Confidential - NDA Restrictions
#
# Copyright (c) 2011 Texas Instruments, Inc.
#
#    This is an unpublished work created in the year stated above.
#    Texas Instruments owns all rights in and to this work and
#    intends to maintain and protect it as an unpublished copyright.
#    In the event of either inadvertent or deliberate publication,
#    the above stated date shall be treated as the year of first
#    publication. In the event of such publication, Texas Instruments
#    intends to enforce its rights in the work under the copyright
#    laws as a published work.
#
#
#  Content        : Script to extract debug print information from C files
#  Created By     : Low Power RF Wireless Business Unit
#                   Helge Coward (h.coward@ti.com)
#
#------------------------------------------------------------------------------
#  Filename       : autodbg.pl
#  Date           : Thu Nov 17 10:26:10 CET 2011
#------------------------------------------------------------------------------

use strict;
use warnings;
use IO::File;
use File::Basename;  # fileparse()


my %newDbgSyms;
my %oldDbgSyms;

my @chanList;
my %channels;
my $supportAssert;

sub findAliasCh(@);
sub findAliasCh(@) {
    my ($chName, $fileName) = @_;
    my $newChName;

    if ($chName eq "DBGCPE") {
        $newChName = "DBGCH1";
    }
    elsif ($chName eq "DBGTOPSM") {
        $newChName = "DBGCH2";
    }
    elsif ($chName eq "DBGSYS") {
        $newChName = "DBGCH3";
    }
    else {
        $newChName = $chName;
    }

    unless ($channels{$newChName}) {
        die "Channel $chName found in $fileName is not supported";
    }

    return $newChName;
}


# PCP: Finds all DBG_PRINTx statements in a given C-file and creates the corresponding preprocessor
# symbol that will be written out to a dbgid.h file in the project. Returns the preprocessor symbol
# that needs to be passed to armcc for each file
sub FindCDbgPrint($);
sub FindCDbgPrint($) {
    my $fileSymbol;
    my ($fileName) = @_;

    if ($fileName =~ /\/?([^\/]+)$/i) {
        $fileSymbol = $1;
        $fileSymbol =~ s/[.-]/_/;
    } else {
        return;
    }
    #print STDERR ">> fileName=$fileName, fileSymbol=$fileSymbol\n";

    if (open(CFILE, "$fileName")) {
        my $commstate = "UNCOMMENTED";
        my $dbgstate = "NONE";
        my $lineno;
        my $nargs;
        my $ch;
        my $dbgstr;

        while (<CFILE>) {
            #print STDERR "--- L$.: $_";
            if ($commstate eq "COMMENTED") {
                if (/^.*?\*\/(.*)$/) {
                    # C close comment
                    #print STDERR "--- L$. C close comment ML\n";
                    $commstate="UNCOMMENTED";
                    $_ = $1."\n";
                    redo;
                }
            } else {
                if (/^(.*?)\/\/.*$/) {
                    # Remove text after C++ comment
                    $_ = $1."\n";
                    #print STDERR "--- L$. C++ comment Line: $_";
                }
                if (/^(.*?)\/\*(.*)$/) {
                    # C open comment
                    my $line = $1;
                    my $comment = $2."\n";
                    #print STDERR "--- L$. C open comment ROL: $comment";
                    if ($comment =~ /^(.*?)\*\/(.*)$/) {
                        # Comment finishes on same line
                        #print STDERR "--- L$. C close comment SL\n";
                        $_ = $line.$2."\n";
                        redo;
                    } else {
                        # Multiline comment
                        #print STDERR "--- L$. C comment goes past EOL\n";
                        $commstate="COMMENTED";
                        $_ = $line."\n";
                    }
                }
                if ($dbgstate eq "NONE") {
                    # Looking for DBG_PRINTx definition
                    if (/^.*DBG_PRINT(L?)([0-4])\(\s*(.*)$/) {
                        # Found debug print command
                        $lineno = $.;
                        $dbgstr = "";
                        if ($1) {
                            $nargs = -$2;
                        }
                        else{
                            $nargs = $2;
                        }
                        $dbgstate = "CH";
                        $_ = $3."\n";
                        redo;
                    } elsif ( /DBG_ASSERT\(\s*(\w+),\s*(.*)\)/ ) {
                        # Found debug assert command
                        #print STDERR "\n>>> DBG_ASSERT found on L$. in $fileName => \"$1\"";
                        $lineno = $.;
                        $dbgstr = "[ASSERTION FAILED] $2";
                        $nargs = 0;
                        $ch = findAliasCh($1, $fileName);


                        $newDbgSyms{"DBGID_${fileSymbol}_${lineno}"}{SYM} = "DBGID_${fileSymbol}_${lineno}";
                        $newDbgSyms{"DBGID_${fileSymbol}_${lineno}"}{FILENAME} = $fileName;
                        $newDbgSyms{"DBGID_${fileSymbol}_${lineno}"}{ID} = 0;
                        $newDbgSyms{"DBGID_${fileSymbol}_${lineno}"}{LINENO} = $lineno;
                        $newDbgSyms{"DBGID_${fileSymbol}_${lineno}"}{CH} = $ch;
                        $newDbgSyms{"DBGID_${fileSymbol}_${lineno}"}{NARGS} = $nargs;
                        $newDbgSyms{"DBGID_${fileSymbol}_${lineno}"}{DBGSTR} = $dbgstr;
                        $dbgstate = "NONE";
                        $_ = "\n";
                    } elsif ( /ASSERT\((.*)\)/ ) {
                        if ($supportAssert) {
                            # Found assert command
                            #print STDERR "\n>>> ASSERT found on L$. in $fileName => \"$1\"";
                            $lineno = $.;
                            $dbgstr = "[ASSERTION FAILED] $1";
                            $nargs = 0;
                            $ch = $chanList[0];
                            $newDbgSyms{"DBGID_${fileSymbol}_${lineno}"}{SYM} = "DBGID_${fileSymbol}_${lineno}";
                            $newDbgSyms{"DBGID_${fileSymbol}_${lineno}"}{FILENAME} = $fileName;
                            $newDbgSyms{"DBGID_${fileSymbol}_${lineno}"}{ID} = 0;
                            $newDbgSyms{"DBGID_${fileSymbol}_${lineno}"}{LINENO} = $lineno;
                            $newDbgSyms{"DBGID_${fileSymbol}_${lineno}"}{CH} = $ch;
                            $newDbgSyms{"DBGID_${fileSymbol}_${lineno}"}{NARGS} = $nargs;
                            $newDbgSyms{"DBGID_${fileSymbol}_${lineno}"}{DBGSTR} = $dbgstr;
                            $dbgstate = "NONE";
                            $_ = "\n";
                        }
                    }
                } elsif ($dbgstate eq "CH") {
                    if (/^\s*(\w+)\s*,(.*)$/) {
                        # Found channel identifier
                        $ch = findAliasCh($1, $fileName);
                        $dbgstate = "STR";
                        $_ = $2."\n";
                        redo;
                    }
                } elsif ($dbgstate eq "STR") {
                    if (/^\s*\"((.*?\\\")*.*?)\"(.*)$/) {
                        # Found string
                        $dbgstr .= $1;
                        $_ = $3."\n";
                        redo;
                    } elsif (/^\s*?(,|\)\s*;)(.*)$/) {
                        # Found terminating parenthesis or comma
                        #print STDERR ">>> [ch $ch, DBGID_${fileSymbol}_${lineno}, nargs $nargs, \"$dbgstr\"\n";
                        $newDbgSyms{"DBGID_${fileSymbol}_${lineno}"}{SYM} = "DBGID_${fileSymbol}_${lineno}";
                        $newDbgSyms{"DBGID_${fileSymbol}_${lineno}"}{FILENAME} = $fileName;
                        $newDbgSyms{"DBGID_${fileSymbol}_${lineno}"}{ID} = 0;
                        $newDbgSyms{"DBGID_${fileSymbol}_${lineno}"}{LINENO} = $lineno;
                        $newDbgSyms{"DBGID_${fileSymbol}_${lineno}"}{CH} = $ch;
                        $newDbgSyms{"DBGID_${fileSymbol}_${lineno}"}{NARGS} = $nargs;
                        $newDbgSyms{"DBGID_${fileSymbol}_${lineno}"}{DBGSTR} = $dbgstr;
                        $dbgstate = "NONE";
                        $_ = $2."\n";
                        redo;
                    }
                }
            }
        }
        close CFILE;
    }
    return $fileSymbol;
}




sub ParseOldDbgIdFile($);
sub ParseOldDbgIdFile($) {
    my ($origFilePath) = @_;  # note: the input filename may include pathname
    # we want to append "_" as prefix in filename
    my ($origFileName, $directories) = fileparse($origFilePath);
    my ($mangledFileName) = "_" . $origFileName;
    my ($mangledFilePath) = $directories . $mangledFileName;

    if (open DF, "<$mangledFilePath") {
        LINE : while (my $line = <DF>) {
            # Remove trailing newline characters
            chomp $line;
            if ($line =~ /^DBG_DEF\(([A-Za-z_0-9]+), ([0-9]+), ([A-Z0-9]+), (-?[0-9]+), \"([^\"]+)\", \"([^\"]+)\", ([0-9]+)\)/) {
                my $duplicate = "";
                # Check for duplicate IDs
                foreach my $key (keys %oldDbgSyms) {
                    if ($oldDbgSyms{$key}{ID} == $2 && $oldDbgSyms{$key}{CH} eq $3) {
                        # ID is duplicate
                        if ($oldDbgSyms{$key}{FILENAME} eq $6) {
                            # If it came from the same source file, just delete the old occurrence
                            delete $oldDbgSyms{$key};
                        }
                        else {
                            # Otherwise, mark as duplicate. This will be handled if both symbols are used.
                            $duplicate = $key;
                        }
                    }
                }
                $oldDbgSyms{"$1"}{SYM}      = $1;
                $oldDbgSyms{"$1"}{ID}       = $2;
                $oldDbgSyms{"$1"}{CH}       = $3;
                $oldDbgSyms{"$1"}{NARGS}    = $4;
                $oldDbgSyms{"$1"}{DBGSTR}   = $5;
                $oldDbgSyms{"$1"}{FILENAME} = $6;
                $oldDbgSyms{"$1"}{LINENO}   = $7;
                $oldDbgSyms{"$1"}{DUPLICATE}= $duplicate;
            }
        }
        close DF;
        # Check if wrapper file existed
        if (open DF, "<$origFilePath") {
            close DF;
            return 1;
        }
        else {
            return 0;
        }
    }
    else {
        # File did not exist
        return 0;
    }

}




sub PrintNewDbgIdFile($);
sub PrintNewDbgIdFile($) {
    my ($origFilePath) = @_;  # note: the input filename may include pathname
    # we want to append "_" as prefix in filename
    my ($origFileName, $directories) = fileparse($origFilePath);
    my ($mangledFileName) = "_" . $origFileName;
    my ($mangledFilePath) = $directories . $mangledFileName;


    open DF, ">$mangledFilePath" || die "Could not open file $mangledFilePath for writing\n";
    print DF "//*****************************************************************************\n".
        "// This file is AUTOGENERATED! Do not check in or edit.\n".
        "// Generated on ".localtime()."\n".
        "//*****************************************************************************\n".
        "#ifndef __DBGID_H\n".
        "#define __DBGID_H\n".
        "#ifndef _DBGID_H\n".
        "#warning \"$mangledFileName should not be directly included in a C file. Include $origFileName instead.\"\n".
        "#endif\n\n".
        "#include \"dbg.h\"\n\n";

    foreach my $sym (sort(keys(%newDbgSyms))) {
        print DF "DBG_DEF($sym, $newDbgSyms{$sym}{ID}, $newDbgSyms{$sym}{CH}, $newDbgSyms{$sym}{NARGS}, ".
            "\"$newDbgSyms{$sym}{DBGSTR}\", \"$newDbgSyms{$sym}{FILENAME}\", $newDbgSyms{$sym}{LINENO})\n";
    }

    print DF "\n".
        "#endif //__DBGID_H\n";
    close DF;
}

sub PrintWrapperDbgIdFile($);
sub PrintWrapperDbgIdFile($) {
    my ($origFilePath) = @_;  # note: the input filename may include pathname
    # we want to append "_" as prefix in filename
    my ($origFileName, $directories) = fileparse($origFilePath);
    my ($mangledFileName) = "_" . $origFileName;
    my ($mangledFilePath) = $directories . $mangledFileName;


    open DF, ">$origFilePath" || die "Could not open file $origFilePath for writing\n";
    print DF "//*****************************************************************************\n".
        "// This file is AUTOGENERATED! Do not check in or edit.\n".
        "// Generated on ".localtime()."\n".
        "//*****************************************************************************\n".
        "#ifndef _DBGID_H\n".
        "#define _DBGID_H\n\n";
    unless ($supportAssert) {
        print DF "#define _NO_ASSERT\n";
    }
    print DF "#include \"dbg.h\"\n".
        "#ifndef _MAKEDEPEND\n".
        "#include \"$mangledFileName\"\n".
        "#endif\n\n".
        "#endif //_DBGID_H\n";

    close DF;
}


sub ParseProjectFile;
sub ParseProjectFile {
   my ($projectDir)  = $_[0];
   my ($projectFile) = $_[1];
   my @cFiles;
   my $absolutePath;

   my $mac_core_path;
   my $lprf_com_path;

   print "$projectDir x $projectFile\n";

   $mac_core_path = "$projectDir../../../../lprf-timac-2";
   $lprf_com_path = "$projectDir../../../../lprf-sd-common-components";


   if (open DF, "<$projectDir/$projectFile") {
   LINE : while (my $line = <DF>) {
      chomp $line;
      if ($line =~ /\s*locationURI\>MAC_CORE(.*)\.c\<\/locationURI\>/) {
          $absolutePath = $1;
          #print "$absolutePath\n";
          #print "$mac_core_path\n";
          $absolutePath = "$mac_core_path$absolutePath";
          #print "$absolutePath\n";
          push(@cFiles, "$absolutePath.c");
      }elsif($line =~ /\s*locationURI\>COM_COMP(.*)\.c\<\/locationURI\>/) {
          $absolutePath = $1;
          #print "$absolutePath\n";
          #print "$lprf_com_path\n";
          $absolutePath = "$lprf_com_path$absolutePath";
          #print "$absolutePath\n";
          push(@cFiles, "$absolutePath.c");
      }

   }
   close DF;
   }
   return @cFiles;
}


######################################
# Entry point
######################################

my $ewpFile = shift @ARGV;
$ewpFile =~ s/\\/\//g;
my ($projectDir, $projectFile) = $ewpFile =~ m/(.*\/)(.*)$/;

print "$ewpFile $projectDir $projectFile\n";

my $outFileName = shift @ARGV;
$outFileName =~ s/\\/\//g;

print "$outFileName \n";

my $createTouchFiles = 1;
$supportAssert = 1;
while ($ARGV[0] =~ /^-/) {
    if ($ARGV[0] =~ /^-c([1-3])/) {
        push @chanList, "DBGCH$1";
        shift @ARGV;
        print "DBGCH$1\n";
    }
    elsif ($ARGV[0] eq "-nt") {
        $createTouchFiles = 0;
        shift @ARGV;
        print "createTouchFiles = 0 \n";
    }
    elsif ($ARGV[0] eq "-na") {
        $supportAssert = 0;
        shift @ARGV;
        print "supportAssert = 0 \n";
    }
    else {
        die "Unknown option $ARGV[0]";
    }
}
# my @inFileNames = @ARGV;

if (@chanList == 0) {
    # Default to all channels with channel 1 as the main one
    @chanList = ("DBGCH1", "DBGCH2", "DBGCH3");
}

for (@chanList) {
    $channels{$_}++;
}

my @inFileNames = ParseProjectFile($projectDir, $projectFile);

# Read the old debug ID header file
my $filesExisted = ParseOldDbgIdFile("${outFileName}");

# Read each input file
foreach my $inFileName (@inFileNames) {
    $inFileName =~ s/\\/\//g;

    print "Opening C file $inFileName\n";
    FindCDbgPrint($inFileName) or die "Unable to find \"$inFileName\"\n";
}

# Prepare the tables of used debug ID numbers
my %dbgIdTable;
for (my $n = 0; $n < 512; $n++) {
    $dbgIdTable{DBGCH1}[$n] = ($n < 3) ? 1 : 0;
    $dbgIdTable{DBGCH2}[$n] = ($n < 3) ? 1 : 0;
    $dbgIdTable{DBGCH3}[$n] = ($n < 3) ? 1 : 0;
}

# Copy all debug IDs found in both the old and the new debug symbol table. All unmatching debug
# IDs will be assigned randomly later on.
foreach my $key (keys %oldDbgSyms) {
    if ($newDbgSyms{$key}) {
        if (($newDbgSyms{$key}{SYM} eq $oldDbgSyms{$key}{SYM}     ) &&
            ($newDbgSyms{$key}{CH} eq $oldDbgSyms{$key}{CH}      ) &&
            ($newDbgSyms{$key}{NARGS} eq $oldDbgSyms{$key}{NARGS}   ) &&
            ($newDbgSyms{$key}{DBGSTR} eq $oldDbgSyms{$key}{DBGSTR}  ) &&
            ($newDbgSyms{$key}{FILENAME} eq $oldDbgSyms{$key}{FILENAME}) &&
            ($newDbgSyms{$key}{LINENO} eq $oldDbgSyms{$key}{LINENO}  )) {
            $newDbgSyms{$key}{ID} = $oldDbgSyms{$key}{ID};
            $dbgIdTable{$newDbgSyms{$key}{CH}}[$newDbgSyms{$key}{ID}] = 1;
        }
    }
}

# Check if any of the assigned debug IDs has been assigned to several symbols. If so, keep the
# last one only and invalidate the source files
foreach my $key (keys %oldDbgSyms) {
    my $duplicate = $oldDbgSyms{$key}{DUPLICATE};
    if (exists($newDbgSyms{$duplicate}) && exists($newDbgSyms{$key})) {
        delete $newDbgSyms{$duplicate}{ID};
        $filesExisted = 0;
    }
}

# If a new debug symbol new debug symbol does not have an ID, then assign one and touch the source
# file's corresponding .dbgid_updated file
my $keysChanged = 0;
foreach my $key (keys %newDbgSyms) {
    unless ($newDbgSyms{$key}{ID}) {
#        if ($createTouchFiles) {
#            # Generate the touch file name, and touch the file!
#            my $touchFileName = $newDbgSyms{$key}{FILENAME};
#            $touchFileName =~ s/[^\/]*\///g;
#            $touchFileName =~ s/\.c$//g;
#            $touchFileName .= ".dbgid_updated";
#            my $TF = IO::File->new(">$touchFileName");
#            close $TF;
#        }
        $keysChanged = 1;
        my $idFound = 0;
        FIND_ID : for (my $n = 0; $n < 512; $n++) {
            unless ($dbgIdTable{$newDbgSyms{$key}{CH}}[$n]) {
                $newDbgSyms{$key}{ID} = $n;
                $dbgIdTable{$newDbgSyms{$key}{CH}}[$n] = 1;
                $idFound = 1;
                last FIND_ID;
            }
        }
        unless ($idFound) {
            die "Too many debug messages on channel $newDbgSyms{$key}{CH}";
        }
    }
}

# Print the new debug ID header file
PrintNewDbgIdFile($outFileName);
#unless ($filesExisted) {
    # Re-create wrapper file
    PrintWrapperDbgIdFile($outFileName);
#}

exit(0);
