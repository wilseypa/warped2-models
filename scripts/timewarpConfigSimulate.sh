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
    modelBinary=$4
    modelConfig=$5
    maxSimTime=$6
    workerThreads=$7
    schedulerCount=$8

    for ((iteration=1; iteration <= $testCycles; ++iteration));
    do
        cd ../models/$model/
        outMsg="\n($iteration/$testCycles) $model $modelConfig : $workerThreads threads, \
                                                                $schedulerCount schedulers"
        echo -e $outMsg
        runCommand="./$modelBinary $modelConfig --max-sim-time $maxSimTime \
                    --simulation-type time-warp --time-warp-worker-threads $workerThreads \
                    --time-warp-scheduler-count $schedulerCount"
        grepMe=`timeout $timeoutPeriod bash -c "$runCommand" | grep "Simulation completed in "`
        runTime=`echo $grepMe | sed -e 's/.*in \(.*\) second.*/\1/'`
        rollbacks=`echo $grepMe | sed -e 's/.*with \(.*\) rollback.*/\1/'`
        echo "Simulation time: $runTime secs, Rollbacks: $rollbacks"
        cd ../../scripts/

        # Write to log file
        echo "$model,$modelConfig,$workerThreads,$schedulerCount,$runTime,$rollbacks" >> $logFile

        sleep 10
    done
}

hostname=`hostname`
date=`date +"%m-%d-%y_%T"`
logFile="logs/$hostname---$date.csv"

# Write csv header
## Simulation Threads ScheduleQCount CausalityType Runtime Rollbacks
echo "Model,ModelConfig,WorkerThreadCount,SchedulerCount,Runtime,Rollbacks" > $logFile

trap control_c SIGINT

. $1

#run 10 5 epidemic epidemic_sim "-m model_5500_obj.xml" 100000 4 2

