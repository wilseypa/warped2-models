#!/usr/bin/env python3

try:
    import os
    import sys
    import json
    import requests
    import json
    import threading
    import time
    import uuid
    import shutil
    import math
    import re  # NOTE comment to test job running status update failure
    import subprocess
    import datetime as dt
    import multiprocessing as mp
    import pathlib
    from bottle import auth_basic, run, route, default_app, template, get, post, request, static_file, response, debug
    import pandas

    # import helper
except Exception as e:
    print(str(type(e).__name__) + ": " + str(e), file=sys.stderr)
    sys.exit(1)

simulateFuncjob = None
formatted_csse_infile_prefix = ".formatted-JHU-data.json"
simulated_file_prefix = ".simulated-data.json"
csse_formatted_json_dir = "../../data"
simulated_json_dir = "simOutfiles"
newJobStartStatus = ""


workerPool = None
initCalled = False
jobstatus = {}
taskCount = None

simJobs = {}


def init():
    """

    :return:
    """
    global initCalled
    global workerPool
    global taskCount

    taskCount = max(1, os.cpu_count() - 1)

    if not initCalled:
        workerPool = mp.Pool(processes=taskCount)

    initCalled = True


@post('/getstatus')
def getstatus():
    """
    """
    # TODO potential race condition ??

    print("!!!!!getstatus called")
    jobid = request.json['jobid']
    jobstatusToReturn = {'jobid':jobid, 'status':"NOT_FOUND"}

    if jobid in simJobs:
        result = simJobs[jobid]
        print("!!!result", result, type(result))

        try:
            print("!!!!trying result.successful")
            result.successful()

        except ValueError:
            # task is running
            jobstatusToReturn['status'] = "RUNNING"

        if jobstatusToReturn['status'] != "RUNNING":
            if result.get() == 0:
                jobstatusToReturn['status'] = "SUCCESS"
            else:
                jobstatusToReturn['status'] = 'FAILURE'

        response.content_type = 'application/json'

    return json.dumps(jobstatusToReturn)


def getCountyStatsFromFile(fips, filepath):
    """
    """
    with open(filepath) as f:
        filedata = f.read()

    matches = re.search(r'^\s*\["' + fips + r'",([^,]*,){5}(([^,]*,){3}[^,]*),[^,]*\],\s*$', filedata,
                        re.MULTILINE)

    print(":::::::filepath", filepath, "matches", matches)

    if not matches:
        print("::::::::no match")
        return

    disease_metrics = [int(x) for x in matches.group(2).split(',')]
    print(":::::::disease_metrics", disease_metrics[0], disease_metrics[1], disease_metrics[2], disease_metrics[3])

    # TODO better approach
    return disease_metrics[0], disease_metrics[1], disease_metrics[2], disease_metrics[3]


def getStatsFromFile(filename):
    """
    """
    with open(filename) as f:
        jsonData = json.load(f)

    totalConfirmed = 0
    totalDeaths = 0
    totalRecovered = 0
    totalActive = 0
    totalPopulation = 0

    for item in jsonData["locations"]:
        confirmed = item[6]
        deaths = item[7]
        recovered = item[8]
        active = item[9]
        population = item[10]

        totalConfirmed += confirmed
        totalDeaths += deaths
        totalRecovered += recovered
        totalActive += active
        totalPopulation += population

    return totalConfirmed, totalDeaths, totalRecovered, totalActive, totalPopulation
    # print(jsonData["locations"][0])


def processFilesForDateRange_county(fips, formatted_or_simulated, from_date, end_date, json_formatted_dir=None,
                                    simulated_json_dir=None, dir_to_write="plotSourceData"):
    """
    """
    to_write_to_file = "Date,Confirmed,Deaths,Recovered,Active\n"  # set header

    curr_date = from_date
    print("::::::::", formatted_or_simulated)

    while curr_date <= end_date:
        curr_datestr = curr_date.strftime("%m-%d-%Y")

        if formatted_or_simulated == "formatted":
            jsonfilepath = json_formatted_dir + "/" + curr_datestr + formatted_csse_infile_prefix
        else:
            jsonfilepath = simulated_json_dir + "/" + curr_datestr + simulated_file_prefix

        print(":::::::fips", fips)
        totalConfirmed, totalDeaths, totalRecovered, totalActive = getCountyStatsFromFile(fips, jsonfilepath)

        curr_date += dt.timedelta(days=1)

        to_write_to_file += ",".join([str(curr_datestr), str(totalConfirmed),
                                      str(totalDeaths),
                                      str(totalRecovered),
                                      str(totalActive)])

        to_write_to_file += '\n'

    # create nested subdir
    complete_dir_path = dir_to_write + "/" + str(fips)
    pathlib.Path(complete_dir_path).mkdir(parents=True, exist_ok=True)

    if formatted_or_simulated == "formatted":
        outfilename = "actual.csv"
    else:
        outfilename = "simulated.csv"

    with open(complete_dir_path + "/" + outfilename, "w") as f:
        f.write(to_write_to_file)


def processFilesForDateRange(formatted_or_simulated, from_date, end_date, json_formatted_dirpath=None,
                             simulated_json_dir=None, dir_to_write="plotSourceData"):
    """
    """
    to_write_to_file = "Date,Confirmed,Deaths,Recovered,Active,Population\n"  # set header

    curr_date = from_date

    while curr_date <= end_date:

        curr_datestr = curr_date.strftime("%m-%d-%Y")

        if formatted_or_simulated == "formatted":
            jsonfilepath = json_formatted_dirpath + "/" + curr_datestr + formatted_csse_infile_prefix
        else:
            jsonfilepath = simulated_json_dir + "/" + curr_datestr + simulated_file_prefix

        totalConfirmed, totalDeaths, totalRecovered, totalActive, totalPopulation = \
            getStatsFromFile(jsonfilepath)

        curr_date += dt.timedelta(days=1)

        to_write_to_file += ",".join([str(curr_datestr), str(totalConfirmed),
                                      str(totalDeaths),
                                      str(totalRecovered),
                                      str(totalActive),
                                      str(totalPopulation)])
        to_write_to_file += '\n'

    # create nested subdir
    complete_dir_path = dir_to_write + "/US"
    pathlib.Path(complete_dir_path).mkdir(parents=True, exist_ok=True)

    if formatted_or_simulated == "formatted":
        outfilename = "actual.csv"
    else:
        outfilename = "simulated.csv"

    with open(complete_dir_path + "/" + outfilename, "w") as f:
        f.write(to_write_to_file)


def tweak_infile(injsonfilepath, reqdata, tweakedjsonfilepath):
    """
    """
    baseinfileobj = open(injsonfilepath, 'r')
    basejsondata = json.load(baseinfileobj)
    baseinfileobj.close()

    tweakdict = {}
    paramRollingCounter = []

    if reqdata['transmissibility']['ifchecked']:
        basejsondata['disease_model']['transmissibility'] = float(reqdata['transmissibility']['value'])

    if reqdata['exposed_confirmed_ratio']['ifchecked']:
        basejsondata['disease_model']['exposed_confirmed_ratio'] = float(reqdata['exposed_confirmed_ratio']['value'])

    if reqdata['mean_incubation_duration_in_days']['ifchecked']:
        basejsondata['disease_model']['mean_incubation_duration_in_days'] = float(
            reqdata['mean_incubation_duration_in_days']['value'])

    if reqdata['mean_infection_duration_in_days']['ifchecked']:
        basejsondata['disease_model']['mean_infection_duration_in_days'] = float(
            reqdata['mean_infection_duration_in_days']['value'])

    if reqdata['mortality_ratio']['ifchecked']:
        basejsondata['disease_model']['mortality_ratio'] = float(reqdata['mortality_ratio']['value'])

    if reqdata['update_trig_interval_in_hrs']['ifchecked']:
        basejsondata['disease_model']['update_trig_interval_in_hrs'] = float(
            reqdata['update_trig_interval_in_hrs']['value'])

    if reqdata['graph_type']['ifchecked']:
        basejsondata['diffusion_model']['graph_type'] = reqdata['graph_type']['value']

    if reqdata['graph_params']['ifchecked']:
        basejsondata['diffusion_model']['graph_params'] = reqdata['graph_params']['K_val'] + "," + \
                                                          reqdata['graph_params']['beta_val']

    if reqdata['diffusion_trig_interval_in_hrs']['ifchecked']:
        basejsondata['diffusion_model']['diffusion_trig_interval_in_hrs'] = float(
            reqdata['diffusion_trig_interval_in_hrs']['value'])

    if reqdata['avg_transport_speed']['ifchecked']:
        basejsondata['diffusion_model']['avg_transport_speed'] = float(reqdata['avg_transport_speed']['value'])

    if reqdata['max_diffusion_cnt']['ifchecked']:
        basejsondata['diffusion_model']['max_diffusion_cnt'] = float(reqdata['max_diffusion_cnt']['value'])

    print("writing to file:", tweakedjsonfilepath)
    with open(tweakedjsonfilepath, "w") as f:
        json.dump(basejsondata, f)

    return {"paramsUsed": {"disease_model": basejsondata["disease_model"],
                           "diffusion_model": basejsondata["diffusion_model"]}}


def run_range_simulations(dict_args):
    """

    :param dict_req:
    :return:
    """

    baseWorkingDir = os.getcwd()

    os.chdir(baseWorkingDir + "/simJobs")
    jobDirName = "simJob_" + dict_args['jobid']

    os.mkdir(jobDirName)
    os.chdir(jobDirName)

    # now, we are in the unique job directory

    os.mkdir('tweakedjsoninfiles')
    os.mkdir('plotSourceData')
    os.mkdir(simulated_json_dir)

    start_date = dt.datetime.strptime(dict_args['start_date']['value'], "%m-%d-%Y")
    days_runtime = int(dict_args['runtime_days']['value'])

    injsonfilepath = baseWorkingDir + "/" + csse_formatted_json_dir + "/" + dict_args['start_date']['value'] + \
                     formatted_csse_infile_prefix

    tweakedjsoninfilepath = "tweakedjsoninfiles/" + dict_args['start_date']['value'] + ".tweaked.json"

    # tweak infile
    final_params = tweak_infile(injsonfilepath, dict_args, tweakedjsoninfilepath)

    # iterate for all days
    curr_days_runtime = 1
    while curr_days_runtime <= days_runtime:
        curr_outfile = (start_date + dt.timedelta(days=curr_days_runtime)).strftime("%m-%d-%Y") \
                       + simulated_file_prefix

        subprocess.run([baseWorkingDir + "/" + "../../pandemic_sim", "-m", tweakedjsoninfilepath, "--max-sim-time",
                        str(24 * int(curr_days_runtime)),
                        "-o",
                        simulated_json_dir + "/" + curr_outfile], stdout=subprocess.DEVNULL)

        curr_days_runtime += 1

    end_date = start_date + dt.timedelta(days=days_runtime)

    if dict_args['actualplot_end_date']['value']:
        actualplot_end_date = dt.datetime.strptime(dict_args['actualplot_end_date']['value'], "%m-%d-%Y")
    else:
        actualplot_end_date = end_date

    # create csv for US from original formatted files for the entire date range
    processFilesForDateRange("formatted", start_date + dt.timedelta(days=1), actualplot_end_date,
                             json_formatted_dirpath=(baseWorkingDir + "/" + csse_formatted_json_dir),
                             simulated_json_dir=None)

    # create file containing tweaked metrics
    with open("plotSourceData/metrics", "w") as f:
        f.write(json.dumps(final_params, indent=2))

    # create csv for US from simulation output files
    processFilesForDateRange("simulated", start_date + dt.timedelta(days=1), end_date, json_formatted_dirpath=None,
                             simulated_json_dir=simulated_json_dir)

    # now, for INDIVIDUAL COUNTIES
    if dict_args['fipslist']['value']:
        fips = dict_args['fipslist']['value']
        listfips = [x.strip() for x in fips.split(',')]

        print("::::::::listfips", listfips)

        for fips in listfips:
            processFilesForDateRange_county(fips, "formatted", start_date + dt.timedelta(days=1), actualplot_end_date,
                                            json_formatted_dir=csse_formatted_json_dir,
                                            simulated_json_dir=None)

            processFilesForDateRange_county(fips, "simulated", start_date + dt.timedelta(days=1), end_date,
                                            json_formatted_dir=None,
                                            simulated_json_dir=simulated_json_dir)

    # copy R plot script to the job directory, and execute
    shutil.copyfile(baseWorkingDir + "/" + "rangeSimulationDataAnalyse.R", "rangeSimulationDataAnalyse.R")
    subprocess.run(["Rscript", "rangeSimulationDataAnalyse.R"], stdout=subprocess.DEVNULL)

    os.chdir(baseWorkingDir)

    return 0



def simulate_func(dict_args):
    """
    :param dictreq:
    :return:
    """
    global simJobs
    global jobstatus
    global workerPool

    jobid = dict_args['jobid']

    global newJObStartStatus

    if jobid in simJobs:
        newJobStartStatus = "job with the same ID already exists"
        return

    simJobs[jobid] = workerPool.apply_async(run_range_simulations, (dict_args,))

    print("!!!! simjobs added")

    jobstatus[jobid] = {"status": "jobadded"}

    newJobStartStatus = "job added"

@post('/simulate')
def simulate():
    """

    :return:
    """
    print("req data", request.json, type(request.json))
    dictreq = request.json
    # dictreq = json.loads(request.forms.get('data'))
    print("post data", dictreq)

    response.content_type = 'application/json'

    global simulateFuncjob

    # in case any job is already running, return error message
    if simulateFuncjob is None or simulateFuncjob.is_alive() is False:
        simulateFuncjob = threading.Thread(target=simulate_func, args=(dictreq,))
        simulateFuncjob.start()
        print("!!!!newJobStartStatus:", newJobStartStatus)
        return json.dumps({"statusmsg": newJobStartStatus})
    else:
        return json.dumps({"statusmsg": "Wait until this message disappears and TRY again, previous simulate jobs "
                                        "aren't scheduled yet"})


@route('/<filename:re:.*\.css>')
def send_static(filename):
    return static_file(filename, root='css/')


@route('/<filename:re:.*\.js>')
def send_static(filename):
    return static_file(filename, root='js/')


@route('/<filename:re:.*\.png>')
def send_static(filename):
    return static_file(filename, root='static/')


def check_user(username, password):
    if username == "scooby" and password == "doobyd00":
        return True
    else:
        return False


@route('/')
@auth_basic(check_user)
def home():
    with open('index.html', 'r') as myfile:
        html_string = myfile.read()

    return template(html_string)  # , select_opts=get_select_opts_genre())

init()
run(host='localhost', port=8081, reloader=True, debug=True)
