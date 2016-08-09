#!/bin/bash
# Allows batch runs of simulations. Saves results to log files

# run if user hits control-c
function control_c() {
    echo -en "*** Ouch! Exiting ***\n"
    exit $?
}

function run {
    testCycles=$1
    timeoutPeriod=$2
    model=$3
    modelCmd=$4
    maxSimTime=$5
    workerThreads=$6
    schedulerCount=$7
    chainSize=$8

    for ((iteration=1; iteration <= $testCycles; ++iteration));
    do
        cd ../models/$model/
        outMsg="\n($iteration/$testCycles) $modelCmd : $workerThreads threads, \
                $schedulerCount schedulers, chain size: $chainSize, max sim time: $maxSimTime"
        echo -e $outMsg
        runCommand="$modelCmd --max-sim-time $maxSimTime --simulation-type time-warp \
            --time-warp-worker-threads $workerThreads --time-warp-scheduler-count $schedulerCount \
            --time-warp-chain-size $chainSize"
        grepMe=`timeout $timeoutPeriod bash -c "$runCommand" | grep "Simulation completed in "`
        runTime=`echo $grepMe | sed -e 's/.*in \(.*\) second.*/\1/'`
        echo "Simulation time: $runTime secs"
        cd ../../scripts/

        # Write to log file
        echo "$model,$modelCmd,$workerThreads,$schedulerCount,$chainSize,$runTime" >> $logFile

        sleep 10
    done
}

hostname=`hostname`
date=`date +"%m-%d-%y_%T"`
logFile="logs/$hostname---$date.csv"

# Write csv header
## Simulation Threads ScheduleQCount CausalityType Runtime Rollbacks
echo "Model,ModelCommand,WorkerThreadCount,SchedulerCount,ChainSize,Runtime" > $logFile

trap control_c SIGINT

. $1

#run 10 5 epidemic "./epidemic_sim -m model_5500_obj.xml" 100000 4 2

