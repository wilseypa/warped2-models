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
    from scipy.spatial import distance

except Exception as e:
    print(str(type(e).__name__) + ": " + str(e), file=sys.stderr)
    sys.exit(1)


dictDistMetricNames = {'wass':'WASSERSTEIN','jshan':'JENSENSHANNON'}



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

    parser.add_argument('--use_metric', nargs='*', help="List of distance metrics to use. 'wass' for "
                        "Wasserstein distance",
                        default=['wass'],
                        required=False)

    parser.add_argument('--tweaked_params_file', help='filepath containing tweaked diseaseparams',
                        required=True)

    parser.add_argument('--sim_result_file', help='filepath to save consolidated result for all simulations',
                        required=True)

    parser.add_argument('--jhu_repo_path', help='path of JHU CSSE repo',
                        default='../data/COVID-19.jhu/',
                        required=False)

    parser.add_argument('--population_data_file', help='filepath for Population data csv',
                        default='../data/US_counties_population_latLong.csv',
                        required=False)

    args = parser.parse_args()

    return (args.sim_start_date, args.sim_runtime_days, args.use_metric, args.tweaked_params_file,
            args.sim_result_file, args.jhu_repo_path, args.population_data_file)



def getDiseaseMetricLists(jsonFile):
    """
    """
    cmd = subprocess.run(['./extractDiseaseMetricCols', '-f', jsonFile, '-i', '6,7,8,9'],
                         stdout=subprocess.PIPE, text=True)

    valStr = cmd.stdout;

    metricValList = []

    for metricCol in valStr.split('\n'):
        metricValList.append(list(map(int, metricCol.split(','))));

    return metricValList



def calcDistanceMetric(listDistanceMetrics, file1, file2):
    """
    """
    metricValList1 = getDiseaseMetricLists(file1)
    metricValList2 = getDiseaseMetricLists(file2)

    for i in range(len(metricValList1)):
        numericList1 = metricValList1[i];
        numericList2 = metricValList2[i];

    distanceResult = {}

    for metric in listDistanceMetrics:

        distVal = 0

        for i in range(len(metricValList1)):
            numericList1 = metricValList1[i]
            numericList2 = metricValList2[i]

            if metric == 'wass':
                distVal += wasserstein_distance(numericList1[0:10], numericList2[1:10])

            if metric == 'jshan':
                distVal += distance.jensenshannon(numericList1, numericList2)

        distVal /= len(metricValList1)

        distanceResult[dictDistMetricNames[metric]] = distVal

    return distanceResult



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
    not used
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



def getDictTweakableDiseaseParamsFromFile(jsonFile):
    """
    """
    with open(jsonFile, 'r') as f:
        jsonData = json.load(f)
        return getDictTweakableDiseaseParams(jsonData)



def getDictTweakableDiseaseParams(dictJson):

    dictParams = {k: dictJson[k] for k in dictJson["tweakable_params"]}
    return dictParams



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

    with open(tweakedjson_filepath, "w") as jsonFile:
        json.dump(jsondata, jsonFile)

    return getDictTweakableDiseaseParams(jsondata)


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


def getListofKeys(dictvar):
    """
    """
    flattened_val = flatten_json(dictvar)
    listKeys = []

    for item in flattened_val:
        keyStr = '-'.join(item[0])
        listKeys.append(keyStr)

    return listKeys



def getListofValues(dictvar):
    flattened_val = flatten_json(dictvar)
    listValues = []

    for item in flattened_val:
        listValues.append(item[1])

    return listValues



def addTSVHeader(fileObj, dictDiseaseParams, listDiseaseMetricKeywords):
    """
    """
    tweakableDiseaseParamKeys = getListofKeys(dictDiseaseParams)
    header_csv_string = '\t'.join(["start_date", "end_date"] + tweakableDiseaseParamKeys \
                                  + [dictDistMetricNames[k] for k in listDiseaseMetricKeywords])

    fileObj.write(header_csv_string + '\n')



def saveSimulationOverview(sim_overview_fileobj, start_date, end_date, tweakedparam_json, listDistMetricNames,
                           dictDistmetrics):
    """
    in tsv format
    """
    tsvLine = '\t'.join([start_date, end_date] + [str(x) for x in getListofValues(tweakedparam_json)] \
                       + [str(dictDistmetrics[k]) for k in listDistMetricNames])

    sim_overview_fileobj.write(tsvLine + '\n')



def trigger():

    try:

        (sim_start_date, sim_runtime_days, use_distMetrics, tweaked_params_file, sim_result_file,
         jhu_repo_path, population_data_file) = parse_cmdargs()

        setup_logging()

        sim_runtime_days = int(sim_runtime_days)
        sim_end_date = get_sim_end_date(sim_start_date, sim_runtime_days)

        sim_runtime_units = sim_runtime_days * 24

        formattedjson_startdate_filepath = prepare_dataset.prepare_data(jhu_csse_path=jhu_repo_path,
                                                                        covid_csse_data_date=sim_start_date,
                                                                        pop_data_filepath=population_data_file)

        formattedjson_enddate_filepath = prepare_dataset.prepare_data(jhu_csse_path=jhu_repo_path,
                                                                      covid_csse_data_date=sim_end_date,
                                                                      pop_data_filepath=population_data_file)

        tweaked_params_fileobj = open(tweaked_params_file, "r")

        sim_result_fileobj = open(sim_result_file, "a+")

        if os.path.getsize(sim_result_file) == 0:
            addTSVHeader(sim_result_fileobj, getDictTweakableDiseaseParamsFromFile(formattedjson_startdate_filepath),
                         use_distMetrics)

        tweaked_params_start_pos = 0

        # TODO change -1
        while tweaked_params_start_pos != -1:

            (tweakedparam_json, tweaked_params_start_pos) = get_json_from_stream(tweaked_params_fileobj,
                                                                                 tweaked_params_start_pos)

            tweakedjson_startdate_filepath = os.getcwd() + "/../data/temp_jsontweakedfile1"

            # TODO add comment
            finalParams = tweakDiseaseParams(tweakedparam_json, formattedjson_startdate_filepath,
                                             tweakedjson_startdate_filepath)

            sim_outjson_filepath = os.getcwd() + "/../data/" + "sim_outjson_temp.json"

            run_simulation(tweakedjson_startdate_filepath, sim_runtime_units, sim_outjson_filepath)

            distMetricResult = calcDistanceMetric(use_distMetrics, formattedjson_enddate_filepath,
                                                  sim_outjson_filepath)

            saveSimulationOverview(sim_result_fileobj, sim_start_date, sim_end_date, finalParams,
                                   [dictDistMetricNames[k] for k in use_distMetrics],
                                   distMetricResult)

            os.remove(tweakedjson_startdate_filepath)
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

