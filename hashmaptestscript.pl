#!/usr/bin/env perl

#
#  Copyright (C) 2011
#  University of Rochester Department of Computer Science
#    and
#  Lehigh University Department of Computer Science and Engineering
# 
# License: Modified BSD
#          Please see the file LICENSE.RSTM for licensing information

#######################################################
#
# Begin User-Specified Configuration Fields
#
#######################################################

use Cwd;

# Names of the microbenchmarks that we want to test.  Note that all
# configuration, other than thread count, goes into this string
@Benches = (#"hashmap -i100 -b1 -r100 -d1000000 -u100","hashmap -i100 -b10 -r100 -d1000000 -u100",
#"hashmap -i1000 -b1 -r1000 -d1000000 -u100","hashmap -i1000 -b10 -r1000 -d1000000 -u100","hashmap -i1000 -b100 -r1000 -d1000000 -u100",
#"hashmap -i10000 -b10 -r10000 -d1000000 -u100", "hashmap -i10000 -b100 -r10000 -d1000000 -u100", "hashmap -i10000 -b1000 -r10000 -d1000000 -u100",
#"hashmap -i100000 -b100 -r100000 -d1000000 -u100", "hashmap -i100000 -b1000 -r100000 -d1000000 -u100", "hashmap -i100000 -b10000 -r100000 -d1000000 -u100",
"hashmap -i500000 -b1000 -r500000 -d100000 -u100"
#"hashmap -i100000 -b100 -r100000 -d1000000 -u100"
    #,"yada",
    #"genome","intruder","labyrinth","ssca2"
    #kmeans, vacation
);

# Names of the STM algorithms that we want to test.  Note that you must
# consider semantics yourself... our policies don't add that support after
# the fact.  So in this case, we're using 'no semantics'
@Algs = ("p8tm-si-fullkill 0 20","p8tm-si 0 20"#,"htm-sgl"
        #,"NOrecHTBOT SPEC_TXS=2"#("NOrec", "NOrecno", "NOrecHTBOT SPEC_TXS=1", "NOrecHTBOT SPEC_TXS=2", "NOrecHTBOT SPEC_TXS=4" #("LLT", "Swiss", "NOrec");#, "NOrecHT SPEC_TXS=1", "NOrecHT SPEC_TXS=2", "NOrecHT SPEC_TXS=4", "NOrecHTBOT SPEC_TXS=1", "NOrecHTBOT SPEC_TXS=2", "NOrecHTBOT SPEC_TXS=4", NOrecHTO);
        #,"NOrecHTBOT SPEC_TXS=1 WPH=2", "NOrecHTBOT SPEC_TXS=2 WPH=2", "NOrecHTBOT SPEC_TXS=4 WPH=2"
        #,"NOrecHTBOT SPEC_TXS=1 WPH=3", "NOrecHTBOT SPEC_TXS=2 WPH=3", "NOrecHTBOT SPEC_TXS=4 WPH=3"
        #,"NOrecHTBOT SPEC_TXS=1 WPH=7", "NOrecHTBOT SPEC_TXS=2 WPH=7", "NOrecHTBOT SPEC_TXS=4 WPH=7"
);
#@Algs = ("NOrec");

# Maximum thread count
$MaxThreadCount = 40;

# Average or Max behavior.  "ProfileAppMax" is deprecated.
$ProfileBehavior = "ProfileAppAvg";

# Path to executables
$ExePath = "/home/joaobarreto/POWER8TM/benchmarks/datastructures/";

# Average of how many trials?
$Trials = 5;

# LD_PRELOAD configuration (e.g., to use libhoard on Linux)
$LDP = "";

#######################################################
#
# End User-Specified Configuration Fields
#
#######################################################

## Note: Nothing below this point should need editing

# Make sure we have exactly one parameter: a file for output
die "You should provide a single argument indicating the name of the output file\n" unless $#ARGV == 0;

# open the output file and print a header
$outfile = $ARGV[0];
open (QTABLE, ">$outfile");
print QTABLE "#BM,threads";
foreach $a (@Algs) {
    for($w = 1; $w <= 1; $w+=0.25){
	print QTABLE ", $a $w, throughput, ROT capacity aborts, ROT conflict aborts, ROT user aborts, wait time";
    }
}
print QTABLE "\n";

# Run all tests
foreach $b (@Benches) {
    # print a message to update on progress, since this can take a while...
    print "Testing ${ExePath}hashmap/${b}\n";
    
    # convert current config into a (hopefully unique) string
    $curr_b = $b;
    $curr_b =~ s/ //g;
    
    # get the single-thread characterization of the workload
    #$cbrline = `LD_PRELOAD=$LDP STM_CONFIG=$ProfileBehavior ${ExePath}${b} -p1 | tail -1`;
    #chomp($cbrline);
    #$cbrline =~ s/ #//g;

    chdir("${ExePath}");
for($r=0; $r <=0; $r++){
    # now for each thread
    for ($p = 1; $p <= $MaxThreadCount; $p+=3) {
        print "Testing at $p thread(s): ";

        $line = "$curr_b, $p";
        
	# test each algorithm
        foreach $a (@Algs) {
            for($w = 1; $w <= 1; $w+=0.25){
                print "\nTesting $a $w\n";
                `sh build-datastructures.sh $a $w`;

                # run a few trials, get the average
                $valtime = 0;
                $valrca = 0;
		$valrcon = 0;
		$valrua = 0;
                $valtt = 0;
                $valwt = 0;

                for ($t = 0; $t < $Trials; $t++) {
                    print ".";
                    @res = `./hashmap/$b -n $p`;
                    if((grep /Time/ , @res)){
                        @restime = grep  /Time =/ , @res;
                        $restime[0] =~ s/.*Time =//;
                        $valtime += $restime[0];
                        @resrca = grep  /ROT capacity aborts:/ , @res;
                        $resrca[0] =~ s/.*ROT capacity aborts://;
                        $valrca += int($resrca[0]);
			@resrcon = grep  /ROT conflict aborts:/ , @res;
                        $resrcon[0] =~ s/.*ROT conflict aborts://;
                        $valrcon += int($resrcon[0]);
			@resrua = grep  /ROT user aborts:/ , @res;
                        $resrua[0] =~ s/.*ROT user aborts://;
                        $valrua += int($resrua[0]);
                        #@restt = grep  /Total time: / , @res;
                        #$restt[0] =~ s/.*Total time: //;
                        #$valtt += int($restt[0]);
                        @reswt = grep  /Total wait time: / , @res;
                        $reswt[0] =~ s/.*Total wait time: //;
                        $valwt += int($reswt[0]);
                    } else {
                        $t--;
                    }
                }
                $valtime /= $Trials;
                $valthroughput = int(1000000/$valtime); 
                $valtime = sprintf "%.2f" , $valtime;
                $valrca /= $Trials;
                $valrca = int($valrca);
		$valrcon /= $Trials;
                $valrcon = int($valrcon);
		$valrua /= $Trials;
                $valrua = int($valrua);
#                $valtt /= $Trials;
#               $valtt = int($valtt);
                $valwt /= $Trials;
                $valwt = int($valwt);

                # add this test to the qtable: must remove all spaces
                $line .=",$valtime,$valthroughput,$valrca,$valrcon,$valrua,$valwt";

                #$valtime = 0;
                #$valgnt = 0;
                #$valgpt = 0;
                #$valgat = 0;

                #for ($t = 0; $t < $Trials; $t++) {
                #    print ".";
                #    @res = `HTM_STATS=yes PREFETCHING=yes make -f Makefile.htm_ibm runlow$p`;
                #    if((grep /Time/ , @res)){
                #        @restime = grep  /Time =/ , @res;
                #        $restime[0] =~ s/.*Time =//;
                #        $valtime += $restime[0];
                #        @resgnt = grep  /#HTM_STATS global_normal_time/ , @res;
                #        $resgnt[0] =~ s/.*#HTM_STATS global_normal_time//;
                #        $valgnt += int($resgnt[0]);
                #        @resgpt = grep  /#HTM_STATS global_prefetch_time/ , @res;
                #        $resgpt[0] =~ s/.*#HTM_STATS global_prefetch_time//;
                #        $valgpt += int($resgpt[0]);
                #        @resgat = grep  /#HTM_STATS global_abort_time/ , @res;
                #        $resgat[0] =~ s/.*#HTM_STATS global_abort_time//;
                #        $valgat += int($resgat[0]);
                #    } else {
                #        $t--;
                #    }
                #}
                #$valtime /= $Trials;
                # $valtime = sprintf "%.2f" , $valtime;
                #$valgnt /= $Trials;
                #$valgnt = int($valgnt);
                #$valgpt /= $Trials;
                #$valgpt = int($valgpt);
                #$valgat /= $Trials;
                #$valgat = int($valgat);

                # add this test to the qtable: must remove all spaces
                #$line .=",$valtime,$valgnt,$valgpt,$valgat";
            }
        }

        print "\n";

        print $line;
        $line.="\n";
        $line =~ s/ //g;
        print QTABLE "$line";

        print "\n";

        `cd ~`;
    }
}
}
close(QTABLE);
