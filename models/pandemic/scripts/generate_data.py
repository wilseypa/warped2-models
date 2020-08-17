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
    from math import radians, cos, sin, asin, sqrt
    from datetime import datetime
    import subprocess
except Exception as e:
    print(str(type(e).__name__) + ": " + str(e), file=sys.stderr)
    sys.exit(1)


cluster_centers = [("Las Vegas", 36.1699, -115.1398),
                   ("Salt Lake City", 40.7608, -111.8910),
                   ("Denver", 39.7392, -104.9903),
                   ("Albuquerque", 35.0844, -106.6504),
                   ("Dallas", 32.7767, -96.7970),
                   ("Kansas city", 39.0997, -94.5786),
                   ("Memphis", 35.1495, -90.0490),
                   ("Charlotte", 35.2271, -80.8431),
                   ("New York", 40.7128, -74.0060),
                   ("Pittsburgh", 40.4406, -79.9959),
                   ("Indianapolis", 39.7684, -86.1581),
                   ("Chicago", 41.8781, -87.6298),
                   ("Minneapolis", 44.9778, -93.2650),
                   ("Bismarck", 46.8083, -100.7837),
                   ("Bozeman", 45.6770, -111.0429),
                   ("Portland", 45.5051, -122.6750),
                   ("Sacramento", 38.5816, -121.4944)]

def setup_logging():
    """
    """
    try:

        logging.basicConfig(format="%(asctime)s [%(levelname)s] : %(message)s",
                            filename='generate_data.log', level=logging.DEBUG)

    except Exception as e:
        print(str(type(e).__name__) + ": " + str(e), file=sys.stderr)
        sys.exit(1)


def parse_cmdargs():
    """
    Using argparse module, get commandline arguments
    :return:
    """
    parser = argparse.ArgumentParser(description='Parse Covid19 dataset from JHU-CSSE, add '
                                     'population data and dump combined data to '
                                     'file in json format')
    parser.add_argument('--covid_data', help='JHU_CSSE Covid19 data csv filepath',
                        required=True)

    parser.add_argument('--pop_data', help='Population data csv filepath',
                        default='../data/county_population_data.csv',
                        required=False)

    parser.add_argument('--graph_type', help="'ws' for Watts-Strogatz, and 'ba' for Barabasi-Albert",
                        choices=['ws', 'ba'],
                        required=True)

    parser.add_argument('--graph_input_param_string', help="comma-seperated input parameter list for selected "
                        "graph type, in form of a string",
                        default="8,0.1",
                        required=False)

    args = parser.parse_args()

    return (args.covid_data, args.pop_data, args.graph_type, args.graph_input_param_string)


def get_dist_between_coordinates(latA, longA, latB, longB):
    """
    haversine
    https://stackoverflow.com/a/4913653/9894266
    """

    # convert decimal degrees to radians
    longA, latA, longB, latB = map(radians, [longA, latA, longB, latB])

    # haversine formula
    dlon = longB - longA
    dlat = latB - latA
    a = sin(dlat/2)**2 + cos(latA) * cos(latB) * sin(dlon/2)**2
    c = 2 * asin(sqrt(a))
    r = 6371 # Radius of earth in kilometers. Use 3956 for miles
    return c * r


def addDFrow_to_cluster(clusterDFs, cluster_centers, row):
    """
    """
    latval = row['Lat']
    longval = row['Long_']

    minInd = -1
    minVal = sys.maxsize

    for ind, center in enumerate(cluster_centers):
        # check
        if not latval or not longval or latval == -1.0 or longval == -1.0:
            continue

        dist = get_dist_between_coordinates(center[1], center[2], latval, longval)

        if dist < minVal:
            minVal = dist
            minInd = ind

    # check
    if minInd == -1:
        return

    clusterDFs[minInd] = clusterDFs[minInd].append(row, ignore_index=True)


def sort_df_rows_into_clusters(mainDF):
    """
    """
    # these DataFrames will store rows corresponding to each geographical partition; essentially
    # storing partitions of original DataFrame
    clusterDFs = [pd.DataFrame(columns=mainDF.columns)] * len(cluster_centers)

    for index, row in mainDF.iterrows():
        addDFrow_to_cluster(clusterDFs, cluster_centers, row)

    # finally, we need a single DataFrame containing rows grouped by the geographical partition they fall in
    mainDF = pd.concat(clusterDFs, ignore_index=True)

    return mainDF


def create_merged_DF_jhu_population(covid_csse_data_filepath, pop_data_filepath):
    """
    """
    # create DF from CSSE data, add new column in order to 'join' with population data
    csse_data_daily_report_df = pd.read_csv(covid_csse_data_filepath, skipinitialspace=True,
                                            usecols=['FIPS','Admin2','Province_State','Country_Region',
                                                     'Lat','Long_','Confirmed','Deaths',
                                                     'Recovered','Active'],
                                            dtype={'FIPS':'object',
                                                   'Admin2':'object',
                                                   'Province_State':'object',
                                                   'Country_Region':'object',
                                                   'Last_Update':'object',
                                                   'Lat':'float64',
                                                   'Long_':'float64',
                                                   'Confirmed':'int64',
                                                   'Deaths':'int64',
                                                   'Recovered':'int64',
                                                   'Active':'int64'})

    # drop all non-US data rows
    csse_data_daily_report_df.dropna(subset=['FIPS'], inplace=True)

    # replace na values
    csse_data_daily_report_df['Admin2'].fillna(value='', inplace=True)
    csse_data_daily_report_df['Province_State'].fillna(value='', inplace=True)
    csse_data_daily_report_df['Country_Region'].fillna(value='', inplace=True)
    csse_data_daily_report_df['Lat'].fillna(value=-999.9, inplace=True)
    csse_data_daily_report_df['Long_'].fillna(value=-999.9, inplace=True)
    csse_data_daily_report_df['Confirmed'].fillna(value=-1, inplace=True)
    csse_data_daily_report_df['Deaths'].fillna(value=-1, inplace=True)
    csse_data_daily_report_df['Recovered'].fillna(value=-1, inplace=True)
    csse_data_daily_report_df['Active'].fillna(value=-1, inplace=True)

    # create an extra column for merging
    csse_data_daily_report_df['Combined_Key_US'] = csse_data_daily_report_df.Admin2 + "," \
        + csse_data_daily_report_df.Province_State

    # now, create DF from population data, add 'Combined_Key_US' column
    population_data_df = pd.read_csv(pop_data_filepath, skipinitialspace=True)
    population_data_df['Combined_Key_US'] = population_data_df.County + "," + population_data_df.State

    logging.info("Merging main DataFrame [{} rows, {} columns] with population DataFrame [{} rows, {} columns] ..."
                 .format(csse_data_daily_report_df.shape[0], csse_data_daily_report_df.shape[1],
                         population_data_df.shape[0], population_data_df.shape[1]))
    # now, join
    csse_data_daily_report_merged_df = pd.merge(csse_data_daily_report_df,
                                                population_data_df[['Combined_Key_US', 'Population']],
                                                on='Combined_Key_US', how='left')

    logging.info("Merged DataFrame has: [{} rows, {} columns]".format(csse_data_daily_report_merged_df.shape[0],
                                                                      csse_data_daily_report_merged_df.shape[1]))

    # merging converts population data to float type, convert it back to int
    csse_data_daily_report_merged_df['Population'] = \
        csse_data_daily_report_merged_df['Population'].fillna(-1.0).astype(int)

    # TODO output merge errors to log file

    # drop unnecessary columns
    csse_data_daily_report_merged_df.drop(columns=['Combined_Key_US'], inplace=True)

    logging.info("Sorting DataFrame to group rows in clusters ...")
    csse_data_daily_report_merged_df = sort_df_rows_into_clusters(csse_data_daily_report_merged_df)

    logging.info("Sorted DataFrame has: [{} rows, {} columns]".format(
        csse_data_daily_report_merged_df.shape[0], csse_data_daily_report_merged_df.shape[1]))

    return csse_data_daily_report_merged_df


def prepare_data():

    """
    read data from JHU CSSE github repo (from '/csse_covid_19_data/csse_covid_19_daily_reports' directory),
    combine it with population data, and write output data to file (to serve as input to covid simulation program)
    in json / plain-text format, depending on user choice
    """

    try:
        # parse command-line arguments
        (covid_csse_data_filepath, pop_data_filepath, graph_type, graph_input_param_string) = \
            parse_cmdargs()

        setup_logging()

        logging.info("")
        logging.info("")
        logging.info("Starting ...")

        # hard-coded values
        transmissibility, mean_incubation_duration_in_days, mean_infection_duration_in_days,\
            mortality_ratio, update_trig_interval_in_hrs, diffusion_trig_interval_in_hrs = 2.2, 2.2,\
                2.3, 0.05, 24, 48


        out_fname = os.path.splitext(os.path.basename(covid_csse_data_filepath))[0] + ".formatted-JHU-data.json"

        # set filepath
        out_filepath = "../data/" + out_fname


        disease_model = {
            "transmissibility": transmissibility,
            "mean_incubation_duration_in_days": mean_incubation_duration_in_days,
            "mean_infection_duration_in_days": mean_infection_duration_in_days,
            "mortality_ratio": mortality_ratio,
            "update_trig_interval_in_hrs": update_trig_interval_in_hrs,
        }

        diffusion_model = {
            "graph_type": ("Watts-Strogatz" if graph_type == "ws" else "Barabasi-Albert"),
            "graph_param_str": graph_input_param_string,
            "diffusion_trig_interval_in_hrs": diffusion_trig_interval_in_hrs
        }

        # create dataframe
        csse_data_daily_report_merged_sorted_df = create_merged_DF_jhu_population(covid_csse_data_filepath,
                                                                                  pop_data_filepath)

        final_dict = {
            "disease_model": disease_model,
            "diffusion_model": diffusion_model,
            "locations": None
        }

        logging.info("Writing to file [{}] in json format".format(out_filepath))
        # write out main dictionary to json file
        final_dict["locations"] = csse_data_daily_report_merged_sorted_df.values.tolist()

        proc = subprocess.Popen(['./pretty_print_json', out_filepath], stdin=subprocess.PIPE,
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

        ret_stdout, ret_stderr = proc.communicate(json.dumps(final_dict))

        if proc.returncode == 0:
            logging.info("Writing finished with return status: {}".format(proc.returncode))
        else:
            raise Exception("Error in writing to file: [{}]".format(ret_stderr.strip()))


        logging.info("Exiting ...\n\n")

    except Exception as e:
        logging.exception(str(type(e).__name__) + ": " + str(e), exc_info=True)
        logging.info("Exiting ...\n\n")

        print("Exception occured, view log file")
        sys.exit(1)


# MAIN
if __name__ == '__main__':
    prepare_data()

