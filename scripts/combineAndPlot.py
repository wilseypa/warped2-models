#!/usr/bin/python

# Combines and averages the given csv file(s) using the given settings

from __future__ import print_function
import csv, sys
import itertools, operator
import subprocess
import Gnuplot
import Gnuplot.funcutils

###### Settings go here ######
TITLE = "Ladder Queue Threshold Manipulation for RAID-5 - Speedup - "
# What to filter by (what data to use) - ex only use threads = 16
FILTER = 1
FILTERCOLUMN1 = "ScheduleQCount"
FILTERVALUE1 = "4"
FILTERCOLUMN2 = "CausalityType"
FILTERVALUE2_1 = "Strict"
FILTERVALUE2_2 = "Relaxed"

TITLE = TITLE+FILTERVALUE1+" LTSF"

# Lines (what to split into lines by) - enter column header
LINES = "Ladder Threshold"
LINEPREFACE = "" ## Appended to beginning of line name in plot

# X-axis
XAXIS = "Threads"
XAXISLABEL = "Number of Threads"

# Y-axis (to take median of)
YAXIS = "Runtime"
YAXISLABEL = "Speedup"

###### Don't edit below here ######

def getIndex(aList, text):
    '''Returns the index of the requested text in the given list'''
    for i,x in enumerate(aList):
        if x == text:
            return i

def median(mylist):
    sorts = sorted(mylist)
    length = len(sorts)
    if length == 0:
        return 0
    if not length % 2:
        return (sorts[length / 2] + sorts[length / 2 - 1]) / 2.0
    return sorts[length / 2]

def plot(data, title, xaxisLabel, yaxisLabel, linePreface):
    g = Gnuplot.Gnuplot()
    g.title(title)
    g("set terminal postscript size 10.0,10.0 enhanced color font 'Helvetica,14' linewidth 2")
    g("set key right top")
    g.xlabel(xaxisLabel)
    g.ylabel(yaxisLabel)
    g('set output "graph.eps"')
    d = []
    #print(data)
    for key in data['data']:
        result = Gnuplot.Data(data['header'],data['data'][key],with_="linespoints",title=linePreface+key)
        d.append(result)
    g.plot(*d)

def main():
    inFileName = sys.argv[1]
    reader = csv.reader(open(inFileName,'rb'))
    header = reader.next()

    writer = open("/tmp/avg_data", "wb")

    # Get Column Values for use below
    nFilterColumn1 = getIndex(header, FILTERCOLUMN1)
    nFilterColumn2 = getIndex(header, FILTERCOLUMN2)
    nLines= getIndex(header, LINES)
    nXaxis = getIndex(header, XAXIS)

    # Column to take median of
    MEDCOL = getIndex(header, YAXIS)

    reader = [i for i in reader if i[nFilterColumn1] == FILTERVALUE1]
    reader1 = [i for i in reader if i[nFilterColumn2] == FILTERVALUE2_1]
    reader2 = [i for i in reader if i[nFilterColumn2] == FILTERVALUE2_2]
    reader1 = sorted(reader1, key=lambda x: x[nLines], reverse=False)
    reader1 = sorted(reader1, key=lambda x: int(x[nXaxis]), reverse=False)
    reader2 = sorted(reader2, key=lambda x: x[nLines], reverse=False)
    reader2 = sorted(reader2, key=lambda x: int(x[nXaxis]), reverse=False)

    outData = {'header':[],'data':{}}
    # First sorting criteria (loadBalancing) - different lines
    for sqCount, data in itertools.groupby(reader1, lambda x: x[nXaxis]):
        # Label column
        outData['header'].append(int(sqCount))
        # Second sorting criteria (sqCount) - x-axis
        for loadBalancing, lbdata in itertools.groupby(data, lambda x: x[nLines]):
            subList1 = []
            for row in lbdata:
                if row[MEDCOL] != '': # Filter out empty results
                    subList1.append(float(row[MEDCOL]))
                label = row[nLines]
            medVal = median(subList1)
            if label not in outData['data']:
                outData['data'][label] = []
            outData['data'][label].append(medVal)

    for sqCount, data in itertools.groupby(reader2, lambda x: x[nXaxis]):
        # Second sorting criteria (sqCount) - x-axis
        for loadBalancing, lbdata in itertools.groupby(data, lambda x: x[nLines]):
            subList2 = []
            for row in lbdata:
                if row[MEDCOL] != '': # Filter out empty results
                    subList2.append(float(row[MEDCOL]))
                label = row[nLines]
            medVal = median(subList2)
            x = outData['data'][label].pop(0)
            x /= medVal
            outData['data'][label].append(x)

    plot(outData, TITLE, XAXISLABEL, YAXISLABEL, LINEPREFACE)

if __name__ == "__main__":
    main()
