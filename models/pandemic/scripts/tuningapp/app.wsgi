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
    import pandas as pd
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
tweaked_json_infile_dir = "tweakedjson_infile"
newJobStartStatus = ""
baseWorkingDir = None

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

    # print("!!!!!getstatus called")
    jobid = request.json['jobid']
    jobstatusToReturn = {'jobid': jobid, 'status': "NOT_FOUND"}

    if jobid in simJobs:
        result = simJobs[jobid]
        # print("!!!result", result, type(result))

        try:
            # print("!!!!trying result.successful")
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
    print("!!!!!filepath:", filepath)
    print("!!!!cwd: ", os.getcwd())

    with open(filepath) as f:
        filedata = f.read()

    print("!! after with")

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


def getAggregateStatsFromFile(jsonfilepath, set_states=None):
    """
    """
    with open(jsonfilepath) as f:
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

        if set_states is not None:
            set_states.add(item[2].lower())

        totalConfirmed += confirmed
        totalDeaths += deaths
        totalRecovered += recovered
        totalActive += active
        totalPopulation += population

    return totalConfirmed, totalDeaths, totalRecovered, totalActive, totalPopulation
    # print(jsonData["locations"][0])


def processFilesForDateRange_county(fips, formatted_or_simulated, from_date, end_date,
                                    json_formatted_dirpath=None,
                                    simulated_json_dir=None, # relative path
                                    dir_to_write="plotSourceData"):
    """
    """
    to_write_to_file = "Date,Confirmed,Deaths,Recovered,Active\n"  # set header

    curr_date = from_date
    print("::::::::", formatted_or_simulated)

    while curr_date <= end_date:
        curr_datestr = curr_date.strftime("%m-%d-%Y")

        if formatted_or_simulated == "formatted":
            jsonfilepath = json_formatted_dirpath + "/" + curr_datestr \
                + formatted_csse_infile_prefix
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


def tweak_json_file_with_transmissibility(sim_mode, injson_filepath,
                                          list_transmissibility_values,
                                          curr_transmissibility_val):
    """
    """

    with open(injson_filepath, 'r+') as f:

        # print("!!h1:", injson_filepath)
        text = f.read()
        # print("!!h2")
        
        tbility_str = None

        if sim_mode is "chained":
            # tuning process NOT implemented for "chained" mode
            tbility_str = '[{}, {}]'.format(curr_transmissibility_val, curr_transmissibility_val)
        else: # range_series
            # HACK: -1.0 is a dummy value, to make sure the 'curr_transmissibility_val' value
            # is used as-is by the pandemic simulation, even if sim runtime is 1 day
            # print("!!h3")
            tbility_str = str(list_transmissibility_values + [{}, -1.0]).format(
                "%.7f" % curr_transmissibility_val)

            # print("!!h4")

        # print("!!h5")
        # print("!text:\n", text[0:160], "\n\n\n\n")
        # print("!!h5.5")
        text = re.sub(r'(.*)"transmissibility":\s*\[[-0-9., ]*\](.*)',
                      r'\1' + '"transmissibility": ' + tbility_str + r'\2',
                      text)
        # print("!!h6")

        # print("!!h7")
        f.seek(0)
        f.write(text)
        f.truncate()
        # print("!!h8")
    # print("!!h9")


def simulate_and_tune_transmissibility_using_bin_search(sim_mode, injson_filepath,
                                                        list_transmissibility_values,
                                                        curr_days_runtime,
                                                        simulated_json_dir,
                                                        curr_outfile,
                                                        curr_date,
                                                        json_formatted_dirpath,
                                                        actual_US_confirmed):
    """
    """
    # define limits
    lower_limit = 0
    upper_limit = 13

    simulated_json_filepath = simulated_json_dir + "/" + curr_outfile

    # print("actual_US_confirmed", actual_US_confirmed)

    i = 0
    while True:

        # print("\n\n>>binsearch tBility iteration: " + str(i))
        # print("sim_mode:", sim_mode)

        current_transmissibility_val = (upper_limit + lower_limit) / 2
        
        print("::{}:: lower limit: {}, upper limit: {}, current_tbility: {}"
              .format(i, lower_limit, upper_limit, current_transmissibility_val))
        i += 1

        # call simulation
        if sim_mode == "chained":
            raise Exception("Not implemented!!")

        else: # range_series
            tweak_json_file_with_transmissibility(sim_mode, injson_filepath,
                                                  list_transmissibility_values,
                                                  current_transmissibility_val)

            # print("calling subprocess.run ...")
            subprocess.run([baseWorkingDir + "/" + "../../pandemic_sim", "-m",
                            injson_filepath,
                            "--max-sim-time",
                            str(24 * int(curr_days_runtime)),
                            "-o",
                            simulated_json_filepath], #,
                           stdout=subprocess.DEVNULL)
            # print("called subprocess.run ...")
            
            # time.sleep(10)
            
            simulatedConfirmed = getAggregateStatsFromFile(simulated_json_filepath)[0]  # necessary!

        # print(">>>>> actual: {}, simulated: {}\n... SLEEPING ...".format(actual_US_confirmed, simulatedConfirmed))
        # time.sleep(2)

        print(">>>>> actual: {}, simulated: {}".format(actual_US_confirmed, simulatedConfirmed))
        
        if math.isclose(lower_limit, upper_limit, abs_tol=0.001):
            # print("..TOO close")
            break
        # else:
        #     print("Not close")

        if simulatedConfirmed > math.ceil(1.05 * actual_US_confirmed):
            upper_limit = current_transmissibility_val
        elif simulatedConfirmed < math.ceil(0.95 * actual_US_confirmed):
            lower_limit = current_transmissibility_val
        else:
            break

    return current_transmissibility_val

    # while simulatedConfirmed > (1.04 * actualConfirmed) or simulatedConfirmed < actualConfirmed:
    #     current_transmissibility = (upper_limit + lower_limit) / 2

    #     if sim_mode == "chained":

    #         tweak_json_file_with_transmissibility(
    #         transmissibi
        

def getactualUSstats(datestr, set_statesToUse):
    """
    get stats from states' data available in 'csse_covid_19_daily_reports_us' dir
    :return:
    """
    usstatsfilepath = baseWorkingDir + "/" +  csse_formatted_json_dir \
        + "/COVID-19.jhu/csse_covid_19_data/csse_covid_19_daily_reports_us" \
        + "/" \
        + datestr \
        + ".csv"

    # with open(usstatsfilepath, 'r') as statfile:
    #     print("!!!!!!", statfile.read(10))

    usstatsdf = pd.read_csv(usstatsfilepath,
                            usecols=['Province_State', 'Confirmed', 'Deaths', 'Recovered',
                                     'Active'],
                            skipinitialspace=True)
    # usstatsdf['Province_State'] = usstatsdf['Province_State'].str.lower()

    US_states_list_df = pd.DataFrame(list(set_statesToUse))

    # print("usstatsdf:\n", usstatsdf)    
    # print("US_states_list_df:\n", US_states_list_df)

    # time.sleep(20)

    US_states_list_df.columns = ['Province_State']


    US_stats_filtered_df = pd.merge(usstatsdf, US_states_list_df,
                                    left_on=usstatsdf['Province_State'].str.lower(),
                                    right_on=US_states_list_df['Province_State'],  # already lowercase
                                    how='inner')

    # print(US_stats_filtered_df)
    
    US_stats_filtered_df['Confirmed'].fillna(value=0, inplace=True, downcast='infer')
    US_stats_filtered_df['Deaths'].fillna(value=0, inplace=True, downcast='infer')
    US_stats_filtered_df['Recovered'].fillna(value=0, inplace=True, downcast='infer')
    US_stats_filtered_df['Active'].fillna(value=0, inplace=True, downcast='infer')

    totalValues = US_stats_filtered_df.sum(axis=0)
    
    # print("\n\nusstatsfilepath:", usstatsfilepath, "\n!!!totalValues:",
    #       totalValues, "\n\n")

    return totalValues.Confirmed, totalValues.Deaths, totalValues.Recovered, totalValues.Active


def processFilesForDateRange(formatted_or_simulated, from_date, end_date,
                             use_state_level_source_data=True,
                             json_formatted_dirpath=None,
                             simulated_json_dir=None,
                             dir_to_write="plotSourceData",
                             list_actual_US_confirmed=None):
    """
    create US-specific csv files for R scripts to plot 
    """

    # TODO here?? get set of all states from formatted county level json files, and use these
    # states to match US states' csv file
    
    to_write_to_file = "Date,Confirmed,Deaths,Recovered,Active,Population\n"  # set header

    curr_date = from_date

    while curr_date <= end_date:

        curr_datestr = curr_date.strftime("%m-%d-%Y")

        if formatted_or_simulated == "formatted":
            jsonfilepath = json_formatted_dirpath + "/" + curr_datestr + formatted_csse_infile_prefix
        else:
            jsonfilepath = simulated_json_dir + "/" + curr_datestr + simulated_file_prefix

        set_states = set()
        # get set of unique states from "formatted" json files containing counties
        totalConfirmed, totalDeaths, totalRecovered, totalActive, totalPopulation = \
            getAggregateStatsFromFile(jsonfilepath, set_states)

        # print("set of states from json files:")
        # print(set_states)
        # now, use the previous set to filter out non-states from states-level csv file
        # for "actual"(formatted) stats, use the US-level data in the jhu source TODO why??
        if formatted_or_simulated == "formatted":

            if use_state_level_source_data:
                totalConfirmed2, totalDeaths2, totalRecovered2, totalActive2 = \
                    getactualUSstats(curr_datestr, set_states)

                # commenting out this
                # # TODO change this to AND
                # # special handling because of csv file issue

                if totalConfirmed2 != 0:
                    totalConfirmed = totalConfirmed2
                
                if totalDeaths2 != 0:
                    totalDeaths = totalDeaths2
                                
                if totalActive2 != 0:
                    totalActive = totalActive2

                # if totalRecovered2 != 0:
                #     totalRecovered = totalRecovered2

                # calculating recovered as difference
                totalRecovered = totalConfirmed - totalDeaths - totalActive
                
                # if totalConfirmed2 != 0 and totalDeaths2 != 0 and totalRecovered2 != 0 and totalActive2 != 0:
                #     totalConfirmed = totalConfirmed2
                #     totalDeaths = totalDeaths2
                #     totalRecovered = totalRecovered2
                #     totalActive = totalActive2
                
            # store values in list. this is done to help the tuning algo
            list_actual_US_confirmed.append(totalConfirmed)

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


def tweak_infile(injsonfilepath, reqdata, tweakedjsoninfilepath):
    """
    """
    baseinfileobj = open(injsonfilepath, 'r')
    basejsondata = json.load(baseinfileobj)
    baseinfileobj.close()

    tweakdict = {}
    paramRollingCounter = []

    # HACK: prepare_data.py script is not updated to write transmissibility as 
    # array into *formatted* json file. So update value to array manually
    basejsondata['disease_model']['transmissibility'] = json.loads("[2.2]")

    print("request data transmissibility:", reqdata['transmissibility']['value'])

    if reqdata['transmissibility']['ifchecked']:
        # basejsondata['disease_model']['transmissibility'] = float(reqdata['transmissibility']['value'])
        basejsondata['disease_model']['transmissibility'] = json.loads(reqdata['transmissibility']['value'])
        
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

    print("writing to file:", tweakedjsoninfilepath)
    with open(tweakedjsoninfilepath, "w") as f:
        json.dump(basejsondata, f)

    return {"paramsUsed": {"disease_model": basejsondata["disease_model"],
                           "diffusion_model": basejsondata["diffusion_model"]}}


def run_range_simulations(dict_args):
    """

    :param dict_req:
    :return:
    """

    global baseWorkingDir
    baseWorkingDir = os.getcwd()

    os.chdir(baseWorkingDir + "/simJobs")
    jobDirName = "simJob_" + dict_args['jobid']

    os.mkdir(jobDirName)
    os.chdir(jobDirName)

    # now, we are in the unique job directory

    os.mkdir(tweaked_json_infile_dir)
    os.mkdir('plotSourceData')
    os.mkdir(simulated_json_dir)

    start_date = dt.datetime.strptime(dict_args['start_date']['value'], "%m-%d-%Y")
    days_runtime = int(dict_args['runtime_days']['value'])

    end_date = start_date + dt.timedelta(days=days_runtime)
    
    tune_transmissibility_flag = dict_args['tune_transmissibility_flag']['ifchecked']
    
    print("!!!tune_transmissibility_flag", tune_transmissibility_flag)

    list_actual_US_confirmed = []
    
    # MOVED
    actualplot_end_date = None
    if dict_args['actualplot_end_date']['value']:
        actualplot_end_date = dt.datetime.strptime(dict_args['actualplot_end_date']['value'], "%m-%d-%Y")
    else:
        actualplot_end_date = end_date

    json_formatted_dirpath = baseWorkingDir + "/" + csse_formatted_json_dir

    use_state_level_source_data = True
    
    # create csv for US from original formatted files for the entire date range
    processFilesForDateRange("formatted", start_date + dt.timedelta(days=1),
                             actualplot_end_date,
                             use_state_level_source_data=use_state_level_source_data,
                             json_formatted_dirpath=json_formatted_dirpath,
                             simulated_json_dir=None,
                             dir_to_write="plotSourceData",
                             list_actual_US_confirmed=list_actual_US_confirmed)
    # end MOVED
    
    sim_mode = dict_args['simulation_mode']['value']
    print("!!!simulation mode:", sim_mode)

    injsonfilepath = baseWorkingDir + "/" + csse_formatted_json_dir + "/" + dict_args['start_date']['value'] + \
                     formatted_csse_infile_prefix

    tweakedjsoninfilepath = tweaked_json_infile_dir + "/" \
        + dict_args['start_date']['value'] + ".tweaked.json"

    # transmissibility array, to be used for ...
    list_transmissibility_values = []

    # tweak infile
    final_params = tweak_infile(injsonfilepath, dict_args, tweakedjsoninfilepath)

    infilepath = tweakedjsoninfilepath

    sim_start_time = dt.datetime.now()
    
    # iterate for all days
    curr_days_runtime = 1
    while curr_days_runtime <= days_runtime:

        curr_date = start_date + dt.timedelta(days=curr_days_runtime)
        curr_outfile = curr_date.strftime("%m-%d-%Y") \
                       + simulated_file_prefix

        if sim_mode == "chained":

            if tune_transmissibility_flag is True:
                # also edit final_params - at the end??
                raise Exception("Not implemented")
            # tuned_val = simulate_and_tune_transmissibility_using_bin_search()
            else:
                subprocess.run([baseWorkingDir + "/" + "../../pandemic_sim", "-m",
                                infilepath,
                                "--max-sim-time",
                                str(24),
                                "-o",
                                simulated_json_dir + "/" + curr_outfile],
                               stdout=subprocess.DEVNULL)

                infilepath = simulated_json_dir + "/" + curr_outfile

        else: # range_series            
            # call new algo here???

            if tune_transmissibility_flag is True:
                tuned_transmissibility_val = simulate_and_tune_transmissibility_using_bin_search(
                    sim_mode,
                    infilepath,
                    list_transmissibility_values,
                    curr_days_runtime,
                    simulated_json_dir,
                    curr_outfile,
                    curr_date,
                    json_formatted_dirpath,
                    list_actual_US_confirmed[curr_days_runtime - 1])

                list_transmissibility_values.append(tuned_transmissibility_val)
            else:
                
                subprocess.run([baseWorkingDir + "/" + "../../pandemic_sim", "-m",
                                infilepath,
                                "--max-sim-time",
                                str(24 * int(curr_days_runtime)),
                                "-o",
                                simulated_json_dir + "/" + curr_outfile],
                               stdout=subprocess.DEVNULL)

        curr_days_runtime += 1

        print('💠', end='', flush=True)

    print("\n")  # add newline after the blue diamonds

    sim_end_time = dt.datetime.now()


    # now create file containing tweaked metrics and other stats

    final_stats = final_params

    # add other relevant info
    final_stats["start_date"] = str(start_date)
    final_stats["end_date"] = str(end_date)
    final_stats["actual_plot_end_date"] = str(actualplot_end_date)
    final_stats["simulation_mode"] = sim_mode
    final_stats["time_elapsed"] = str(sim_end_time - sim_start_time)
    final_stats["use_state_level_source_data"] = use_state_level_source_data
    
    with open("plotSourceData/metrics", "w") as f:
        f.write(json.dumps(final_stats, indent=2))

    # create csv for US from simulation output files
    processFilesForDateRange("simulated", start_date + dt.timedelta(days=1), end_date,
                             json_formatted_dirpath=None,
                             simulated_json_dir=simulated_json_dir)

    # now, for INDIVIDUAL COUNTIES
    if dict_args['fipslist']['value']:
        fips = dict_args['fipslist']['value']
        listfips = [x.strip() for x in fips.split(',')]

        print("::::::::listfips", listfips)

        for fips in listfips:
            processFilesForDateRange_county(fips, "formatted", start_date + dt.timedelta(days=1),
                                            actualplot_end_date,
                                            json_formatted_dirpath=json_formatted_dirpath,
                                            simulated_json_dir=None)

            processFilesForDateRange_county(fips, "simulated", start_date + dt.timedelta(days=1),
                                            end_date,
                                            json_formatted_dirpath=None,
                                            simulated_json_dir=simulated_json_dir)

    print("\n💀 {} 💀\n".format(jobDirName))
    # copy R plot script to the job directory, and execute
    shutil.copyfile(baseWorkingDir + "/" + "rangeSimulationDataAnalyse.R", "rangeSimulationDataAnalyse.R")
    subprocess.run(["Rscript", "rangeSimulationDataAnalyse.R"])
    # subprocess.run(["Rscript", "rangeSimulationDataAnalyse.R"], stdout=subprocess.DEVNULL)

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

    global newJobStartStatus

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
    global newJobStartStatus

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
run(host='localhost', port=8081, reloader=True, debug=True, quiet=True)
