#!/usr/bin/python

# Combines and averages the given csv file(s) using the given settings

from __future__ import print_function
import csv, sys
import numpy as np
import scipy.stats as sps
import pandas as pd
import re, shutil, tempfile


###### Settings go here ######
filterAttr = ['Model' , 'WorkerThreadCount' , 'ScheduleQCount' , 'ChainSize']
searchAttr = 'Runtime'
outFileName = 'logs/consolidated_result.csv'
columnList = ['Mean' , 'CI_Lower' , 'CI_Upper' , 'Median' , 'Lower_Quartile' , 'Upper_Quartile']

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

def statistics(data):
    # check the input is not empty
    if not data:
        raise StatsError('statistics - no data points passed')
    mean, ci_lower, ci_upper = mean_confidence_interval(data)
    med = median(data)
    lower_quartile, upper_quartile = quartiles(data)
    statList = (str(mean), str(ci_lower), str(ci_upper), str(med), str(lower_quartile), str(upper_quartile))
    return ",".join(statList)

def sed_inplace(filename, pattern, repl):
    # For efficiency, precompile the passed regular expression.
    pattern_compiled = re.compile(pattern)

    # For portability, NamedTemporaryFile() defaults to mode "w+b" (i.e., binary
    # writing with updating). In this case, binary writing imposes non-trivial 
    # encoding constraints resolved by switching to text writing.
    with tempfile.NamedTemporaryFile(mode='w', delete=False) as tmp_file:
        with open(filename) as src_file:
            for line in src_file:
                tmp_file.write(pattern_compiled.sub(repl, line))

    # Overwrite the original file with the temporary file in a
    # manner preserving file attributes (e.g., permissions).
    shutil.copystat(filename, tmp_file.name)
    shutil.move(tmp_file.name, filename)

def main():
    inFileName = sys.argv[1]
    # Load data from csv file
    data = pd.read_csv(inFileName)

    # Create the filtered list
    filterList = data.groupby(filterAttr)
    results = filterList.apply(lambda x : statistics(x[searchAttr].tolist()))
    results.to_csv(outFileName, header=[",".join(columnList)], sep=',')

    # Remove " from the newly created csv file
    sed_inplace(outFileName, r'"', '')

if __name__ == "__main__":
    main()
