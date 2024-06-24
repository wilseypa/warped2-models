#!/bin/bash
# Allows batch runs of simulations. Saves results to log files

for config in config/gaurav/* ; do
    echo $config
    ./configSimulate.sh $config
done
