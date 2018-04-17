#!/usr/bin/python

# Calculates statistics and plots the bag metrics from raw data

from __future__ import print_function
import csv
import os, sys
import numpy as np
import scipy as sp
import scipy.stats as sps
import pandas as pd
import re, shutil, tempfile
import itertools, operator
import subprocess
import Gnuplot
import Gnuplot.funcutils

###### Settings go here ######

filterAttrsList = [ [   'Model' , 'ModelCommand' , 'WorkerThreadCount' , 'StaticBagWindowSize'  ], \
                    [   'Model' , 'ModelCommand' , 'WorkerThreadCount' , 'FracBagWindow'        ] \
                  ]

outputList      = [ 'bags_threads_vs_static_window_size', \
                    'bags_threads_vs_frac_window' \
                  ]

metricList      = [ 'EventCommitmentRate', \
                    'TotalRollbacks', \
                    'Runtime', \
                    'AvgMaxMemory' \
                  ]

rawDataFileName = 'bags.csv'

statType        = [ 'Mean', \
                    'CI_Lower', \
                    'CI_Upper', \
                    'Median', \
                    'Lower_Quartile', \
                    'Upper_Quartile' \
                  ]


###### Don't edit below here ######

def mean_confidence_interval(data, confidence=0.95):
    # check the input is not empty
    if not data:
        raise RuntimeError('mean_ci - no data points passed')
    a = 1.0*np.array(data)
    n = len(a)
    m, se = np.mean(a), sps.sem(a)
    h = se * sps.t._ppf((1+confidence)/2., n-1)
    return m, m-h, m+h

def median(data):
    # check the input is not empty
    if not data:
        raise RuntimeError('median - no data points passed')
    return np.median(np.array(data))

def quartiles(data):
    # check the input is not empty
    if not data:
        raise RuntimeError('quartiles - no data points passed')
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
        raise RuntimeError('statistics - no data points passed')

    mean = ci_lower = ci_upper = med = lower_quartile = upper_quartile = data[0]
    if len(data) > 1:
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

def getIndex(aList, text):
    '''Returns the index of the requested text in the given list'''
    for i,x in enumerate(aList):
        if x == text:
            return i

def plot(data, fileName, title, xaxisLabel, yaxisLabel, linePreface):
    g = Gnuplot.Gnuplot()
    g.title(title)
    g("set terminal svg enhanced background rgb 'white' size 1000,800 fname 'Helvetica' fsize 16")
    g("set key inside top right font ',12' width 1.8")
    g("set grid")
    g("set autoscale")
    g.xlabel(xaxisLabel)
    g.ylabel(yaxisLabel)
    g('set output "' + fileName + '"')
    d = []
    for key in data['data']:
        result = Gnuplot.Data(data['header'][key],data['data'][key],with_="linespoints",title=linePreface+key)
        d.append(result)
    g.plot(*d)

def plot_stats(dirPath, fileName, xaxisLabel, keyLabel):
    outDir = dirPath + "plots/"
    if not os.path.exists(outDir):
        os.makedirs(outDir)

    inFile = dirPath + "stats/" + fileName + ".csv"
    reader = csv.reader(open(inFile,'rb'))
    header = reader.next()

    # Get Column Values for use below
    xaxis  = getIndex(header, xaxisLabel)
    kid    = getIndex(header, keyLabel)

    reader = sorted(reader, key=lambda x: int(x[xaxis]), reverse=False)
    reader = sorted(reader, key=lambda x: x[kid], reverse=False)

    modelId = getIndex(header, 'Model')
    for index, data in itertools.groupby(reader, lambda x: x[modelId]):
        model = [item[modelId] for item in data][0]

    for metric in metricList:

        columnName  = metric + "_" + statType[0]
        columnIndex = getIndex(header, columnName)

        outData = {'header':{},'data':{}}

        # Populate the header
        for kindex, kdata in itertools.groupby(reader, lambda x: x[kid]):
            if kindex not in outData['header']:
                outData['header'][kindex] = []
            for xindex, data in itertools.groupby(kdata, lambda x: x[xaxis]):
                outData['header'][kindex].append(xindex)

        # Populate the statistical data
        for xindex, data in itertools.groupby(reader, lambda x: x[xaxis]):
            for kindex, kdata in itertools.groupby(data, lambda x: x[kid]):
                if kindex not in outData['data']:
                    outData['data'][kindex] = []
                value = [item[columnIndex] for item in kdata][0]
                outData['data'][kindex].append(value)

        title = model.upper() + ' : Performance of Bags for ' + keyLabel + ' keys'
        outFile = outDir + fileName + "_" + metric + ".svg"
        plot(outData, outFile, title, xaxisLabel, metric+'('+statType[0]+')', '')

def calc_and_plot(dirPath):
    # Load data from csv file
    inFile = dirPath + rawDataFileName
    if not os.path.exists(inFile):
        print('Bags raw data not available')
        sys.exit()

    data = pd.read_csv(inFile, sep=',')

    data['EventCommitmentRate'] = data['EventsProcessed'] / data['EventsCommitted']
    data['TotalRollbacks'] = data['PrimaryRollbacks'] + data['SecondaryRollbacks']

    for index, filterAttrs in enumerate(filterAttrsList):
        # Create the filtered list
        filterList = data.groupby(filterAttrs)
        columnNames = list(filterAttrs)

        frames = []
        for metric in metricList:
            # Generate stats for EventsProcessed
            columnNames += [metric + '_' + x for x in statType]
            stats = filterList.apply(lambda x : statistics(x[metric].tolist()))
            # Append the result
            frames.append(stats)

        result = pd.concat(frames, keys=filterAttrs, axis=1)

        # Write to the csv
        outDir = dirPath + "stats/"
        if not os.path.exists(outDir):
            os.makedirs(outDir)
        outFile = outDir + outputList[index] + ".csv"
        statFile = open(outFile,'w')
        for colName in columnNames:
            statFile.write(colName + ",")
        statFile.write("\n")
        statFile.close()
        result.to_csv(outFile, mode='a', header=False, sep=',')

        # Remove " from the newly created csv file
        # Note: It is needed since pandas package has an unresolved bug for 
        # quoting arg which retains the double quotes for column attributes.
        sed_inplace(outFile, r'"', '')

        # Plot the statistics
        plot_stats(dirPath, outputList[index], filterAttrs[-2], filterAttrs[-1])


def main():
    dirPath = sys.argv[1]
    if not os.path.exists(dirPath):
        print('Invalid path to source')
        sys.exit()

    calc_and_plot(dirPath)

if __name__ == "__main__":
    main()


