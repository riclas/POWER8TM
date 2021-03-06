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
@Benches = ("tpcc -t 5 -w 1 -s 87 -d 1 -o 1 -p 1 -r 10","tpcc -t 5 -w 1 -s 47 -d 1 -o 1 -p 1 -r 50","tpcc -t 5 -w 1 -s 7 -d 1 -o 1 -p 1 -r 90"
    #,"yada",
    #"genome","intruder","labyrinth","ssca2"
    #kmeans, vacation
);

# Names of the STM algorithms that we want to test.  Note that you must
# consider semantics yourself... our policies don't add that support after
# the fact.  So in this case, we're using 'no semantics'
@Algs = ("p8tm","p8tm-si"#,"htm-sgl"
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
$ExePath = "/home/joaobarreto/POWER8TM/benchmarks/tpcc/";

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
	print QTABLE ", $a wait $w, ROT capacity aborts, ROT RW aborts, total time, wait time";
    }
}
print QTABLE "\n";

# Run all tests
foreach $b (@Benches) {
    # print a message to update on progress, since this can take a while...
    print "Testing ${ExePath}${b}\n";
    
    # convert current config into a (hopefully unique) string
    $curr_b = $b;
    $curr_b =~ s/ //g;
    
    # get the single-thread characterization of the workload
    #$cbrline = `LD_PRELOAD=$LDP STM_CONFIG=$ProfileBehavior ${ExePath}${b} -p1 | tail -1`;
    #chomp($cbrline);
    #$cbrline =~ s/ #//g;

    chdir("${ExePath}");

    # now for each thread
    for ($p = 1; $p <= $MaxThreadCount; $p+=1) {
        print "Testing at $p thread(s): ";

        $line = "$curr_b,$p";
        
	# test each algorithm
        foreach $a (@Algs) {
            for($w = 1; $w <= 1; $w+=0.25){
                print "\nTesting $a with wait ratio $w\n";
                `sh build-tpcc.sh $a 0 2 $w`;

                # run a few trials, get the average
                $valthroughput = 0;
                $valrca = 0;
		$valrwa = 0;
                $valtt = 0;
                $valwt = 0;

                for ($t = 0; $t < $Trials; $t++) {
                    print ".";
                    @res = `./code/$b -n $p`;
                    if((grep /ms =/ , @res)){
                        @resthroughput = grep  /ms =/ , @res;
                        $resthroughput[0] =~ s/.*ms =//;
                        $valthroughput += $resthroughput[0];
                        @resrca = grep  /ROT capacity aborts:/ , @res;
                        $resrca[0] =~ s/.*ROT capacity aborts://;
                        $valrca += int($resrca[0]);
                        @resrwa = grep  /ROT non-trans aborts:/ , @res;
			$resrwa[0] =~ s/.*ROT non-trans aborts://;
			$valrwa += int($resrwa[0]);
			@restt = grep  /Total time: / , @res;
                        $restt[0] =~ s/.*Total time: //;
                        $valtt += int($restt[0]);
                        @reswt = grep  /Total wait time: / , @res;
                        $reswt[0] =~ s/.*Total wait time: //;
                        $valwt += int($reswt[0]);
                    } else {
                        $t--;
                    }
                }
                $valthroughput /= $Trials;
                $valthroughput = int($valthroughput);
                $valrca /= $Trials;
                $valrca = int($valrca);
		$valrwa /= $Trials;
		$valrwa = int($valrwa);
                $valtt /= $Trials;
                $valtt = int($valtt);
                $valwt /= $Trials;
                $valwt = int($valwt);

                # add this test to the qtable: must remove all spaces
                $line .=",$valthroughput,$valrca,$valrwa,$valtt,$valwt";

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
        print $line;
        $line.="\n";
        $line =~ s/ //g;
        print QTABLE "$line";

        print "\n";

        `cd ~`;
    }
}

close(QTABLE);
