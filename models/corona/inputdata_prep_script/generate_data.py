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
                            filename='coronadata_errors.log',
                            level=logging.DEBUG)

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
                                     'file in csv or json format')
    parser.add_argument('--covid_data', help='JHU_CSSE Covid19 data csv filepath',
                        required=True)
    parser.add_argument('--pop_data', help='Population data csv filepath',
                        required=True)
    parser.add_argument('--date', help='Date of dataset in YYYY-MM-DD format',
                        required=False)
    parser.add_argument('--format_json', help='Dump output as json', action='store_true',
                        default=False,
                        required=False)

    args = parser.parse_args()

    return (args.covid_data, args.pop_data, args.date, args.format_json)


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
    clusterDFs = [pd.DataFrame(columns=mainDF.columns)] * len(cluster_centers)

    for index, row in mainDF.iterrows():
        addDFrow_to_cluster(clusterDFs, cluster_centers, row)

    # TODO delete original mainDF??

    mainDF = pd.concat(clusterDFs, ignore_index=True)

    return mainDF


def prepare_data():

    """
    read data from JHU CSSE github repo (from '/csse_covid_19_data/csse_covid_19_daily_reports' directory),
    combine it with population data, and
    output data (to be supplied to corona simulation program) in json / plain-text format, depending on user choice
    """

    # hard-coded values
    transmissibility, mean_incubation_duration_in_days, mean_infection_duration_in_days,\
        mortality_ratio, update_trig_interval_in_hrs, diffusion_trig_interval_in_hrs = 2.2, 2.2,\
            2.3, 0.05, 24, 48

    disease_model = [transmissibility, mean_incubation_duration_in_days,
                     mean_infection_duration_in_days, mortality_ratio,
                     update_trig_interval_in_hrs, diffusion_trig_interval_in_hrs]


    # store user input
    (covid_csse_data_filepath, pop_data_filepath, data_date, format_json) = \
        parse_cmdargs()

    out_fname = os.path.splitext(os.path.basename(covid_csse_data_filepath))[0] + ".jhu-source-data"

    if format_json:
        out_fname = out_fname + ".json"

    # create file object to output data
    outdatafileobj = open(out_fname, 'w')

    # create DF from CSSE data, add new column in order to 'join' with population data
    csse_data_daily_report_df = pd.read_csv(covid_csse_data_filepath, skipinitialspace=True,
                                            usecols=['FIPS','Admin2','Province_State','Country_Region',
                                                     'Lat','Long_','Confirmed','Deaths',
                                                     'Recovered','Active','Combined_Key'],
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
                                                   'Active':'int64',
                                                   'Combined_Key':'object'})

    csse_data_daily_report_df['Combined_Key_US'] = csse_data_daily_report_df.Admin2 + "," \
        + csse_data_daily_report_df.Province_State

    # create DF from population data, add 'Combined_Key_US' column
    population_data_df = pd.read_csv(pop_data_filepath, skipinitialspace=True)
    population_data_df['Combined_Key_US'] = population_data_df.County + "," + population_data_df.State

    # now, join
    csse_data_daily_report_merged_df = pd.merge(csse_data_daily_report_df,
                                                population_data_df[['Combined_Key_US', 'Population']],
                                                on='Combined_Key_US', how='left')

    # dropna ASK
    csse_data_daily_report_merged_df['FIPS'].dropna(inplace=True)

    # replace na values
    # csse_data_daily_report_merged_df['FIPS'].fillna(value='', inplace=True)
    csse_data_daily_report_merged_df['Admin2'].fillna(value='', inplace=True)
    csse_data_daily_report_merged_df['Province_State'].fillna(value='', inplace=True)
    csse_data_daily_report_merged_df['Country_Region'].fillna(value='', inplace=True)
    csse_data_daily_report_merged_df['Lat'].fillna(value=-999.9, inplace=True)
    csse_data_daily_report_merged_df['Long_'].fillna(value=-999.9, inplace=True)
    csse_data_daily_report_merged_df['Confirmed'].fillna(value=-1, inplace=True)
    csse_data_daily_report_merged_df['Deaths'].fillna(value=-1, inplace=True)
    csse_data_daily_report_merged_df['Recovered'].fillna(value=-1, inplace=True)
    csse_data_daily_report_merged_df['Active'].fillna(value=-1, inplace=True)

    # merged data has population column converted to float type, convert it back to int
    csse_data_daily_report_merged_df['Population'] = \
        csse_data_daily_report_merged_df['Population'].fillna(-1.0).astype(int)

    # TODO output merge errors to log file

    # drop unnecessary columns
    csse_data_daily_report_merged_df.drop(columns=['Combined_Key_US'], inplace=True)

    # TODO doesnt work why?? sort_df_rows_into_clusters(csse_data_daily_report_merged_df)
    csse_data_daily_report_merged_df = sort_df_rows_into_clusters(csse_data_daily_report_merged_df)

    # for json output, build a dictionary before writing it out
    final_dict = {
        "disease_model": {
            "transmissibility": transmissibility,
            "mean_incubation_duration_in_days": mean_incubation_duration_in_days,
            "mean_infection_duration_in_days": mean_infection_duration_in_days,
            "mortality_ratio": mortality_ratio,
            "update_trig_interval_in_hrs": update_trig_interval_in_hrs,
            "diffusion_trig_interval_in_hrs": diffusion_trig_interval_in_hrs,
            "data_date": data_date
        },
        "locations": None
    }

    if format_json:
        # write out main dictionary to json file
        final_dict["locations"] = csse_data_daily_report_merged_df.values.tolist()
        # TODO pretty print json??
        outdatafileobj.write(json.dumps(final_dict))
    else:
        # for plain text output, write out (comma-seperated) disease model to outfile
        strout = ','.join(str(v) for v in disease_model + [data_date])
        outdatafileobj.write(strout + "\n")
        outdatafileobj.write(csse_data_daily_report_merged_df.to_csv(header=False, index=False))


# MAIN
if __name__ == '__main__':
    prepare_data()

