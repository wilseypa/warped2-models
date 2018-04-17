#!/bin/bash
# Generates statistical analysis and plots for the raw data collected

./plotScheduleQ.py $1
./plotBags.py $1
./plotChains.py $1
./plotBlocks.py $1

