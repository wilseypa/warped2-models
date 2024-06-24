#!/bin/bash
# Allows batch runs of simulations. Saves results to log files


# run if user hits control-c
function control_c() {
    echo -en "*** Ouch! Exiting ***\n"
    exit $?
}


# Install WARPED-2 and build WARPED-2 models
# build <rootPath> <gitBranch> <mpiIncludePath> <mpiLibraryPath> <additionalFlags>
function build {
    rootPath=$1
    gitBranch=$2
    export mpiIncludePath=/usr/lib/x86_64-linux-gnu/openmpi/include
    additionalFlags=$5

    garbageSearch="cincinnati"

    if [ "$additionalFlags" != "" ]
    then
        echo -e "\nInstalling WARPED2:$gitBranch with additional flag(s) $additionalFlags"
    else
        echo -e "\nInstalling WARPED2:$gitBranch"
    fi

    cd $rootPath/warped2/
    git restore *
    git checkout $gitBranch
    git pull
    autoreconf -i | grep $garbageSearch
    ./configure --with-mpi-includedir=$mpiIncludePath \
        --prefix=$rootPath/installation/ \
        $additionalFlags CXXFLAGS='-g -O3'| grep $garbageSearch
    make -s clean all | grep $garbageSearch
    make -j 8 install | grep $garbageSearch

    echo -e "Building WARPED2-MODELS"

    cd $rootPath/warped2-models/
    autoreconf -i | grep $garbageSearch
    ./configure --with-warped=$rootPath/installation/ CXXFLAGS='-g -O3' CXX=mpicxx | grep $garbageSearch
    make -j 8 | grep $garbageSearch

    cd $rootPath/warped2-models/scripts/

    buildCmd="build $rootPath $gitBranch $mpiIncludePath $mpiLibraryPath \"$additionalFlags\""
    echo $buildCmd >> $errlogFile

    sleep 10
}

# Install WARPED-2 and build WARPED-2 models for LadderQ, Unsorted Bottom and Lockfree
# buildLadder <rootPath> <gitBranch> <mpiIncludePath> <mpiLibraryPath> <additionalFlags> <bottomSize>
function buildLadder {
    rootPath=$1
    gitBranch=$2
    mpiIncludePath=$3
    mpiLibraryPath=$4
    additionalFlags=$5
    bottomSize=$6

    garbageSearch="cincinnati"

    if [ "$additionalFlags" != "" ]
    then
        echo -e "\nInstalling LadderQueue / Unsorted Bottom with flag(s) $additionalFlags"
    else
        echo -e "\nInstalling Lockfree Unsorted Bottom"
    fi

    echo -e "Bottom Size = $bottomSize"

    cd $rootPath/warped2/
    git checkout src/LadderQueue.hpp
    git checkout $gitBranch
    git pull
    sed -i '/#define THRESHOLD/c\#define THRESHOLD '$bottomSize'' src/LadderQueue.hpp
    autoreconf -i | grep $garbageSearch
    ./configure --with-mpi-includedir=$mpiIncludePath \
        --with-mpi-libdir=$mpiLibraryPath --prefix=$rootPath/installation/ \
        $additionalFlags | grep $garbageSearch
    make -s clean all | grep $garbageSearch
    make install | grep $garbageSearch
    git checkout src/LadderQueue.hpp

    echo -e "Building WARPED2-MODELS"

    cd $rootPath/warped2-models/
    autoreconf -i | grep $garbageSearch
    ./configure --with-warped=$rootPath/installation/ | grep $garbageSearch
    make -s clean all | grep $garbageSearch

    cd $rootPath/warped2-models/scripts/

    buildCmd="buildLadder $rootPath $gitBranch $mpiIncludePath $mpiLibraryPath \"$additionalFlags\" $bottomSize"
    echo $buildCmd >> $errlogFile

    sleep 10
}


# Create the Louvain partition profile for bags
# bagProfile <model> <modelCmd> <seqSimTime> <fileName>
function bagProfile {
    model=$1
    modelCmd=$2
    seqSimTime=$3
    fileName=$4

    cd ../models/$model/
    echo -e "\nProfile $modelCmd , File: $fileName , Max Sim Time: $seqSimTime"
    $modelCmd --max-sim-time $seqSimTime --simulation-type "sequential" \
            --sequential-statistics-type "louvain" --sequential-statistics-file $fileName

    cd ../../scripts/

    buildCmd="bagProfile $model \"$modelCmd\" $seqSimTime $fileName"
    echo $buildCmd >> $errlogFile

    sleep 10
}


# Run certain number of iterations for a bag configuration
# runBag    <testCycles> <timeoutPeriod> <model> <modelCmd> <maxSimTime>
#           <workerThreads> <staticBagWindowSize> <fracBagWindow> <gvtMethod>
#           <gvtPeriod> <stateSavePeriod> <partitioningFile>
function runBag {
    testCycles=$1
    timeoutPeriod=$2
    model=$3
    modelCmd=$4
    maxSimTime=$5
    workerThreads=$6
    staticBagWindowSize=$7
    fracBagWindow=$8
    gvtMethod=$9
    gvtPeriod=${10}
    stateSavePeriod=${11}
    partitioningFile=${12}

    logFile="logs/bags.csv"

    header="Model,Model_Command,Max_Simulation_Time,Worker_Thread_Count,Static_Window_Size,\
            Fraction_of_Total_Window,GVT_Method,GVT_Period,State_Save_Period,Simulation_Runtime_(secs.),\
            Number_of_Objects,Number_of_Partitions,Local_Positive_Events_Sent,\
            Remote_Positive_Events_Sent,Local_Negative_Events_Sent,Remote_Negative_Events_Sent,\
            Primary_Rollbacks,Secondary_Rollbacks,Coast_Forwarded_Events,Cancelled_Events,\
            Events_Processed,Events_Committed,Average_Memory_Usage_(MB)"

    headerRefined=`echo $header | sed -e 's/\t//g' -e 's/ //g'`

    if grep --quiet --no-messages $headerRefined $logFile
    then
        echo -e "\nData will now be recorded in $logFile"
    else
        echo -e "\nNew logfile $logFile created"
        echo $headerRefined >> $logFile
    fi

    for ((iteration=1; iteration <= $testCycles; iteration++))
    do
        cd ../models/$model/
        outMsg="\n($iteration/$testCycles) $modelCmd : $workerThreads threads, \
                bags-$partitioningFile, static bag window size: $staticBagWindowSize, \
                fraction of bag window: $fracBagWindow, GVT: $gvtMethod-$gvtPeriod, \
                state saving period: $stateSavePeriod, max sim time: $maxSimTime"
        echo -e $outMsg

        tmpFile=`tempfile`
        runCommand="$modelCmd \
                    --max-sim-time $maxSimTime \
                    --time-warp-worker-threads $workerThreads \
                    --time-warp-gvt-calculation-method $gvtMethod \
                    --time-warp-gvt-calculation-period $gvtPeriod  \
                    --time-warp-state-saving-period $stateSavePeriod \
                    --time-warp-partitioning-type profile-guided \
                    --time-warp-partitioning-file $partitioningFile \
                    --time-warp-statistics-file $tmpFile"

        timeout $timeoutPeriod bash -c "$runCommand" | grep -e "Simulation completed in "

        statsRaw=`cat $tmpFile | grep "Total,"`
        rm $tmpFile
        cd ../../scripts/

        if [ "$statsRaw" != "" ]
        then
            # Parse stats
            # Write to log file
            totalStats="$model,$modelCmd,$maxSimTime,$workerThreads,$staticBagWindowSize,\
                        $fracBagWindow,$gvtMethod,$gvtPeriod,$stateSavePeriod,$statsRaw"
            statsRefined=`echo $totalStats | sed -e 's/Total,//g' -e 's/\t//g' -e 's/ //g'`
            echo $statsRefined >> $logFile
        else
            errMsg="runBag 1 $timeoutPeriod $model \"$modelCmd\" $maxSimTime \
                    $workerThreads $staticBagWindowSize $fracBagWindow $gvtMethod \
                    $gvtPeriod $stateSavePeriod $partitioningFile"
            errMsgRefined=`echo $errMsg | sed -e 's/\t//g'`
            echo $errMsgRefined >> $errlogFile
        fi

        sleep 10
    done
}

# Run bulk simulations for bags
# permuteConfigBag  <testCycles> <timeoutPeriod> <model> <modelCmd> <arrMaxSimTime>
#                   <arrWorkerThreads> <arrStaticBagWindowSize> <arrFracBagWindow>
#                   <arrGvtMethod> <arrGvtPeriod> <arrStateSavePeriod> <partitioningFile>
function permuteConfigBag() {
    testCycles=$1
    timeoutPeriod=$2
    model=$3
    modelCmd=$4
    local -n arrMaxSimTime=$5
    local -n arrWorkerThreads=$6
    local -n arrStaticBagWindowSize=$7
    local -n arrFracBagWindow=$8
    local -n arrGvtMethod=$9
    local -n arrGvtPeriod=${10}
    local -n arrStateSavePeriod=${11}
    partitioningFile=${12}

    for gvtMethod in "${arrGvtMethod[@]}"
    do
        for gvtPeriod in "${arrGvtPeriod[@]}"
        do
            for stateSavePeriod in "${arrStateSavePeriod[@]}"
            do
                for maxSimTime in "${arrMaxSimTime[@]}"
                do
                    for fracBagWindow in "${arrFracBagWindow[@]}"
                    do
                        for staticBagWindowSize in "${arrStaticBagWindowSize[@]}"
                        do
                            for workerThreads in "${arrWorkerThreads[@]}"
                            do
                                runBag  $testCycles $timeoutPeriod $model "$modelCmd" \
                                        $maxSimTime $workerThreads $staticBagWindowSize \
                                        $fracBagWindow $gvtMethod $gvtPeriod $stateSavePeriod \
                                        $partitioningFile
                            done
                        done
                    done
                done
            done
        done
    done
}


# Run simulations for chain scheduling
# runChain  <testCycles> <timeoutPeriod> <model> <modelCmd>
#           <maxSimTime> <workerThreads> <scheduleQCount>
#           <chainSize> <isLpMigrationOn> <gvtMethod>
#           <gvtPeriod> <stateSavePeriod>
function runChain {
    testCycles=$1
    timeoutPeriod=$2
    model=$3
    modelCmd=$4
    maxSimTime=$5
    workerThreads=$6
    scheduleQCount=$7
    chainSize=$8
    isLpMigrationOn=$9
    gvtMethod=${10}
    gvtPeriod=${11}
    stateSavePeriod=${12}

    logFile="logs/chains.csv"

    header="Model,Model_Command,Max_Simulation_Time,Worker_Thread_Count,Schedule_Queue_Count,\
            Chain_Size,is_LP_Migration_ON,GVT_Method,GVT_Period,State_Save_Period,\
            Simulation_Runtime_(secs.),Number_of_Objects,Local_Positive_Events_Sent,\
            Remote_Positive_Events_Sent,Local_Negative_Events_Sent,Remote_Negative_Events_Sent,\
            Primary_Rollbacks,Secondary_Rollbacks,Coast_Forwarded_Events,Cancelled_Events,\
            Events_Processed,Events_Committed,Average_Memory_Usage_(MB)"

    headerRefined=`echo $header | sed -e 's/\t//g' -e 's/ //g'`

    if grep --quiet --no-messages $headerRefined $logFile
    then
        echo -e "\nData will now be recorded in $logFile"
    else
        echo -e "\nNew logfile $logFile created"
        echo $headerRefined >> $logFile
    fi

    for ((iteration=1; iteration <= $testCycles; iteration++))
    do
        cd ../models/$model/
        outMsg="\n($iteration/$testCycles) $modelCmd : $workerThreads threads, \
                $scheduleQCount schedule queue(s), chain size: $chainSize, \
                is LP migration on: $isLpMigrationOn, GVT: $gvtMethod-$gvtPeriod, \
                state saving period: $stateSavePeriod, max sim time: $maxSimTime"
        echo -e $outMsg

        tmpFile=`tempfile`
        runCommand="$modelCmd \
                    --max-sim-time $maxSimTime \
                    --time-warp-worker-threads $workerThreads \
                    --time-warp-scheduler-count $scheduleQCount \
                    --time-warp-group-size $chainSize \
                    --time-warp-lp-migration $isLpMigrationOn \
                    --time-warp-gvt-calculation-method $gvtMethod \
                    --time-warp-gvt-calculation-period $gvtPeriod  \
                    --time-warp-state-saving-period $stateSavePeriod \
                    --time-warp-statistics-file $tmpFile"

        timeout $timeoutPeriod bash -c "$runCommand" | grep -e "Simulation completed in " -e "Type of Schedule queue: "

        statsRaw=`cat $tmpFile | grep "Total,"`
        rm $tmpFile
        cd ../../scripts/

        if [ "$statsRaw" != "" ]
        then
            # Parse stats
            # Write to log file
            totalStats="$model,$modelCmd,$maxSimTime,$workerThreads,$scheduleQCount,$chainSize,\
                        $isLpMigrationOn,$gvtMethod,$gvtPeriod,$stateSavePeriod,$statsRaw"
            statsRefined=`echo $totalStats | sed -e 's/Total,//g' -e 's/\t//g' -e 's/ //g'`
            echo $statsRefined >> $logFile
        else
            errMsg="runChain 1 $timeoutPeriod $model \"$modelCmd\" $maxSimTime \
                    $workerThreads $scheduleQCount $chainSize $isLpMigrationOn \
                    $gvtMethod $gvtPeriod $stateSavePeriod"
            errMsgRefined=`echo $errMsg | sed -e 's/\t//g'`
            echo $errMsgRefined >> $errlogFile
        fi

        sleep 10
    done
}

# Run simulations for block scheduling
# runBlock  <testCycles> <timeoutPeriod> <model> <modelCmd>
#           <maxSimTime> <workerThreads> <scheduleQCount>
#           <blockSize> <isLpMigrationOn> <gvtMethod>
#           <gvtPeriod> <stateSavePeriod>
function runBlock {
    testCycles=$1
    timeoutPeriod=$2
    model=$3
    modelCmd=$4
    maxSimTime=$5
    workerThreads=$6
    scheduleQCount=$7
    blockSize=$8
    isLpMigrationOn=$9
    gvtMethod=${10}
    gvtPeriod=${11}
    stateSavePeriod=${12}

    logFile="logs/blocks.csv"

    header="Model,Model_Command,Max_Simulation_Time,Worker_Thread_Count,Schedule_Queue_Count,\
            Block_Size,is_LP_Migration_ON,GVT_Method,GVT_Period,State_Save_Period,\
            Simulation_Runtime_(secs.),Number_of_Objects,Local_Positive_Events_Sent,\
            Remote_Positive_Events_Sent,Local_Negative_Events_Sent,Remote_Negative_Events_Sent,\
            Primary_Rollbacks,Secondary_Rollbacks,Coast_Forwarded_Events,Cancelled_Events,\
            Events_Processed,Events_Committed,Average_Memory_Usage_(MB)"

    headerRefined=`echo $header | sed -e 's/\t//g' -e 's/ //g'`

    if grep --quiet --no-messages $headerRefined $logFile
    then
        echo -e "\nData will now be recorded in $logFile"
    else
        echo -e "\nNew logfile $logFile created"
        echo $headerRefined >> $logFile
    fi

    for ((iteration=1; iteration <= $testCycles; iteration++))
    do
        cd ../models/$model/
        outMsg="\n($iteration/$testCycles) $modelCmd : $workerThreads threads, \
                $scheduleQCount schedule queue(s), block size: $chainSize, \
                is LP migration on: $isLpMigrationOn, GVT: $gvtMethod-$gvtPeriod, \
                state saving period: $stateSavePeriod, max sim time: $maxSimTime"
        echo -e $outMsg

        tmpFile=`tempfile`
        runCommand="$modelCmd \
                    --max-sim-time $maxSimTime \
                    --time-warp-worker-threads $workerThreads \
                    --time-warp-scheduler-count $scheduleQCount \
                    --time-warp-group-size $blockSize \
                    --time-warp-lp-migration $isLpMigrationOn \
                    --time-warp-gvt-calculation-method $gvtMethod \
                    --time-warp-gvt-calculation-period $gvtPeriod  \
                    --time-warp-state-saving-period $stateSavePeriod \
                    --time-warp-statistics-file $tmpFile"

        timeout $timeoutPeriod bash -c "$runCommand" | grep -e "Simulation completed in "

        statsRaw=`cat $tmpFile | grep "Total,"`
        rm $tmpFile
        cd ../../scripts/

        if [ "$statsRaw" != "" ]
        then
            # Parse stats
            # Write to log file
            totalStats="$model,$modelCmd,$maxSimTime,$workerThreads,$scheduleQCount,$blockSize,\
                        $isLpMigrationOn,$gvtMethod,$gvtPeriod,$stateSavePeriod,$statsRaw"
            statsRefined=`echo $totalStats | sed -e 's/Total,//g' -e 's/\t//g' -e 's/ //g'`
            echo $statsRefined >> $logFile
        else
            errMsg="runBlock 1 $timeoutPeriod $model \"$modelCmd\" $maxSimTime \
                    $workerThreads $scheduleQCount $blockSize $isLpMigrationOn \
                    $gvtMethod $gvtPeriod $stateSavePeriod"
            errMsgRefined=`echo $errMsg | sed -e 's/\t//g'`
            echo $errMsgRefined >> $errlogFile
        fi

        sleep 10
    done
}

# Run bulk simulations for blocks and chains
# permuteConfigGroup  <testCycles> <timeoutPeriod> <model> <modelCmd> <arrMaxSimTime>
#           <arrWorkerThreads> <arrScheduleQCount> <groupType> <arrGroupSize>
#           <arrLpMigration> <arrGvtMethod> <arrGvtPeriod> <arrStateSavePeriod>
function permuteConfigGroup() {
    testCycles=$1
    timeoutPeriod=$2
    model=$3
    modelCmd=$4
    local -n arrMaxSimTime=$5
    local -n arrWorkerThreads=$6
    local -n arrScheduleQCount=$7
    groupType=$8
    local -n arrGroupSize=$9
    local -n arrLpMigration=${10}
    local -n arrGvtMethod=${11}
    local -n arrGvtPeriod=${12}
    local -n arrStateSavePeriod=${13}

    for gvtMethod in "${arrGvtMethod[@]}"
    do
        for gvtPeriod in "${arrGvtPeriod[@]}"
        do
            for stateSavePeriod in "${arrStateSavePeriod[@]}"
            do
                for maxSimTime in "${arrMaxSimTime[@]}"
                do
                    for isLpMigrationOn in "${arrLpMigration[@]}"
                    do
                        for scheduleQCount in "${arrScheduleQCount[@]}"
                        do
                            for workerThreads in "${arrWorkerThreads[@]}"
                            do
                                for groupSize in "${arrGroupSize[@]}"
                                do
                                    if [ "$groupType" == "chain" ]
                                    then
                                        runChain    $testCycles $timeoutPeriod $model "$modelCmd" \
                                                    $maxSimTime $workerThreads $scheduleQCount \
                                                    $groupSize $isLpMigrationOn $gvtMethod \
                                                    $gvtPeriod $stateSavePeriod
                                    else
                                        runBlock    $testCycles $timeoutPeriod $model "$modelCmd" \
                                                    $maxSimTime $workerThreads $scheduleQCount \
                                                    $groupSize $isLpMigrationOn $gvtMethod \
                                                    $gvtPeriod $stateSavePeriod
                                    fi
                                done
                            done
                        done
                    done
                done
            done
        done
    done
}


# Run simulations for STL MultiSet, Splay, LadderQ, Unsorted Bottom (Locked and Lockfree)
# runScheduleQ  <testCycles> <timeoutPeriod> <model> <modelCmd>
#               <maxSimTime> <workerThreads> <scheduleQType>
#               <scheduleQCount> <isLpMigrationOn> <gvtMethod>
#               <gvtPeriod> <stateSavePeriod>
function runScheduleQ {
    testCycles=$1
    timeoutPeriod=$2
    model=$3
    modelCmd=$4
    maxSimTime=${5}
    workerThreads=${6}
    scheduleQCount=${7}
    gvtMethod=${8}
    gvtPeriod=${9}
    stateSavePeriod=${10}

    logFile="logs/scheduleq.csv"

    header="Model,Model_Command,Max_Simulation_Time,Worker_Thread_Count,Schedule_Queue_Type,\
            Schedule_Queue_Count,is_LP_Migration_ON,GVT_Method,GVT_Period,State_Save_Period,\
            Simulation_Runtime_(secs.),Number_of_Objects,Local_Positive_Events_Sent,\
            Remote_Positive_Events_Sent,Local_Negative_Events_Sent,Remote_Negative_Events_Sent,\
            Primary_Rollbacks,Secondary_Rollbacks,Coast_Forwarded_Events,Cancelled_Events,\
            Events_Processed,Events_Committed,Events_for_Starved_Objects,\
            Sched_Event_Swaps_Success,Sched_Event_Swaps_Failed,Average_Memory_Usage_(MB)"

    headerRefined=`echo $header | sed -e 's/\t//g' -e 's/ //g'`

    if grep --quiet --no-messages $headerRefined $logFile
    then
        echo -e "\nData will now be recorded in $logFile"
    else
        echo -e "\nNew logfile $logFile created"
        echo $headerRefined >> $logFile
    fi

    for ((iteration=1; iteration <= $testCycles; iteration++))
    do
        cd ../models/$model/
        outMsg="\n($iteration/$testCycles) $modelCmd : $workerThreads threads, \
                $scheduleQCount , is LP migration on: $isLpMigrationOn, \
                GVT: $gvtMethod-$gvtPeriod, state saving period: $stateSavePeriod, \
                max sim time: $maxSimTime"
        echo -e $outMsg

        tmpFile=`tempfile`
        echo -e "non unified $sceduleQCount"
        runCommand="$modelCmd \
                    --max-sim-time $maxSimTime \
                    --time-warp-worker-threads $workerThreads \
                    --time-warp-scheduler-count $scheduleQCount \
                    --time-warp-gvt-calculation-method $gvtMethod \
                    --time-warp-gvt-calculation-period $gvtPeriod  \
                    --time-warp-state-saving-period $stateSavePeriod \
                    --time-warp-statistics-file $tmpFile"

        timeout $timeoutPeriod bash -c "$runCommand" | grep -e "Simulation completed in " -e "Type of Schedule queue: "

        statsRaw=`cat $tmpFile | grep "Total,"`
        rm $tmpFile
        cd ../../scripts/

        if [ "$statsRaw" != "" ]
        then
            # Parse stats
            # Write to log file
            totalStats="$model,$modelCmd,$maxSimTime,$workerThreads,"multiset",\
                        $scheduleQCount,$isLpMigrationOn,$gvtMethod,$gvtPeriod,\
                        $stateSavePeriod,$statsRaw"
            statsRefined=`echo $totalStats | sed -e 's/Total,//g' -e 's/\t//g' -e 's/ //g'`
            echo $statsRefined >> $logFile
        else
            errMsg="runScheduleQ 1 $timeoutPeriod $model \"$modelCmd\" $maxSimTime \
                    $workerThreads \"$scheduleQType\" $scheduleQCount $isLpMigrationOn \
                    $gvtMethod $gvtPeriod $stateSavePeriod"
            errMsgRefined=`echo $errMsg | sed -e 's/\t//g'`
            echo $errMsgRefined >> $errlogFile
        fi

        sleep 10
    done
}



# Run simulations for STL MultiSet, Splay, LadderQ, Unsorted Bottom (Locked and Lockfree)
# runScheduleQ  <testCycles> <timeoutPeriod> <model> <modelCmd>
#               <maxSimTime> <workerThreads> <scheduleQType>
#               <scheduleQCount> <isLpMigrationOn> <gvtMethod>
#               <gvtPeriod> <stateSavePeriod>
function runUnifiedQ {
    testCycles=$1
    timeoutPeriod=$2
    model=$3
    modelCmd=$4
    maxSimTime=${5}
    workerThreads=${6}
    gvtPeriod=${7}
    stateSavePeriod=${8}

    logFile="logs/scheduleq.csv"

    header="Model,Model_Command,Max_Simulation_Time,Worker_Thread_Count,Schedule_Queue_Type,\
            Schedule_Queue_Count,is_LP_Migration_ON,GVT_Method,GVT_Period,State_Save_Period,\
            Simulation_Runtime_(secs.),Number_of_Objects,Local_Positive_Events_Sent,\
            Remote_Positive_Events_Sent,Local_Negative_Events_Sent,Remote_Negative_Events_Sent,\
            Primary_Rollbacks,Secondary_Rollbacks,Coast_Forwarded_Events,Cancelled_Events,\
            Events_Processed,Events_Committed,Events_for_Starved_Objects,\
            Sched_Event_Swaps_Success,Sched_Event_Swaps_Failed,Average_Memory_Usage_(MB)"

    headerRefined=`echo $header | sed -e 's/\t//g' -e 's/ //g'`

    if grep --quiet --no-messages $headerRefined $logFile
    then
        echo -e "\nData will now be recorded in $logFile"
    else
        echo -e "\nNew logfile $logFile created"
        echo $headerRefined >> $logFile
    fi

    for ((iteration=1; iteration <= $testCycles; iteration++))
    do
        cd ../models/$model/
        outMsg="\n($iteration/$testCycles) $modelCmd : $workerThreads threads, \
                $workerThread , is LP migration on: $isLpMigrationOn, \
                GVT: $gvtMethod-$gvtPeriod, state saving period: $stateSavePeriod, \
                max sim time: $maxSimTime"
        echo -e $outMsg

        tmpFile=`tempfile`
        
        runCommand="$modelCmd \
                --max-sim-time $maxSimTime \
                --time-warp-worker-threads $workerThreads \
                --time-warp-gvt-calculation-period $gvtPeriod  \
                --time-warp-state-saving-period $stateSavePeriod \
                --time-warp-statistics-file $tmpFile"
        timeout $timeoutPeriod bash -c "$runCommand" | grep -e "Simulation completed in " -e "Type of Schedule queue: "

        statsRaw=`cat $tmpFile | grep "Total,"`
        rm $tmpFile
        cd ../../scripts/

        if [ "$statsRaw" != "" ]
        then
            # Parse stats
            # Write to log file
            totalStats="$model,$modelCmd,$maxSimTime,$workerThreads, "multiset"\
                        $workerThreads,$isLpMigrationOn,$gvtMethod,$gvtPeriod,\
                        $stateSavePeriod,$statsRaw"
            
            statsRefined=`echo $totalStats | sed -e 's/Total,//g' -e 's/\t//g' -e 's/ //g'`
            echo $statsRefined >> $logFile
        else
            errMsg="runScheduleQ 1 $timeoutPeriod $model \"$modelCmd\" $maxSimTime \
                    $workerThreads \"$scheduleQType\" $workerThreads $isLpMigrationOn \
                    $gvtMethod $gvtPeriod $stateSavePeriod"
            errMsgRefined=`echo $errMsg | sed -e 's/\t//g'`
            echo $errMsgRefined >> $errlogFile
        fi

        sleep 10
    done
}

# Run bulk simulations for stl-multiset, splay-tree, ladder-queue, unsorted-bottom and lockfree
# permuteConfigScheduleQ  <testCycles> <timeoutPeriod> <model> <modelCmd>
#           <arrMaxSimTime> <arrWorkerThreads> <scheduleQType> <arrScheduleQCount>
#           <arrLpMigration> <arrGvtMethod> <arrGvtPeriod> <stateSavePeriod>
function permuteConfigScheduleQ() {
    #print all arguments
    echo -e "permuteConfigScheduleQ: $@"
    echo -e "1 : $1"
    echo -e "2 : $2"
    echo -e "3 : $3"
    echo -e "4 : $4"
    echo -e "5 : $5"
    echo -e "6 : $6"
    echo -e "7 : $7"
    echo -e "8 : $8"
    echo -e "9 : $9" 
    testCycles=$1
    timeoutPeriod=$2
    model=$3
    modelCmd=$4
    local -n arrMaxSimTime=${5}
    local -n arrWorkerThreads=${6}
    local -n arrScheduleQCount=${6}
    local -n arrGvtMethod=${8}
    local -n arrGvtPeriod=${9}
    local -n arrStateSavePeriod=${10}

    outMsg="\warped : $modelCmd : max sim time: $arrMaxSimTime, : worker threads: $arrWorkerThreads, \
             schedule queue count: $arrScheduleQCount, \
            GVT: $arrGvtMethod-$arrGvtPeriod, state saving period: $arrStateSavePeriod"
    echo -e $outMsg

    for gvtMethod in "${arrGvtMethod[@]}"
    do
        for gvtPeriod in "${arrGvtPeriod[@]}"
        do
            for stateSavePeriod in "${arrStateSavePeriod[@]}"
            do
                for maxSimTime in "${arrMaxSimTime[@]}"
                do 
                    for workerThreads in "${arrWorkerThreads[@]}"
                    do
                        echo -e "hmm went to last loop"
                        runScheduleQ    $testCycles $timeoutPeriod $model "$modelCmd" \
                                        $maxSimTime $workerThreads \
                                        $workerThreads $gvtMethod \
                                        $gvtPeriod $stateSavePeriod
                        #workerThreads =schedule queue count
                    done
                done
            done
        done
    done
}


function permuteConfigUnifiedQ() {
    #print all arguments
    echo -e "permuteConfigScheduleQ: $@"
    echo -e "1 : $1"
    echo -e "2 : $2"
    echo -e "3 : $3"
    echo -e "4 : $4"
    echo -e "5 : $5"
    echo -e "6 : $6"
    echo -e "7 : $7"
    echo -e "8 : $8"
    echo -e "9 : $9" 
    testCycles=$1
    timeoutPeriod=$2
    model=$3
    modelCmd=$4
    local -n arrMaxSimTime=${5}
    local -n arrWorkerThreads=${6}
    local -n arrGvtMethod=${7}
    local -n arrGvtPeriod=${8}
    local -n arrStateSavePeriod=${9}

    outMsg="\warped : $modelCmd : max sim time: $arrMaxSimTime, : worker threads: $arrWorkerThreads, \
            GVT: $arrGvtMethod-$arrGvtPeriod, state saving period: $arrStateSavePeriod"
    echo -e $outMsg

    for gvtMethod in "${arrGvtMethod[@]}"
    do
        for gvtPeriod in "${arrGvtPeriod[@]}"
        do
            for stateSavePeriod in "${arrStateSavePeriod[@]}"
            do
                for maxSimTime in "${arrMaxSimTime[@]}"
                do 
                    for workerThreads in "${arrWorkerThreads[@]}"
                    do
                        echo -e "hmm went to last unified loop"
                        runUnifiedQ    $testCycles $timeoutPeriod $model "$modelCmd" \
                                        $maxSimTime $workerThreads \
                                        $gvtPeriod $stateSavePeriod
                        #workerThreads =schedule queue count
                    done
                done
            done
        done
    done
}



# Run sequential simulation
# runSequential <timeoutPeriod> <model> <modelCmd> <maxSimTime>
function runSequential {
    timeoutPeriod=$1
    model=$2
    modelCmd=$3
    maxSimTime=$4

    outMsg="\nSequential : $modelCmd : max sim time: $maxSimTime"
    echo -e $outMsg

    cd ../models/$model/
    runCommand="$modelCmd \
                --simulation-type sequential \
                --max-sim-time $maxSimTime"
    result=$(timeout $timeoutPeriod bash -c "$runCommand" | \
                grep -e "Simulation completed in " -e "Events processed: " \
                -e "LP count: " | grep -Eo '[+-]?[0-9]+([.][0-9]+)?')
    echo -e "$result"

    cd ../../scripts/
    logFile="logs/sequential.dat"
    echo $result > $logFile

    sleep 10
}

hostName=`hostname`
date=`date +"%m-%d-%y_%T"`
errlogFile="logs/errlog_${date}.config"

trap control_c SIGINT

. $1

