#! /bin/bash
SIM_TIME=$1
cd models/epidemic/
./epidemic_sim --max-sim-time ${SIM_TIME} -m model-lp10000-p1000000-ba.config --time-warp-worker-threads 3 --time-warp-scheduler-count 3
echo "Simulation complete"