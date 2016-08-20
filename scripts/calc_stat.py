#!/usr/bin/python

# Combines and averages the given csv file(s) using the given settings

from __future__ import print_function
import csv, sys
import numpy as np
import scipy.stats as sps
import pandas as pd

###### Settings go here ######
filterList = ['Model' , 'WorkerThreadCount' , 'ScheduleQCount' , 'ChainSize']
searchAttr = 'Runtime'
outFileName = 'logs/consolidated_result.csv'

###### Don't edit below here ######

def mean_confidence_interval(data, confidence=0.95):
    # check the input is not empty
    if not data:
        raise StatsError('mean_ci - no data points passed')
    a = 1.0*np.array(data)
    n = len(a)
    m, se = np.mean(a), sps.sem(a)
    h = se * sps.t._ppf((1+confidence)/2., n-1)
    return m, m-h, m+h

def median(data):
    # check the input is not empty
    if not data:
        raise StatsError('median - no data points passed')
    return np.median(np.array(data))

def quartiles(data):
    # check the input is not empty
    if not data:
        raise StatsError('quartiles - no data points passed')
    sorts = sorted(data)
    mid = len(sorts) / 2
    if (len(sorts) % 2 == 0):
        # even
        lowerQ = median(sorts[:mid])
        upperQ = median(sorts[mid:])
    else:
        # odd
        lowerQ = median(sorts[:mid])  # same as even
        upperQ = median(sorts[mid+1:])
    return lowerQ, upperQ

def main():
    inFileName = sys.argv[1]
    # Load data from csv file
    data = pd.read_csv(inFileName)

    runtimeList = data.groupby(filterList)
    results = runtimeList[searchAttr].describe()
    results.to_csv(outFileName, sep=',')

if __name__ == "__main__":
    main()
