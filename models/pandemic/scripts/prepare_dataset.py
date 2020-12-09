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

DEFAULT_JHU_CSSE_PATH = '../data/COVID-19.jhu/'
DEFAULT_POPULATION_FILEPATH = '../data/US_counties_population_latLong.csv'
SCRIPTS_LOGS_DIR_PATH = "./logs/"
DEFAULT_GRAPH_TYPE = 'ws'
DEFAULT_GRAPH_INPUT_PARAM_STRING = '8,0.1'

logger_prepare_dataset = None


def setup_logging():
    """
    """
    try:
        pass
        # print("logger", logger)

        global logger_prepare_dataset

        logger_prepare_dataset = logging.getLogger(__name__)

        rotateHandler = ConcurrentRotatingFileHandler(os.path.abspath(SCRIPTS_LOGS_DIR_PATH + \
                                                                      "pandemic_backend_scripts.log"),
                                                      "a", 1024 * 1024 * 4, 100)

        logFormatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
        rotateHandler.setFormatter(logFormatter)

        logger_prepare_dataset.addHandler(rotateHandler)
        logger_prepare_dataset.setLevel(logging.DEBUG)

        # logger_prepare_dataset.info("from prepare_dataset")


    except Exception as e:
        print(str(type(e).__name__) + ": " + str(e), file=sys.stderr)
        sys.exit(1)


def parse_cmdargs(population_data, graph_input_param_string):
    """
    Using argparse module, get commandline arguments
    :return:
    """
    parser = argparse.ArgumentParser(description='Parse Covid19 dataset from JHU-CSSE, add '
                                                 'population data and dump combined data to '
                                                 'file in json format')
    parser.add_argument('--jhu_repo_path', help='path of JHU CSSE repo',
                        default=DEFAULT_JHU_CSSE_PATH,
                        required=False)
    parser.add_argument('--date', help='date for which corresponding Covid data should be '
                                       'pulled from JHU_CSSE repo, in MM-DD-YYYY format',
                        required=True)

    parser.add_argument('--population_data', help='filepath for Population data csv',
                        default=DEFAULT_POPULATION_FILEPATH,
                        required=False)

    # parser.add_argument('--exposed_confirmed_ratio', help='ratio of exposed to confirmed',
    #                     type=float,
    #                     default=2.5,
    #                     required=False)

    parser.add_argument('--graph_type', help="'ws' for Watts-Strogatz, and 'ba' for Barabasi-Albert",
                        choices=['ws', 'ba'],
                        required=True)

    parser.add_argument('--graph_input_param_string', help='comma-seperated input parameter list for selected '
                                                           'graph type, in form of a string',
                        default=DEFAULT_GRAPH_INPUT_PARAM_STRING,
                        required=False)

    args = parser.parse_args()

    return (args.jhu_repo_path, args.date, args.population_data, args.graph_type, args.graph_input_param_string)


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
    a = sin(dlat / 2) ** 2 + cos(latA) * cos(latB) * sin(dlon / 2) ** 2
    c = 2 * asin(sqrt(a))
    r = 6371  # Radius of earth in kilometers. Use 3956 for miles
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
                                            usecols=['Admin2', 'Province_State', 'Country_Region',
                                                     'Confirmed',
                                                     'Deaths',
                                                     'Recovered',
                                                     'Active'],
                                            dtype={'Admin2': 'object',
                                                   'Province_State': 'object'})

    # replace na values
    csse_data_daily_report_df['Admin2'].fillna(value='', inplace=True)
    csse_data_daily_report_df['Province_State'].fillna(value='', inplace=True)
    csse_data_daily_report_df['Country_Region'].fillna(value='', inplace=True)

    # create an extra column for merging
    csse_data_daily_report_df['Combined_Key_US'] = csse_data_daily_report_df.Admin2.str.lower() + "," \
                                                   + csse_data_daily_report_df.Province_State.str.lower()

    # now, create DF from population data, add 'Combined_Key_US' column
    population_data_df = pd.read_csv(pop_data_filepath, dtype={'FIPS': object}, skipinitialspace=True)
    population_data_df['Combined_Key_US'] = population_data_df.County.str.lower() + "," + \
                                            population_data_df.State.str.lower()

    logger_prepare_dataset.info(
        "Merging main DataFrame [{} rows, {} columns] with population DataFrame [{} rows, {} columns] ...".format(
            csse_data_daily_report_df.shape[0],
            csse_data_daily_report_df.shape[1],
            population_data_df.shape[0],
            population_data_df.shape[1]))

    # now, join
    csse_data_daily_report_merged_df = pd.merge(population_data_df,
                                                csse_data_daily_report_df[['Country_Region',
                                                                           'Confirmed',
                                                                           'Deaths',
                                                                           'Recovered',
                                                                           'Active',
                                                                           'Combined_Key_US']],

                                                on='Combined_Key_US',
                                                how='left')

    # due to left join, there'd be unmatched rows with NA values, fill them
    csse_data_daily_report_merged_df['Country_Region'].fillna(value='US', inplace=True)

    # replace NA disease metric values with 0
    csse_data_daily_report_merged_df['Confirmed'].fillna(value=0, inplace=True, downcast='infer')
    csse_data_daily_report_merged_df['Deaths'].fillna(value=0, inplace=True, downcast='infer')
    csse_data_daily_report_merged_df['Recovered'].fillna(value=0, inplace=True, downcast='infer')
    csse_data_daily_report_merged_df['Active'].fillna(value=0, inplace=True, downcast='infer')

    # reorder columns
    csse_data_daily_report_merged_df = csse_data_daily_report_merged_df[['FIPS', 'County', 'State', 'Country_Region',
                                                                         'Lat', 'Long_', 'Confirmed', 'Deaths',
                                                                         'Recovered', 'Active', 'Population']]

    logger_prepare_dataset.info(
        "Merged DataFrame has: [{} rows, {} columns]".format(csse_data_daily_report_merged_df.shape[0],
                                                             csse_data_daily_report_merged_df.shape[1]))

    # TODO output merge errors to log file

    logger_prepare_dataset.info("Sorting DataFrame to group rows in clusters ...")
    csse_data_daily_report_merged_df = sort_df_rows_into_clusters(csse_data_daily_report_merged_df)

    logger_prepare_dataset.info("Sorted DataFrame has: [{} rows, {} columns]".format(
        csse_data_daily_report_merged_df.shape[0], csse_data_daily_report_merged_df.shape[1]))

    return csse_data_daily_report_merged_df


def get_jhu_csse_data_filepath(jhu_csse_path, dateStr):
    """
    """
    # check for JHU github folder
    if not os.path.isdir(jhu_csse_path):
        raise Exception("JHU CSSE github repo does not exist in {} directory".format(jhu_csse_path))

    filepath = jhu_csse_path + "/csse_covid_19_data/csse_covid_19_daily_reports/" + dateStr + ".csv"

    if not os.path.isfile(filepath):
        raise Exception("File does not exist. Make sure date requested is valid, and run 'git pull'")

    return filepath


def fix_metrics_values(jhu_csse_path, main_df, curr_date_str):
    """
    """
    # print("h1\n", main_df.dtypes)

    curr_date = datetime.strptime(curr_date_str, "%m-%d-%Y")

    prev_date_filepath = get_jhu_csse_data_filepath(jhu_csse_path,
                                                    (curr_date - timedelta(days=14)).strftime("%m-%d-%Y"))

    # print("prev_date_filepath", prev_date_filepath)
    # print("main dtypes", main_df.dtypes)

    prev_date_csse_df = pd.read_csv(prev_date_filepath, skipinitialspace=True, dtype={'FIPS': 'object'})  # ',
    # 'Active':'object'})
    if 'FIPS' not in prev_date_csse_df.columns:
        return

    # print(prev_date_csse_df.iloc[2000:2010])
    # sys.exit(1)

    # print("prev_date_csse_df dtypes\n", prev_date_csse_df.dtypes)
    # print("prev_date_csse_df[FIPS]", list(prev_date_csse_df['FIPS']))
    # print(prev_date_csse_df.loc[prev_date_csse_df.FIPS == '32003'])
    # sys.exit(1)

    for index, row in main_df.iterrows():

        fips = row['FIPS']

        # print("type", type(fips))
        # print("fips", fips)

        # sys.exit(1)

        # if not fips:
        #     print("null true")
        #     sys.exit(1)

        if fips not in prev_date_csse_df['FIPS'].values:
            # print("h3")
            continue

        prev_active_val = int((prev_date_csse_df[prev_date_csse_df['FIPS'] == fips]['Active']).to_list()[0])

        main_active_val = int(main_df[main_df['FIPS'] == fips]['Active'])
        main_confirmed_val = int(main_df[main_df['FIPS'] == fips]['Confirmed'])
        # main_recovered_val = int(main_df[main_df['FIPS'] == fips]['Recovered'])
        main_deaths_val = int(main_df[main_df['FIPS'] == fips]['Deaths'])

        # print("prev_active_val", prev_active_val, main_active_val)

        # print(main_df[main_df['FIPS'] == fips])
        # print("h2")
        new_active_val = main_active_val - prev_active_val
        new_recovered_val = main_confirmed_val - new_active_val - main_deaths_val

        if new_active_val < 0:
            new_active_val = 0

        if new_recovered_val < 0:
            new_recovered_val = 0

        main_df.loc[(main_df['FIPS'] == fips), 'Active'] = new_active_val
        main_df.loc[(main_df['FIPS'] == fips), 'Recovered'] = new_recovered_val

        # print(main_df.loc[index])

        # print(main_df[main_df['FIPS'] == fips])
        # sys.exit(1)

        # main_df[main_df['FIPS'] == fips]['Active'] = main_active_val - prev_active_val


def prepare_data(jhu_csse_path=DEFAULT_JHU_CSSE_PATH, covid_csse_data_date=None,
                 pop_data_filepath=DEFAULT_POPULATION_FILEPATH,
                 # exposed_confirmed_ratio=2.5,
                 graph_type=DEFAULT_GRAPH_TYPE,
                 graph_input_param_string=DEFAULT_GRAPH_INPUT_PARAM_STRING,
                 logger=None):
    """
    read data from JHU CSSE github repo (from '/csse_covid_19_data/csse_covid_19_daily_reports' directory),
    combine it with population data, and write output data to file (to serve as input to covid simulation program)
    in json / plain-text format, depending on user choice

    returns output json filepath
    """

    try:
        # parse command-line arguments
        # global logger

        # TODO add comment
        if not covid_csse_data_date:
            # print("h1", covid_csse_data_date, pop_data_filepath, graph_type, graph_input_param_string)
            # print("h2", "calling cmdargs")
            (jhu_csse_path, covid_csse_data_date, pop_data_filepath, graph_type,
             graph_input_param_string) = parse_cmdargs(pop_data_filepath, graph_input_param_string)

        # pathlib.Path(SCRIPTS_LOGS_DIR_PATH).mkdir(parents=True, exist_ok=True)
        setup_logging()

        # Default values
        (transmissibility, exposed_confirmed_ratio, mean_incubation_duration_in_days,
         mean_infection_duration_in_days,
         mortality_ratio,
         update_trig_interval_in_hrs,
         diffusion_trig_interval_in_hrs,
         avg_transport_speed,
         max_diffusion_cnt) = 2.2, 2.5, 2.2, 2.3, 0.05, 24, 48, 100, 10

        covid_csse_data_filepath = get_jhu_csse_data_filepath(jhu_csse_path, covid_csse_data_date)

        # get csv`filename and set output json full filepath
        out_fname = os.path.splitext(os.path.basename(covid_csse_data_filepath))[0] + ".formatted-JHU-data.json"
        out_filepath = os.getcwd() + "/../data/" + out_fname

        if os.path.isfile(out_filepath):
            logger_prepare_dataset.info("File [{}] already exists ...".format(out_filepath))
            logger_prepare_dataset.info("Exiting ...")
            return (out_filepath)

        disease_model = {
            "transmissibility": transmissibility,
            "exposed_confirmed_ratio": exposed_confirmed_ratio,
            "mean_incubation_duration_in_days": mean_incubation_duration_in_days,
            "mean_infection_duration_in_days": mean_infection_duration_in_days,
            "mortality_ratio": mortality_ratio,
            "update_trig_interval_in_hrs": update_trig_interval_in_hrs,
        }

        diffusion_model = {
            "graph_type": ("Watts-Strogatz" if graph_type == "ws" else "Barabasi-Albert"),
            "graph_params": graph_input_param_string,
            "diffusion_trig_interval_in_hrs": diffusion_trig_interval_in_hrs,
            "avg_transport_speed": avg_transport_speed,
            "max_diffusion_cnt": max_diffusion_cnt
        }

        # create dataframe
        csse_data_daily_report_merged_sorted_df = create_merged_DF_jhu_population(covid_csse_data_filepath,
                                                                                  pop_data_filepath)
        #
        # covid_csse_data_prev_date = (datetime.strptime(covid_csse_data_date, "%m-%d-%Y") -
        #                              timedelta(days=14)).strftime("%m-%d-%Y")
        #
        # covid_csse_prev_data_filepath = get_jhu_csse_data_filepath(jhu_csse_path, covid_csse_data_prev_date)
        #
        # csse_data_daily_report_merged_sorted_prev_df = \
        #     create_merged_DF_jhu_population(covid_csse_prev_data_filepath,
        #                                     pop_data_filepath)

        fix_metrics_values(jhu_csse_path, csse_data_daily_report_merged_sorted_df, covid_csse_data_date)

        tweakable_params = ["disease_model", "diffusion_model"]

        final_dict = {
            "tweakable_params": tweakable_params,
            "disease_model": disease_model,
            "diffusion_model": diffusion_model,
            "locations": None
        }

        logger_prepare_dataset.info("Writing to file [{}] in json format".format(out_filepath))
        # write out main dictionary to json file
        final_dict["locations"] = csse_data_daily_report_merged_sorted_df.values.tolist()

        proc = subprocess.Popen(['./pretty_print_json', out_filepath], stdin=subprocess.PIPE,
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

        ret_stdout, ret_stderr = proc.communicate(json.dumps(final_dict))

        if proc.returncode == 0:
            logger_prepare_dataset.info("Writing finished with return status: {}".format(proc.returncode))
        else:
            raise Exception("Error in writing to file: [{}]".format(ret_stderr.strip()))

        logger_prepare_dataset.info("Exiting ...\n\n")

        return out_filepath

    except Exception as e:
        logger_prepare_dataset.exception(str(type(e).__name__) + ": " + str(e), exc_info=True)
        logger_prepare_dataset.info("Exiting ...\n\n")

        print("Exception occured, view log file")
        sys.exit(1)


# MAIN
if __name__ == '__main__':
    prepare_data()
