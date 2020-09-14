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
    import datetime
    import subprocess

    import prepare_dataset

    from scipy.stats import wasserstein_distance

    from enum import Enum, auto

except Exception as e:
    print(str(type(e).__name__) + ": " + str(e), file=sys.stderr)
    sys.exit(1)


class DistanceMetric(Enum):
    WASSERSTEIN = auto()


def setup_logging():
    """
    """
    # TODO configure logger to print module name in same log file.
    try:

        logging.basicConfig(format="%(asctime)s [%(levelname)s] : %(message)s",
                            filename='pandemic-sim-driver.log', level=logging.DEBUG)

    except Exception as e:
        print(str(type(e).__name__) + ": " + str(e), file=sys.stderr)
        sys.exit(1)


def parse_cmdargs():
    """
    Using argparse module, get commandline arguments
    """
    parser = argparse.ArgumentParser(description='Prepare Covid19 dataset, invoke simulation, '
                                     'distance function between expected & simulated output, ')

    parser.add_argument('--sim_start_date', help='Simulation start date',
                        required=True)

    parser.add_argument('--sim_runtime_days', help='Simulation runtime in days',
                        required=True)

    parser.add_argument('--graph_type', help="'ws' for Watts-Strogatz, and 'ba' for Barabasi-Albert",
                        default='ws',
                        choices=['ws', 'ba'],
                        required=False)

    parser.add_argument('--use_metric', nargs='*', help='Distance metric to use',
                        default=['WASSERSTEIN'],
                        required=False)

    parser.add_argument('--paramtweaks_filepath', help='filepath containing tweaked diseaseparams',
                        required=True)

    parser.add_argument('--sim_overview_filepath', help='filepath to save overview for all simulations',
                        required=True)


    args = parser.parse_args()

    return (args.sim_start_date, args.sim_runtime_days, args.graph_type, args.use_metric, args.paramtweaks_filepath,
            args.sim_overview_filepath)


def convertFiletoNumericList(filepath):
    """
    """
    fileobj = open(filepath, 'r')
    filestr = fileobj.read()
    fileobj.close()

    filestr_ascii_list = [ord(c) for c in filestr]

    return filestr_ascii_list


def calcDistanceMetric(listDistanceMetric, file1, file2):
    """
    - return list of (tuple of distancemetric enum and distance value)
    """

    numericList1 = convertFiletoNumericList(file1)
    numericList2 = convertFiletoNumericList(file2)

    distanceResult = []

    for metric in listDistanceMetric:
        if metric == DistanceMetric.WASSERSTEIN.name:
            metricVal = calc_wasserstein_distance(numericList1, numericList2)
            distanceResult.append({DistanceMetric.WASSERSTEIN.name : metricVal})

    return distanceResult


def calc_wasserstein_distance(list1, list2):
    """
    """
    return wasserstein_distance(list1, list2)


def run_simulation(formatted_input_json_filepath, sim_runtime_units, sim_output_filepath):
    """
    """
    cmd = subprocess.run(['../pandemic_sim', '-m', formatted_input_json_filepath, '--max-sim-time',
                          str(sim_runtime_units), '-o', sim_output_filepath])

    if cmd.returncode != 0:
        raise Exception("Simulation Error!")


def add_start_log():
    logging.info("")
    logging.info("")
    logging.info("Starting ...")


def add_end_log():
    logging.info("Exiting ...")
    logging.info("")
    logging.info("")


def get_sim_end_date(sim_start_date, sim_runtime_days):
    """
    return end_date in MM-DD-YYYY format
    """
    sim_end_date = datetime.datetime.strptime(sim_start_date, "%m-%d-%Y") \
        + datetime.timedelta(days=sim_runtime_days)

    sim_end_date = sim_end_date.strftime("%m-%d-%Y")

    return sim_end_date


def calc_distance_ratio(enum_distanceMetric, simulated_json_file, sim_end_date, sim_start_date):
    """
    TODO

    ratio = distance between simulated_json_file & formatted_file(sim_start_date) / distance between formatted_file(sim_end_date) & formatted_file(sim_start_date)

    return ratio

    """
    pass


def flatten_json(y):
    """
    """
    out = []

    def flatten(x, name=[]):
        if type(x) is dict:
            for key in x:
                name.append(key)
                flatten(x[key], name=name.copy())
                name.pop()
        else:
            out.append((name, x))

    flatten(y)
    return out


def get_json_from_stream(fileobj, start_pos):
    """
    """
    try:
        obj = json.load(fileobj)
        return (obj, -1) # -1 signals end of file

    except json.JSONDecodeError as e:
        fileobj.seek(start_pos)
        json_str = fileobj.read(e.pos)
        obj = json.loads(json_str)
        start_pos += e.pos
        return (obj, start_pos)


def tweakDiseaseParams(json_obj, basejson_filepath, tweakedjson_filepath):
    """
    """
    flattened_json = flatten_json(json_obj)

    basejson_fileobj = open(basejson_filepath, 'r')
    jsondata = json.load(basejson_fileobj)
    basejson_fileobj.close()

    for element in flattened_json:
        list_keys = element[0]
        new_val = element[1]

        jsondata2 = jsondata
        for key in list_keys[:-1]:
            jsondata2 = jsondata2[key]

        last_key = list_keys[-1]
        jsondata2[last_key] = new_val

    updatedParams = {k: jsondata[k] for k in jsondata["tweakable_params"]}

    with open(tweakedjson_filepath, "w") as jsonFile:
        json.dump(jsondata, jsonFile)

    return updatedParams


def abbreviateTermsInDict(dict_var):
    """
    """
    abbr_dict = {
        "diseaseParam":"dp",
        "disease_model":"dm",
        "transmissibility":"tm",
        "mean_incubation_duration_in_days":"mi",
        "mean_infection_duration_in_days":"mn",
        "mortality_ratio":"mr",
        "update_trig_interval_in_hrs":"ut",
        "diffusion_model":"df",
        "graph_type":"gt",
        "graph_params":"gp",
        "diffusion_trig_interval_in_hrs":"di",
        "distanceMetricValues":"dv",
    }


    def recursiveAbbrDict(abbrThisDict):
        delete_keys_list = []

        for k in list(abbrThisDict.keys()):

            if type(abbrThisDict[k]) is dict:
                recursiveAbbrDict(abbrThisDict[k])

            if k in abbr_dict:
                abbrThisDict[abbr_dict[k]] = abbrThisDict[k]
                del abbrThisDict[k]


    recursiveAbbrDict(dict_var)



def saveSimulationOverview(sim_overview_fileobj, start_date, end_date, tweakedparam_json, list_distmetrics):
    """
    """
    abbreviateTermsInDict(tweakedparam_json)

    json_dict = {
        'start_date':start_date,
        'end_date':end_date,
        'diseaseParams':tweakedparam_json,
        'distanceMetricValues':list_distmetrics
    }

    json.dump(json_dict, sim_overview_fileobj)
    sim_overview_fileobj.write('\n')


def trigger():

    try:

        (sim_start_date, sim_runtime_days, graph_type, distMetrics_list, diseaseparam_tweak_filepath,
         sim_overview_filepath) = parse_cmdargs()

        setup_logging()

        sim_runtime_days = int(sim_runtime_days)
        sim_end_date = get_sim_end_date(sim_start_date, sim_runtime_days)

        sim_runtime_units = sim_runtime_days * 24

        formattedjson_startdate_filepath = prepare_dataset.prepare_data(covid_csse_data_date=sim_start_date,
                                                                        graph_type=graph_type)



        formattedjson_enddate_filepath = prepare_dataset.prepare_data(covid_csse_data_date=sim_end_date,
                                                                      graph_type=graph_type)


        param_tweak_fileobj = open(diseaseparam_tweak_filepath, "r")
        sim_overview_fileobj = open(sim_overview_filepath, "a+")



        param_tweak_start_pos = 0
        while param_tweak_start_pos != -1:

            (tweakedparam_json, param_tweak_start_pos) = get_json_from_stream(param_tweak_fileobj,
                                                                              param_tweak_start_pos)



            tweakedjson_startdate_filepath = os.getcwd() + "/../data/temp_jsontweakedfile1"
            tweakedjson_enddate_filepath = os.getcwd() + "/../data/temp_jsontweakedfile2"

            finalParams = tweakDiseaseParams(tweakedparam_json, formattedjson_startdate_filepath,
                                             tweakedjson_startdate_filepath)
            tweakDiseaseParams(tweakedparam_json, formattedjson_enddate_filepath, tweakedjson_enddate_filepath)



            sim_outjson_filepath = os.getcwd() + "/../data/" + "sim_outjson_temp.json"

            run_simulation(tweakedjson_startdate_filepath, sim_runtime_units, sim_outjson_filepath)

            distMetricValList = calcDistanceMetric(distMetrics_list, tweakedjson_enddate_filepath,
                                                   sim_outjson_filepath)

            saveSimulationOverview(sim_overview_fileobj, sim_start_date, sim_end_date, finalParams,
                                   distMetricValList)

            # delete unnecessary files
            os.remove(tweakedjson_startdate_filepath)
            os.remove(tweakedjson_enddate_filepath)
            os.remove(sim_outjson_filepath)


        add_end_log()

    except Exception as e:
        logging.exception(str(type(e).__name__) + ": " + str(e), exc_info=True)
        logging.info("Exiting ...\n\n")

        print("Exception occured, view log file")

        sys.exit(1)


# MAIN
if __name__ == '__main__':
    trigger()

