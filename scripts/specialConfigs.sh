#!/bin/bash
# Allows batch runs of simulations. Saves results to log files

for config in config/special/* ; do
    echo $config
    ./configSimulate $config
done
