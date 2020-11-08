#!/usr/bin/env python3

try:
    import sys
    import os
    import time
    import re
    import argparse
    import json
    import pandas as pd
    # import logging
    # import logging.handlers
    import numpy as np
    from math import radians, cos, sin, asin, sqrt, isnan
    import datetime
    import subprocess
    import fcntl
    import errno
    import uuid
    import pathlib
    import prepare_dataset

    from scipy.spatial import distance
    from scipy.stats import wasserstein_distance
    import logging
    import logging.config
    # from logging import getLogger, INFO, DEBUG
    from concurrent_log_handler import ConcurrentRotatingFileHandler
    # import concurrent_log_handler

except Exception as e:
    print(str(type(e).__name__) + ": " + str(e), file=sys.stderr)
    sys.exit(1)

SCRIPTS_LOGS_DIR_PATH = "./logs/"
dictDistMetricNames = {'wass': 'WASSERSTEIN', 'jenshan': 'JENSENSHANNON', 'eucd': 'EUCLIDEAN-DISTANCE'}
logger = None
COUNT_SUCCESSFULL_SIM_LOG_LIMIT = 10
COUNT_SUCCESSFULL_SIM_WRITERESULT_LIMIT = 100


def setup_logging():
    """
    """
    # TODO configure logger to print module name in same log file.
    try:
        global logger

        logger = logging.getLogger("pandemic_sim_driver")
        rotateHandler = ConcurrentRotatingFileHandler(
            os.path.abspath(SCRIPTS_LOGS_DIR_PATH + "pandemic_backend_scripts.log"),
            "a", 1024 * 1024 * 4, 100)

        logFormatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
        rotateHandler.setFormatter(logFormatter)

        logger.addHandler(rotateHandler)
        logger.setLevel(logging.DEBUG)

        # logger.info("concurrent logging")

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

    # parser.add_argument('--exposed_confirmed_ratio', help='YYG or custom value',
    #                     default='2.5',
    #                     required=False)

    parser.add_argument('--use_metric', nargs='*', help="List of distance metrics to use. 'wass' for "
                                                        "Wasserstein distance, 'jshan' for Jenson-shannon"
                                                        "'eucd' for Euclidean distance",
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

    return (args.sim_start_date, args.sim_runtime_days, args.use_metric,
            args.tweaked_params_file,
            args.sim_result_file, args.jhu_repo_path,
            args.population_data_file)


def extract_cols_list(simJsonFile):
    """
    """
    cmd = subprocess.run(['./extractDiseaseMetricCols', '-f', simJsonFile, '-i', '6,7,8,9,10'],
                         stdout=subprocess.PIPE, text=True)

    valStr = cmd.stdout

    listDistanceMetricsVal = []

    for metricCol in valStr.split('\n'):
        listDistanceMetricsVal.append(list(map(int, metricCol.split(','))))

    return listDistanceMetricsVal


def get_simulation_actual_distance_metrics(listDistanceMetrics, simJsonFileBase, simJsonFileSimulated):
    """
    """
    tempList = extract_cols_list(simJsonFileBase)
    listDiseaseMetricsBase = tempList[0:4]  # exclude population column

    # print("simJsonFileSimulated", simJsonFileSimulated)
    tempList = extract_cols_list(simJsonFileSimulated)
    listDiseaseMetricsSimulated = tempList[0:4]

    listCountyPopulation = tempList[4]
    totalPopulation = sum(listCountyPopulation)

    distanceMetricsResult = {}

    # print("listdistmetrics", listDistanceMetrics)

    for metric in listDistanceMetrics:
        # print("metric", metric)
        distVal = 0.0

        for i in range(len(listDiseaseMetricsBase)):

            # print("\n\n\ni:", i)
            numericListBase = listDiseaseMetricsBase[i]
            numericListSimulated = listDiseaseMetricsSimulated[i]

            # print("numericList2", numericListSimulated[1050:1055])

            # to find offending county(s)
            # print("1", [float(x) / float(y) for x, y in zip(numericListBase, listCountyPopulation)][1052:1053])
            # print("2", [float(x) / float(y) for x, y in zip(numericListSimulated, listCountyPopulation)][1052:1053])
            #
            # print("2", [(float(x), float(y)) for x, y in zip(numericListSimulated, listCountyPopulation)][1052:1053])
            # sys.exit(1)

            if metric == 'wass':
                # print("wass metric")
                distVal += wasserstein_distance(numericListBase, numericListSimulated)

            if metric == 'jenshan':
                # templist1 = [0.00010482588959891951, 8.259212587266646e-06]
                # templist2 = [0.0009865204440965508, 8.036204707743254e-05]
                #
                # print("!!!!!", distance.jensenshannon(templist1, templist2))

                # templist1 = np.array([float(x) / totalPopulation for x in numericListBase])
                # templist2 = np.array([float(x) / totalPopulation for x in numericListSimulated])

                templist1 = [float(x) / totalPopulation for x in numericListBase]
                templist2 = [float(x) / totalPopulation for x in numericListSimulated]

                jenshanval = distance.jensenshannon(templist1, templist2)
                # print("jenshan metric", jenshanval)

                if not isnan(jenshanval):
                    distVal += jenshanval

                # print("distval", distVal)
                # print(templist1)
                # print(templist2)

            if metric == 'eucd':
                distVal += distance.euclidean(numericListBase, numericListSimulated)

        distVal /= len(listDiseaseMetricsBase)

        distanceMetricsResult[dictDistMetricNames[metric]] = round(distVal, 3)

    return distanceMetricsResult


def run_simulation(simInputJsonFile, simRuntimeUnits, simOutJsonFile):
    """
    """
    try:
        cmd = subprocess.run(['../pandemic_sim', '-m', simInputJsonFile, '--max-sim-time',
                              str(simRuntimeUnits), '-o', simOutJsonFile], stdout=subprocess.DEVNULL,
                             stderr=subprocess.PIPE,
                             text=True,
                             timeout=30)
    except subprocess.TimeoutExpired:
        errString = "Timeout expired for simulation job"
        return -1, errString

    errString = cmd.stderr

    if cmd.returncode != 0:
        # print("Simulation Error!")
        return -1, errString

    return 0, errString


def add_start_log():
    logger.info("")
    logger.info("")
    logger.info("Starting ...")


def add_end_log():
    logger.info("Exiting ...")
    logger.info("")
    logger.info("")


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
    TODO add comment
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
    """

    :return:
    """
    try:
        def writeToFile():
            # lock file and write
            nonlocal simResultFileobj
            nonlocal simResultStr

            while True:
                try:
                    fcntl.flock(simResultFileobj, fcntl.LOCK_EX | fcntl.LOCK_NB)
                    break

                except IOError as e:
                    if e.errno != errno.EAGAIN:
                        raise
                    else:
                        logger.info("file [{}] is locked, sleeping for 2 sec".format(simResultFile))
                        time.sleep(2)

            # logger.info("locked file [{}]".format(simResultFile))

            if os.path.getsize(simResultFile) == 0:
                simResultStr = simResultFileHeader + simResultStr

            simResultFileobj.write(simResultStr)

            # unlock
            fcntl.flock(simResultFileobj, fcntl.LOCK_UN)
            # logger.info("Wrote to & unlocked file [{}]".format(simResultFile))

            # print("unlocked file [{}]".format(simResultFile))

        # END helper function

        (simStartDate, simRuntimeDays, distMetricsToUse, paramTweaksFile,
         simResultFile,
         jhuCSSErepoPath,
         usCountiesListFile) = parse_cmdargs()

        # NOTE: infected_confirmed_ratio, distMetricsToUse remain constant for entire set of simulations,
        # so, they are not included in the "tweakfile"

        setup_logging()

        simRuntimeDays = int(simRuntimeDays)
        simEndDate = get_sim_end_date(simStartDate, simRuntimeDays)

        simRuntimeTimeUnits = simRuntimeDays * 24

        #
        # if exposedConfirmedRatio == "YYG":
        #     # read from csv file
        #     yyg_df = pandas.read_csv("cumulative-estimated-infections-of-covid-19.dateFormatted.csv")
        #
        #     if simStartDate in yyg_df.Date.values:
        #         confirmedCount = yyg_df.loc[yyg_df['Date'] == simStartDate]["Cumulative_confirmed_cases"].iloc[0]
        #         estimatedInfectedCount = yyg_df.loc[yyg_df['Date'] == \
        #                                             simStartDate]["Cumulative_estimated_infections_YYG"].iloc[0]
        #
        #         if confirmedCount == 0 or estimatedInfectedCount == 0:
        #             exposedConfirmedRatio = 2.5
        #         else:
        #             exposedConfirmedRatio = round(estimatedInfectedCount / confirmedCount, 4)
        #
        #     else:
        #         # if date not found in csv
        #         exposedConfirmedRatio = 2.5
        #
        # else:
        #     # if custom value, convert string to float
        #     exposedConfirmedRatio = float(exposedConfirmedRatio)
        
        simStartdateInputJsonFile = prepare_dataset.prepare_data(jhu_csse_path=jhuCSSErepoPath,
                                                                 covid_csse_data_date=simStartDate,
                                                                 pop_data_filepath=usCountiesListFile)

        simEnddateInputFormattedJsonFile = prepare_dataset.prepare_data(jhu_csse_path=jhuCSSErepoPath,
                                                                        covid_csse_data_date=simEndDate,
                                                                        pop_data_filepath=usCountiesListFile)

        paramTweaksFileobj = open(paramTweaksFile, "r")

        simResultStr = ""
        simResultFileHeader = get_csv_header(getDictTweakableDiseaseParamsFromFile(simStartdateInputJsonFile),
                                             distMetricsToUse)

        paramTweaksFilePos = 0
        # print(">>>>h1")

        simResultFileobj = open(simResultFile, "a")

        # TODO change -1
        successfulSimulationWriteCounter = 0
        countSuccessfulSimulation = 0
        countFailedSimulation = 0
        pidVal = os.getpid()
        while paramTweaksFilePos != -1:  # TODO why not simply read each line ???
            (dictParamTweaks, paramTweaksFilePos) = get_json_from_stream(paramTweaksFileobj,
                                                                         paramTweaksFilePos)

            simStartDateInputJsonTweakedFile = os.getcwd() + "/../data/tempfile_jsontweakedfile_" + str(uuid.uuid4())

            # TODO add comment
            dictCompleteParamsTweaked = tweak_params_sim_inputfile(dictParamTweaks, simStartdateInputJsonFile,
                                                                   simStartDateInputJsonTweakedFile)

            # print("dictCompleteParamsTweaked", dictCompleteParamsTweaked)

            simOutJsonFile = os.getcwd() + "/../data/" + "tempfile_sim_outjson_" + str(uuid.uuid4())

            retStatus, errStr = run_simulation(simStartDateInputJsonTweakedFile, simRuntimeTimeUnits, simOutJsonFile)
            if retStatus == -1:
                countFailedSimulation += 1
                logger.error(
                    "pid[{}]: pandemic_sim for dates [{}:{}] failed with:\n"
                    "<STDERR_STRING>\n{}\n</STDERR_STRING>\n"
                    "<DISEASE_PARAMS>\n{}\n</DISEASE_PARAMS>\n".format(
                        simStartDate,
                        simEndDate,
                        pidVal,
                        errStr,
                        str(dictCompleteParamsTweaked)))

                try:
                    # print("trying to remove file", simStartDateInputJsonTweakedFile, simOutJsonFile)
                    os.remove(simStartDateInputJsonTweakedFile)
                    os.remove(simOutJsonFile)
                except OSError:
                    pass

                continue  # TODO add comment

            # simulation completed successfully

            countSuccessfulSimulation += 1
            successfulSimulationWriteCounter += 1

            distMetricsResult = get_simulation_actual_distance_metrics(distMetricsToUse,
                                                                       simEnddateInputFormattedJsonFile,
                                                                       simOutJsonFile)

            simResultStr += get_line_sim_params_result(simStartDate, simEndDate, dictCompleteParamsTweaked,
                                                       [dictDistMetricNames[k] for k in distMetricsToUse],
                                                       distMetricsResult)

            if countSuccessfulSimulation % COUNT_SUCCESSFULL_SIM_LOG_LIMIT == 0:
                logger.info("pid[{}]: Total {} simulations completed successfully, more may follow ...".format(
                    pidVal, countSuccessfulSimulation))

            if countSuccessfulSimulation % COUNT_SUCCESSFULL_SIM_WRITERESULT_LIMIT == 0:
                logger.info("pid[{}]: writing results for {} simulations to file, more may follow ...".format(
                    os.getpid(),
                    COUNT_SUCCESSFULL_SIM_WRITERESULT_LIMIT))  # TODO use variable

                writeToFile()
                simResultStr = ""

            try:
                os.remove(simStartDateInputJsonTweakedFile)
                os.remove(simOutJsonFile)
            except OSError:
                pass

        # if something is left unwritten, write it
        if simResultStr:
            writeToFile()
            # not needed ??
            simResultStr = ""

        simResultFileobj.close()

        logger.info("pid[{}]: Total {} simulations completed, results written to file [{}]".format(
            os.getpid(),
            countSuccessfulSimulation,
            simResultFile))

        # add_end_log()

        # TODO clear stderr first ??
        # HACK since stderr will hold existing data, use some unique string to extract count values
        print("CDRQM{}/{}PAMCU".format(countSuccessfulSimulation,
                                       countSuccessfulSimulation + countFailedSimulation),
              end='',
              file=sys.stderr)

    except Exception as e:
        logger.exception(str(type(e).__name__) + ": " + str(e), exc_info=True)
        logger.info("Exiting ...\n\n")

        print("Exception occurred, view log file")

        sys.exit(1)


# MAIN
if __name__ == '__main__':
    trigger()
