#!/usr/bin/python

import glob
import os, sys
import pandas as pd
import numpy as np
from matplotlib import pyplot as plt

###### Settings go here ######

threadFilter    =   {   'active'    : False,
                        'value'     : 16
                    }

plotDetails     =   {   'xaxis'     : 'Scheduling-Technique',
                        'yaxis'     : 'Speedup_w.r.t._Sequential_Simulation_Mean',
                        'ylabel'    : 'Speedup',
                        'filename'  : 'consolidated',
                        'sorted'    : False,
                        'quantile'  : 0.95
                    }

solutionList    =   [
                        {   'search': 'stats/scheduleq/threads_vs_count_key_type_',
                            'xaxis' : ['Schedule_Queue_Type', 'Worker_Thread_Count', 'Schedule_Queue_Count'],
                            'xlabel': ['', '-th', '-sq'],
                            'label' : ''        },

                        {   'search': 'stats/chains/threads_vs_count_key_chainsize_',
                            'xaxis' : ['Chain_Size', 'Worker_Thread_Count', 'Schedule_Queue_Count'],
                            'xlabel': ['size', '-th', '-sq'],
                            'label' : 'chain-'  },

                        {   'search': 'stats/blocks/threads_vs_count_key_blocksize_',
                            'xaxis' : ['Block_Size', 'Worker_Thread_Count', 'Schedule_Queue_Count'],
                            'xlabel': ['size', '-th', '-sq'],
                            'label' : 'block-'  },

                        {   'search': 'stats/bags/threads_vs_fractionwindow_key_staticwindow_0',
                            'xaxis' : ['Worker_Thread_Count', 'Fraction_of_Total_Window'],
                            'xlabel': ['th', '-fw'],
                            'label' : 'bag-'    },

                        {   'search': 'stats/bags/threads_vs_staticwindow_key_fractionwindow_1.0',
                            'xaxis' : ['Worker_Thread_Count', 'Static_Window_Size'],
                            'xlabel': ['th', '-sw'],
                            'label' : 'bag-'    }
                    ]


###### Don't edit below here ######

def plotBar(dirPath):
    # Read dataset
    statFile = dirPath + 'stats/' + plotDetails['filename'] + '.csv'
    df = pd.read_csv(statFile)

    xName = plotDetails['xaxis']
    yName = plotDetails['yaxis']
    yAxisLabel = plotDetails['ylabel']

    # Sort data in descending order (if needed)
    if plotDetails['sorted']:
        df.sort(yName, inplace=True, ascending=False, kind='quicksort')

    # Retain only the top x% of data
    quantVal = plotDetails['quantile']
    threshold = df[yName].quantile(quantVal)
    df = df[df[yName] >= threshold]

    # Build the bar plot
    quantPert = str(quantVal*100)
    plotLabel = dirPath.rsplit('/', 2)[-2].replace('_', '-').upper() + ' ' +\
                                yAxisLabel + ' >= ' + quantPert + 'th percentile'
    if threadFilter['active']:
        plotLabel += '\nwith worker thread count = ' + str(threadFilter['value'])
    ax = df.plot(kind='barh', title=plotLabel, grid=True, legend=False, x=xName, fontsize=5)
    ax.set_xlabel(yAxisLabel)
    plt.tight_layout()

    plotFile = dirPath + 'plots/' + plotDetails['filename'] + '.pdf'
    plt.savefig(plotFile, width=0.8)


def calc_and_plot(dirPath):

    fieldX = plotDetails['xaxis']
    fieldY = plotDetails['yaxis']

    outFile = dirPath + 'stats/' + plotDetails['filename'] + '.csv'
    statFile = open(outFile,'w')
    statFile.write(fieldX + ',' + fieldY + '\n')
    statFile.close()

    for solution in solutionList:
        searchPattern = dirPath + solution['search'] + '*'

        for name in glob.glob(searchPattern):
            if not os.path.exists(name):
                print(name + ' not available')
                sys.exit()

            data = pd.read_csv(name, sep=',')

            result = pd.DataFrame(columns=[fieldX, fieldY])
            for index, row in data.iterrows():
                if threadFilter['active'] and \
                        data.at[index,'Worker_Thread_Count'] != threadFilter['value']:
                    continue
                yValue = data.at[index,fieldY]
                xValue = solution['label']
                for xaxisname, xaxislabel in zip(solution['xaxis'], solution['xlabel']):
                    xValue = xValue + xaxislabel + str(data.at[index,xaxisname])
                result.loc[xValue] = [xValue, yValue]

            if not result.empty:
                result.to_csv(outFile, mode='a', index=False, header=False, sep=',')

    plotBar(dirPath)


def main():
    dirPath = sys.argv[1]
    if not os.path.exists(dirPath):
        print('Invalid path to source')
        sys.exit()

    calc_and_plot(dirPath)

if __name__ == "__main__":
    main()


