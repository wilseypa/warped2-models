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
    import fcntl
    import errno
    import uuid
    import prepare_dataset

    from scipy.stats import wasserstein_distance
    from scipy.spatial import distance

except Exception as e:
    print(str(type(e).__name__) + ": " + str(e), file=sys.stderr)
    sys.exit(1)

dictDistMetricNames = {'wass': 'WASSERSTEIN', 'jshan': 'JENSENSHANNON'}


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
                                                        "Wasserstein distance, 'jshan' for Jenson-shannon",
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


def get_disease_metrics_list(simJsonFile):
    """
    """
    cmd = subprocess.run(['./extractDiseaseMetricCols', '-f', simJsonFile, '-i', '6,7,8,9'],
                         stdout=subprocess.PIPE, text=True)

    valStr = cmd.stdout

    listDistanceMetricsVal = []

    for metricCol in valStr.split('\n'):
        listDistanceMetricsVal.append(list(map(int, metricCol.split(','))))

    return listDistanceMetricsVal


def get_simulation_actual_distance_metrics(listDistanceMetrics, simJsonFile1, simJsonFile2):
    """
    """
    listDiseaseMetrics1 = get_disease_metrics_list(simJsonFile1)
    listDiseaseMetrics2 = get_disease_metrics_list(simJsonFile2)

    distanceMetricsResult = {}

    for metric in listDistanceMetrics:

        distVal = 0

        for i in range(len(listDiseaseMetrics1)):
            numericList1 = listDiseaseMetrics1[i]
            numericList2 = listDiseaseMetrics2[i]

            if metric == 'wass':
                distVal += wasserstein_distance(numericList1[0:10], numericList2[1:10])

            if metric == 'jshan':
                distVal += distance.jensenshannon(numericList1, numericList2)

        distVal /= len(listDiseaseMetrics1)

        distanceMetricsResult[dictDistMetricNames[metric]] = round(distVal, 3)

    return distanceMetricsResult


def run_simulation(simInputJsonFile, simRuntimeUnits, simOutJsonFile):
    """
    """
    cmd = subprocess.run(['../pandemic_sim', '-m', simInputJsonFile, '--max-sim-time',
                          str(simRuntimeUnits), '-o', simOutJsonFile])

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


def get_sim_end_date(simStartDate, simRuntimeDays):
    """
    return end_date in MM-DD-YYYY format
    """
    simEndDate = datetime.datetime.strptime(simStartDate, "%m-%d-%Y") \
                 + datetime.timedelta(days=simRuntimeDays)

    simEndDate = simEndDate.strftime("%m-%d-%Y")

    return simEndDate


def calc_distance_ratio(enum_distanceMetric, simulated_json_file, sim_end_date, sim_start_date):
    """
    not used
    TODO

    ratio = distance between simulated_json_file & formatted_file(sim_start_date) / distance between formatted_file(sim_end_date) & formatted_file(sim_start_date)

    return ratio

    """
    pass


def flatten_dict(y):
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


def get_json_from_stream(paramTweaksFileobj, startPos):
    """
    """
    try:
        dictParamTweaks = json.load(paramTweaksFileobj)
        return (dictParamTweaks, -1)  # -1 signals end of file

    except json.JSONDecodeError as e:
        paramTweaksFileobj.seek(startPos)
        strJson = paramTweaksFileobj.read(e.pos)
        dictParamTweaks = json.loads(strJson)
        startPos += e.pos
        return (dictParamTweaks, startPos)


def getDictTweakableDiseaseParamsFromFile(jsonFile):
    """
    """
    with open(jsonFile, 'r') as f:
        jsonData = json.load(f)
        return getDictTweakableDiseaseParams(jsonData)


def getDictTweakableDiseaseParams(dictJson):
    dictParams = {k: dictJson[k] for k in dictJson["tweakable_params"]}
    return dictParams


def tweak_params_sim_inputfile(dictParamTweaks, baseSimInputJsonFile, tweakedSimInputJsonFile):
    """
    """
    dictFlattened = flatten_dict(dictParamTweaks)

    baseSimInputJsonFileobj = open(baseSimInputJsonFile, 'r')
    jsonData = json.load(baseSimInputJsonFileobj)
    baseSimInputJsonFileobj.close()

    for element in dictFlattened:
        listKeys = element[0]
        newVal = element[1]

        jsondata2 = jsonData
        for key in listKeys[:-1]:
            jsondata2 = jsondata2[key]

        lastKey = listKeys[-1]
        jsondata2[lastKey] = newVal

    with open(tweakedSimInputJsonFile, "w") as jsonFile:
        json.dump(jsonData, jsonFile)

    return getDictTweakableDiseaseParams(jsonData)


def abbreviate_dict_terms(dict_var):
    """
    """
    abbr_dict = {
        "diseaseParam": "dp",
        "disease_model": "dm",
        "transmissibility": "tm",
        "mean_incubation_duration_in_days": "mi",
        "mean_infection_duration_in_days": "mn",
        "mortality_ratio": "mr",
        "update_trig_interval_in_hrs": "ut",
        "diffusion_model": "df",
        "graph_type": "gt",
        "graph_params": "gp",
        "diffusion_trig_interval_in_hrs": "di",
        "distanceMetricValues": "dv",
    }

    def helper_func(dictToAbbreviate):
        delete_keys_list = []

        for k in list(dictToAbbreviate.keys()):

            if type(dictToAbbreviate[k]) is dict:
                helper_func(dictToAbbreviate[k])

            if k in abbr_dict:
                dictToAbbreviate[abbr_dict[k]] = dictToAbbreviate[k]
                del dictToAbbreviate[k]

    helper_func(dict_var)


def flatten_dict_get_keys(dictVar):
    """
    """
    dictFlattened = flatten_dict(dictVar)
    listKeys = []

    for item in dictFlattened:
        keyStr = '-'.join(item[0])
        listKeys.append(keyStr)

    return listKeys


def flatten_dict_get_values(dictVar):
    flattenedDict = flatten_dict(dictVar)
    listValues = []

    for item in flattenedDict:
        listValues.append(item[1])

    return listValues


def get_csv_header(dictDiseaseParams, listDiseaseMetricKeywords):
    """
    """
    tweakableDiseaseParamKeys = flatten_dict_get_keys(dictDiseaseParams)
    header_csv_string = ','.join(["start_date", "end_date"] + tweakableDiseaseParamKeys \
                                 + [dictDistMetricNames[k] for k in listDiseaseMetricKeywords])

    header_csv_string += '\n'

    return header_csv_string
    # fileObj.write(header_csv_string + '\n')


def get_line_sim_params_result(startDate, endDate, dictParamTweaks, listDistMetricNames,
                               dictDistMetrics):
    """
    in tsv format
    """
    temp = flatten_dict_get_values(dictParamTweaks)

    # HACK brute-forced replacement of comma with semi-colon (for graph_params)
    # TODO any alternative elegant approach ??
    csvLine = ','.join(
        [startDate, endDate] + [str(x).replace(',', ';') for x in flatten_dict_get_values(dictParamTweaks)] \
        + [str(dictDistMetrics[k]) for k in listDistMetricNames])

    csvLine += '\n'

    return csvLine


def trigger():
    try:

        (simStartDate, simRuntimeDays, distMetricsToUse, paramTweaksFile, simResultFile,
         jhuCSSErepoPath, usCountiesListFile) = parse_cmdargs()

        setup_logging()

        simRuntimeDays = int(simRuntimeDays)
        simEndDate = get_sim_end_date(simStartDate, simRuntimeDays)

        simRuntimeTimeUnits = simRuntimeDays * 24

        simStartdateInputJsonFile = prepare_dataset.prepare_data(jhu_csse_path=jhuCSSErepoPath,
                                                                 covid_csse_data_date=simStartDate,
                                                                 pop_data_filepath=usCountiesListFile)

        simEnddateInputFormattedJsonFile = prepare_dataset.prepare_data(jhu_csse_path=jhuCSSErepoPath,
                                                                        covid_csse_data_date=simEndDate,
                                                                        pop_data_filepath=usCountiesListFile)

        paramTweaksFileobj = open(paramTweaksFile, "r")

        #

        simResultStr = ""
        simResultFileHeader = get_csv_header(getDictTweakableDiseaseParamsFromFile(simStartdateInputJsonFile),
                                             distMetricsToUse)

        paramTweaksFilePos = 0
        print(">>>>h1")

        # TODO change -1
        while paramTweaksFilePos != -1:
            print(">>>>h2")
            (dictParamTweaks, paramTweaksFilePos) = get_json_from_stream(paramTweaksFileobj,
                                                                         paramTweaksFilePos)

            simStartDateInputJsonTweakedFile = os.getcwd() + "/../data/tempfile_jsontweakedfile_" + str(uuid.uuid4())

            # TODO add comment
            dictCompleteParamsTweaked = tweak_params_sim_inputfile(dictParamTweaks, simStartdateInputJsonFile,
                                                                   simStartDateInputJsonTweakedFile)

            simOutJsonFile = os.getcwd() + "/../data/" + "tempfile_sim_outjson_" + str(uuid.uuid4())

            run_simulation(simStartDateInputJsonTweakedFile, simRuntimeTimeUnits, simOutJsonFile)

            distMetricsResult = get_simulation_actual_distance_metrics(distMetricsToUse,
                                                                       simEnddateInputFormattedJsonFile,
                                                                       simOutJsonFile)

            simResultStr += get_line_sim_params_result(simStartDate, simEndDate, dictCompleteParamsTweaked,
                                                       [dictDistMetricNames[k] for k in distMetricsToUse],
                                                       distMetricsResult)

            os.remove(simStartDateInputJsonTweakedFile)
            os.remove(simOutJsonFile)

        simResultFileobj = open(simResultFile, "a+")

        while True:
            try:
                print(">>>>h3")
                fcntl.flock(simResultFileobj, fcntl.LOCK_EX | fcntl.LOCK_NB)
                print(">>>>h4")
                break

            except IOError as e:
                if e.errno != errno.EAGAIN:
                    raise
                else:
                    logging.info("file [{}] is locked, sleeping for 2 sec".format(simResultFile))
                    time.sleep(2)

        logging.info("locked file [{}]".format(simResultFile))

        if os.path.getsize(simResultFile) == 0:
            simResultStr = simResultFileHeader + simResultStr

        simResultFileobj.write(simResultStr)

        # unlock
        fcntl.flock(simResultFileobj, fcntl.LOCK_UN)
        print("unlocked file [{}]".format(simResultFile))

        add_end_log()

    except Exception as e:
        logging.exception(str(type(e).__name__) + ": " + str(e), exc_info=True)
        logging.info("Exiting ...\n\n")

        print("Exception occurred, view log file")

        sys.exit(1)


# MAIN
if __name__ == '__main__':
    trigger()
