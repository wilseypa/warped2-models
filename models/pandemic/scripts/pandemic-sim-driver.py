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


class TweakedDiseaseParam(Enum):
    DISEASE_MODEL_TRANSMISSIBILITY = auto()
    DISEASE_MODEL_MEAN_INCUBATION_DURATION_IN_DAYS = auto()
    DISEASE_MODEL_MEAN_INFECTION_DURATION_IN_DAYS = auto()
    DISEASE_MODEL_MORTALITY_RATIO = auto()
    DISEASE_MODEL_UPDATE_TRIG_INTERVAL_IN_HRS = auto()
    DIFFUSION_MODEL_GRAPH_TYPE = auto()
    DIFFUSION_MODEL_GRAPH_PARAMS = auto()
    DIFFUSION_MODEL_DIFFUSION_TRIG_INTERVAL_IN_HRS = auto()

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
    :return:
    """
    parser = argparse.ArgumentParser(description='Prepare Covid19 dataset, invoke simulation, '
                                     'distance function between expected & simulated output, ')

    parser.add_argument('--start_date', help='Simulation start date',
                        required=True)
    
    parser.add_argument('--sim_runtime', help='Simulation runtime in days',
                        required=True)

    parser.add_argument('--graph_type', help="'ws' for Watts-Strogatz, and 'ba' for Barabasi-Albert",
                        choices=['ws', 'ba'],
                        required=True)
    
    parser.add_argument('--use_metric', help='Distance metric to use',
                        default='wasserstein',
                        required=False)

    # parser.add_argument('--graph_input_param_string', help="comma-seperated input parameter list for selected "
    #                     "graph type, in form of a string",
    #                     default="8,0.1",
    #                     required=False)

    args = parser.parse_args()

    return (args.start_date, args.sim_runtime, args.graph_type, args.use_metric)


# def call_prepare_dataset_module(sim_start_date, sim_end_date, graph_type):
#     """
#     """
#     (formatted_json_filepath) = prepare_dataset.prepare_data(covid_csse_data_date=sim_start_date,
#                                                             graph_type=graph_type)
    # cmd1 = subprocess.run(['./prepare_dataset.py', '--date', sim_start_date, '--graph_type', 'ws'])

    # if cmd1.returncode != 0:
    #     raise Exception("error in cmd1")

    # cmd2 = subprocess.run(['./prepare_dataset.py', '--date', sim_end_date, '--graph_type', 'ws'])

    # if cmd2.returncode != 0:
    #     raise Exception("error in cmd2")

    # logging.info("return code: {}".format(cmd1.returncode))

    # return formatted_json_filepath

def convertFiletoNumericList(filepath):
    """
    TODO 
    - read json file from "filepath"
    - extract numeric values from locations[] array and add to list
    - return list
    """
    pass

    
def calc_distance(listDistanceMetric, file1, file2):
    """
    - return list of (tuple of distancemetric enum and distance value)
    """

    list1 = convertFiletoNumericList(file1)
    list2 = convertFiletoNumericList(file2)

    distanceResult = []

    for metric in listDistanceMetric:
        if distanceMetric == DistanceMetric.WASSERSTEIN:
            result = calc_wasserstein_distance(list1, list2)
            # append tuple
            distanceResult.append(DistanceMetric.WASSERSTEIN, result)

    return distanceResult


def calc_wasserstein_distance(list1, list2):
    """
    """
    return wasserstein_distance(list1, list2)


def run_simulation(formatted_json_filepath, sim_runtime_units, sim_output_filepath):
    """
    """

    sim_output_filename = sim_end_date + ".simulated-data.json"
    sim_output_filepath = os.getcwd() + "/../data/" + sim_out_filename

    cmd = subprocess.run(['../pandemic_sim', '-m', formatted_json_filepath, '--max-sim-time', sim_runtime_units,
                           '-o', sim_output_filepath])

    if cmd.returncode != 0:
        raise Exception("simuation error")


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
    sim_end_date = datetime.datetime.strptime(sim_start_date, "%m-%d-%Y")
    + datetime.timedelta(days=sim_runtime_days)
    
    sim_end_date = sim_end_date.strftime("%d-%m-%Y")

    return sim_end_date


def append_tweaked_params_to_file(csv_filepath, list_tuple_TweakedDiseaseParam_newval):
    """
    """
    params_line = "comma-seperated existing disease_model, existing diffusion_model"

    for item_tuple in list_tuple_TweakedDiseaseParam_newval:

        if item_tuple[0] == TweakedDiseaseParam.DISEASE_MODEL_TRANSMISSIBILITY:
            # TODO replace tuple_TweakedDiseaseParam_newval[1] in params_line at appropriate position

        if item_tuple[0] == TweakedDiseaseParam.DISEASE_MODEL_MEAN_INCUBATION_DURATION_IN_DAYS:
            # TODO
        if item_tuple[0] == TweakedDiseaseParam.DISEASE_MODEL_MEAN_INFECTION_DURATION_IN_DAYS:
            # TODO
        if item_tuple[0] == TweakedDiseaseParam.DISEASE_MODEL_MORTALITY_RATIO:
            # TODO
        if item_tuple[0] == TweakedDiseaseParam.DISEASE_MODEL_UPDATE_TRIG_INTERVAL_IN_HRS:
            # TODO
        if item_tuple[0] == TweakedDiseaseParam.DIFFUSION_MODEL_GRAPH_TYPE:
            # TODO
        if item_tuple[0] == TweakedDiseaseParam.DIFFUSION_MODEL_GRAPH_PARAMS:
            # TODO
        if item_tuple[0] == TweakedDiseaseParam.DIFFUSION_MODEL_DIFFUSION_TRIG_INTERVAL_IN_HRS:
            # TODO

    # append params_line to csv_filepath

    
def create_file_with_tweaked_parameters(csv_filepath):
    """
    TODO this function will create file with tweaked parameters:
    FORMAT: each line has comma-seperated list of: <values for each disease model param>, 
    <values for each diffusion model param>
    """
    pass


def analyze_simulation_results():
    """
    TODO analyze the DataStructure or file created after running read_tweaked_parameter_file_run_simulation()
    """
    pass



def calc_distance_ratio(enum_distanceMetric, simulated_json_file, sim_end_date, sim_start_date):
    """
    TODO 

            distance between simulated_json_file & formatted_file(sim_start_date)
    ratio = ---------------------------------------------------------------------
            distance between formatted_file(sim_end_date) & formatted_file(sim_start_date)

    return ratio

    """


    
def read_tweaked_parameter_file_run_simulation(list_date_simtime_pair, tweaked_param_csv):
    """
    FORMAT of csv file with tweaked params: comma-seperated list of: <values for each disease model param>, 
    <values for each diffusion model param>

    - for each (date, sim_time) pair:
        - use date to construct formatted-jhu-data json file by calling prepare_dataset.prepare_data
        - for each line in the "tweaked param" csv:
            - substitute values from the line in the formatted-json-file, and construct a new json file, lets
              call it "tweaked-json-file"
            - run simulation with tweaked-json-file:
              run_simulation(tweaked-json-file, sim_runtime_units, sim_out_filepath)
            - calculate "distance" ratio, call calc_distance_ratio()
            - capture the tuple of (start_date, end_date, list_of_disease_params_used) in a DataStructure or file

    """



def trigger():

    try:
        
        (sim_start_date, sim_runtime_days, graph_type, distance_metric) = parse_cmdargs()

        setup_logging()

        sim_runtime_units = sim_runtime_days * 24

        
        formatted_json_filepath = prepare_dataset.prepare_data(covid_csse_data_date=sim_start_date,
                                                               graph_type=graph_type)

        run_simulation(formatted_json_filepath, sim_runtime_units, sim_output_filepath)

        read_tweaked_parameter_file_run_simulation(list_date_simtime_pair, tweaked_param_csv)

        
        add_end_log()

    except Exception as e:
        logging.exception(str(type(e).__name__) + ": " + str(e), exc_info=True)
        logging.info("Exiting ...\n\n")

        print("Exception occured, view log file")

        sys.exit(1)


# MAIN
if __name__ == '__main__':
    trigger()

