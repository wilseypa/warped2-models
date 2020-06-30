#!/usr/bin/env python3

try:
    import time
    import re
    import argparse
    import json
    import pandas as pd
    import logging
except Exception as e:
    print(str(type(e).__name__) + ": " + str(e), file=sys.stderr)
    sys.exit(1)


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
    parser.add_argument('--out_file', help='Output data filepath',
                        required=True)
    parser.add_argument('--format_json', help='Dump output as json', action='store_true',
                        default=False,
                        required=False)

    args = parser.parse_args()

    return (args.covid_data, args.pop_data, args.date, args.out_file, args.format_json)


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
    (covid_csse_data_filepath, pop_data_filepath, data_date, out_fname, format_json) = \
        parse_cmdargs()

    # create file object to output data
    outdatafileobj = open(out_fname, 'w')

    # create DF from CSSE data, add new column in order to 'join' with population data
    csse_data_daily_report_df = pd.read_csv(covid_csse_data_filepath, skipinitialspace=True,
                                            dtype={'FIPS':'object'})
    csse_data_daily_report_df['Combined_Key_US'] = csse_data_daily_report_df.Admin2 + "," \
        + csse_data_daily_report_df.Province_State

    # create DF from population data, add 'Combined_Key_US' column
    population_data_df = pd.read_csv(pop_data_filepath, skipinitialspace=True)
    population_data_df['Combined_Key_US'] = population_data_df.County + "," + population_data_df.State

    # now, join
    csse_data_daily_report_merged_df = pd.merge(csse_data_daily_report_df,
                                                population_data_df[['Combined_Key_US', 'Population']],
                                                on='Combined_Key_US', how='left')

    # TODO merged data has population column converted to float type, convert it to int

    # TODO output merge errors to log file

    # drop unnecessary columns
    csse_data_daily_report_merged_df.drop(columns=['Last_Update', 'Combined_Key_US'])

    # for plain text output, write out (comma-seperated) disease model to outfile
    if not format_json:
        strout = ','.join(str(v) for v in disease_model + [data_date])
        outdatafileobj.write(strout + "\n")

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
        "locations": []
    }

    if format_json:
        # write out main dictionary to json file
        final_dict["locations"] = csse_data_daily_report_merged_df.values.tolist()
        # TODO pretty print json??
        outdatafileobj.write(json.dumps(final_dict))
    else:
        outdatafileobj.write(csse_data_daily_report_merged_df.to_csv(header=False, index=False))


# MAIN
if __name__ == '__main__':
    prepare_data()

