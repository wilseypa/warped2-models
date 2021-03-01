#!/usr/bin/env python3

try:
    import sys
    import os
    import time
    import re
    import argparse
    import json
    import pandas as pd
    import logging
    import logging.config
    import pathlib
    from concurrent_log_handler import ConcurrentRotatingFileHandler
    from math import radians, cos, sin, asin, sqrt
    from datetime import datetime, timedelta
    import subprocess
    from frechetdist import frdist
except Exception as e:
    print(str(type(e).__name__) + ": " + str(e), file=sys.stderr)
    sys.exit(1)

DIRPATH_PLOTSRC_DATA = "./tuningapp/plotSourceData"
DIRPATH_PLOTSRC_DATA_US = DIRPATH_PLOTSRC_DATA + "/US"


def calc_frechet():
    """
    """

    actual_filepath = DIRPATH_PLOTSRC_DATA_US + "/actual.csv"
    simulated_filepath = DIRPATH_PLOTSRC_DATA_US + "/simulated.csv"

    actual_us_df = pd.read_csv(actual_filepath, skipinitialspace=True,
                               usecols=['Date', 'Confirmed', 'Deaths',
                                        'Recovered',
                                        'Active',
                                        'Population'],
                               dtype={'Confirmed': int, 'Deaths': int,
                                      'Recovered': int,
                                      'Active': int,
                                      'Population': int})

    simulated_us_df = pd.read_csv(simulated_filepath, skipinitialspace=True,
                                  usecols=['Date', 'Confirmed', 'Deaths',
                                           'Recovered',
                                           'Active',
                                           'Population'],
                                  dtype={'Confirmed': int, 'Deaths': int,
                                         'Recovered': int,
                                         'Active': int,
                                         'Population': int})

    
    print(simulated_us_df['Confirmed'], type(list(simulated_us_df['Confirmed'])))
    print("len", len(actual_us_df), len(simulated_us_df))
    actual_list = list(actual_us_df['Confirmed'])
    simulated_list = list(simulated_us_df['Confirmed'])

    print(actual_list, type(actual_list))
    print(simulated_list, type(simulated_list))    

    actual_list_2d = []
    simulated_list_2d = []
    
    for i in range(len(actual_list)):
        actual_list_2d.append([i, actual_list])
        simulated_list_2d.append([i, simulated_list])

    frdist(actual_list_2d, simulated_list_2d)
    

# MAIN
if __name__ == '__main__':
    calc_frechet()
