#! /bin/bash
MODEL=$1
SIM_TIME=$2
./models/${MODEL}/${MODEL}_sim --max-sim-time ${SIM_TIME}
