#!/bin/bash
# Allows batch runs of simulations. Saves results to log files

# run if user hits control-c
function control_c() {
    echo -en "*** Ouch! Exiting ***\n"
    exit $?
}

# Install WARPED-2 and build WARPED-2 models
function build {
    rootPath=$1
    gitBranch=$2
    mpiIncludePath=$3
    mpiLibraryPath=$
    additionalFlags=$5

    echo -e "\nInstalling WARPED-2:$gitBranch with additional flags $additionalFlags"

    cd $rootPath/warped2/
    git checkout $gitBranch
    autoreconf -i
    ./configure --with-mpi-includedir=$mpiIncludePath \
        --with-mpi-libdir=$mpiLibraryPath --prefix=$rootPath/installation/ $additionalFlags
    make
    make install

    cd $rootPath/warped2-models/
    autoreconf -i
    ./configure --with-warped=$rootPath/installation/
    make

    cd $rootPath/warped2-models/scripts/
}

# Create the Louvain partition profile for bags
function bagProfile {
    model=$1
    modelCmd=$2
    maxSimTime=$3
    fileName=$4

    cd ../models/$model/
    echo -e "\nProfile $modelCmd , File: $fileName , Max Sim Time: $maxSimTime"
    $modelCmd --max-sim-time $maxSimTime --simulation-type sequential \
            --sequential-statistics-type louvain --sequential-statistics-file $fileName
    $runCommand

    cd ../../scripts/
}

function  {
    testCycles=$1
    timeoutPeriod=$2
    model=$3
    modelCmd=$4
    maxSimTime=$5
    partitioningFile=$6
    workerThreads=$7
    fractionWindow=$8
    staticWindowSize=$9

    for ((iteration=1; iteration <= $testCycles; ++iteration));
    do
        cd ../models/$model/
        outMsg="\n($iteration/$testCycles) $modelCmd : $workerThreads threads, \
                $scheduleQCount schedulers, max sim time: $maxSimTime"
        echo -e $outMsg
        tmpFile=`tempfile`
        runCommand="$modelCmd --max-sim-time $maxSimTime --simulation-type time-warp \
            --time-warp-worker-threads $workerThreads --time-warp-scheduler-count $scheduleQCount \
            --partitioning-type profile-guided --time-warp-gvt-calculation-period 5000 \
            --time-warp-statistics-file $tmpFile"
        timeout $timeoutPeriod bash -c "$runCommand" | grep -e "Simulation completed in " -e "Type of Schedule queue: "

        # Parse stats
        statsRaw=`cat $tmpFile | grep "Total,"`
        statsRefined=`echo $statsRaw | sed -e 's/Total,//g' -e 's/\t//g' -e 's/ //g'`

        rm $tmpFile
        cd ../../scripts/

        # Write to log file
        echo "$model,$modelCmd,$workerThreads,$scheduleQCount,$statsRefined" >> $logFile

        sleep 10
    done
}

logfile=""

trap control_c SIGINT

. $1

