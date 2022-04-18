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

pandemic_sim_driver_path = "./pandemic_sim_driver.py"
TWEAKFILES_DIR_PATH = "./tweakfiles/"
workerPool = None
initCalled = False
jobstatus = {}
# create directory explicitly ??
sim_result_files_dir = 'simresults/'  # TODO remove slash /
taskCount = None

simJobs = {}
simulateFuncjob = None


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

    pathlib.Path(TWEAKFILES_DIR_PATH).mkdir(parents=True, exist_ok=True)

    initCalled = True


def create_paramtweaks_file(reqdata):
    """
    :param reqdata:
    :return: list of tweaked json infile paths
    """

    # helper function
    def get_tweak_dict():
        """
        TODO: what is happening here ???
        :return:
        """
        # print("paramRollingCounter", paramRollingCounter)
        dictTweak = {}

        # not sure how this works:
        # >>> d1 = {}
        # >>> d2 = d1
        # >>> d2["a"] = 1
        # >>> d2
        # {'a': 1}
        # >>> d1
        # {'a': 1}

        for item in paramRollingCounter:
            # copy reference
            dictTemp = dictTweak

            for key in item[0]:
                if key not in dictTemp:
                    dictTemp[key] = {}

                dictTemp2 = dictTemp
                dictTemp = dictTemp[key]

            if dictTemp2:
                dictTemp2[key] = item[4]

        return dictTweak

    # helper function
    def roll_param_meter():
        """

        :param listCounter:
        :param listLimit:
        :return:
        """

        def getCarryOverIndex():
            # print("paramRollingCounter", paramRollingCounter)
            for i in range(len(paramRollingCounter) - 2, -1, -1):  # start with second last index
                if paramRollingCounter[i][3] is not None and paramRollingCounter[i][4] < paramRollingCounter[i][2] and \
                        not math.isclose(paramRollingCounter[i][4], paramRollingCounter[i][2], rel_tol=1e-5):
                    # print("h1")
                    return i

            return -1

        nonlocal paramRollingCounter

        # TODO handle empty paramRollingCounter ???

        if paramRollingCounter[-1][3] is not None and paramRollingCounter[-1][4] < paramRollingCounter[-1][2] and \
                not math.isclose(paramRollingCounter[-1][4], paramRollingCounter[-1][2], rel_tol=1e-5):

            paramRollingCounter[-1][4] = round(paramRollingCounter[-1][4] + paramRollingCounter[-1][3], 3)
        else:
            carryIndex = getCarryOverIndex()

            if carryIndex == -1:
                return -1

            paramRollingCounter[carryIndex][4] = round(
                paramRollingCounter[carryIndex][4] + paramRollingCounter[carryIndex][3], 3)

            # reset
            for i in range(carryIndex + 1, len(paramRollingCounter)):
                paramRollingCounter[i][4] = paramRollingCounter[i][1]

    # END roll_param_meter()

    # HELPER function
    def add_tweak_dict_to_file(dictTweak):
        """
        TODO purpose ??
        """
        if not dictTweak:
            return

        # TODO add comment
        if 'diffusion_model' in dictTweak and 'graph_params_K' in dictTweak['diffusion_model'] and \
                'graph_params_beta' in dictTweak['diffusion_model']:
            graph_params_str = str(dictTweak['diffusion_model']['graph_params_K']) + ',' + \
                               str(dictTweak['diffusion_model']['graph_params_beta'])

            dictTweak['diffusion_model']['graph_params'] = graph_params_str

            del dictTweak['diffusion_model']['graph_params_K']
            del dictTweak['diffusion_model']['graph_params_beta']

        # nonlocal tweakfileobj
        nonlocal tweakfilesobj
        tweakfilesobj[tweakFileIndex].write(json.dumps(dictTweak) + '\n')

    # END addTweakDictToFile()
    ################ END HELPER FUNCTIONS ################

    # TODO comment
    yyg_df = pandas.read_csv(os.getcwd() + "/../data/" + \
                             "cumulative-estimated-infections-of-covid-19.dateFormatted.csv")

    simStartDate = reqdata['start_date']['value']

    if simStartDate in yyg_df.Date.values:
        confirmedCount = yyg_df.loc[yyg_df['Date'] == simStartDate]["Cumulative_confirmed_cases"].iloc[0]
        estimatedInfectedCount = yyg_df.loc[yyg_df['Date'] == \
                                            simStartDate]["Cumulative_estimated_infections_YYG"].iloc[0]

        if confirmedCount == 0 or estimatedInfectedCount == 0:
            yygExposedConfirmedRatio = 4
        else:
            yygExposedConfirmedRatio = round(estimatedInfectedCount / confirmedCount, 4)

    else:
        # if date not found in csv
        yygExposedConfirmedRatio = 4

    # create counter limit lists
    paramRollingCounter = []    # graph_param_betaIndex = None

    if reqdata['transmissibility']['ifchecked'] is True:
        transmissibilityfrom = float(reqdata['transmissibility']['fromval'])
        transmissibilityto = float(reqdata['transmissibility']['toval'])
        transmissibilitystep = float(reqdata['transmissibility']['step'])

        # [4] index holds current value
        paramRollingCounter.append([('disease_model', 'transmissibility'), transmissibilityfrom, transmissibilityto,
                                    transmissibilitystep,
                                    transmissibilityfrom])

    # for exposed confirmed factor
    # TODO since this value does not vary, all tweaklines will contain same value, wasting space

    if reqdata['exposed_confirmed_ratio']['ifchecked'] is True:
        if reqdata['exposed_confirmed_ratio']['ifYYGchecked'] is True:
            exposed_confirmed_ratio = yygExposedConfirmedRatio

            paramRollingCounter.append([('disease_model', 'exposed_confirmed_ratio'),
                                        exposed_confirmed_ratio,
                                        exposed_confirmed_ratio,
                                        0,
                                        exposed_confirmed_ratio])
        else:
            exposed_confirmed_ratiofrom = float(reqdata['exposed_confirmed_ratio']['fromval'])
            exposed_confirmed_ratioto = float(reqdata['exposed_confirmed_ratio']['toval'])
            exposed_confirmed_ratiostep = float(reqdata['exposed_confirmed_ratio']['step'])

            paramRollingCounter.append([('disease_model', 'exposed_confirmed_ratio'),
                                        exposed_confirmed_ratiofrom,
                                        exposed_confirmed_ratioto,
                                        exposed_confirmed_ratiostep,
                                        exposed_confirmed_ratiofrom])

    if reqdata['mean_incubation_duration_in_days']['ifchecked'] is True:
        mean_incubation_duration_in_daysfrom = \
            float(reqdata['mean_incubation_duration_in_days']['fromval'])
        mean_incubation_duration_in_daysto = \
            float(reqdata['mean_incubation_duration_in_days']['toval'])
        mean_incubation_duration_in_daysstep = \
            float(reqdata['mean_incubation_duration_in_days']['step'])

        paramRollingCounter.append([('disease_model', 'mean_incubation_duration_in_days'),
                                    mean_incubation_duration_in_daysfrom,
                                    mean_incubation_duration_in_daysto,
                                    mean_incubation_duration_in_daysstep,
                                    mean_incubation_duration_in_daysfrom])

    if reqdata['mean_infection_duration_in_days']['ifchecked'] is True:
        mean_infection_duration_in_daysfrom = \
            float(reqdata['mean_infection_duration_in_days']['fromval'])
        mean_infection_duration_in_daysto = \
            float(reqdata['mean_infection_duration_in_days']['toval'])
        mean_infection_duration_in_daysstep = \
            float(reqdata['mean_infection_duration_in_days']['step'])

        paramRollingCounter.append([('disease_model', 'mean_infection_duration_in_days'),
                                    mean_infection_duration_in_daysfrom,
                                    mean_infection_duration_in_daysto,
                                    mean_infection_duration_in_daysstep,
                                    mean_infection_duration_in_daysfrom])

    if reqdata['mortality_ratio']['ifchecked'] is True:
        mortality_ratiofrom = float(reqdata['mortality_ratio']['fromval'])
        mortality_ratioto = float(reqdata['mortality_ratio']['toval'])
        mortality_ratiostep = float(reqdata['mortality_ratio']['step'])

        paramRollingCounter.append([('disease_model', 'mortality_ratio'),
                                    mortality_ratiofrom,
                                    mortality_ratioto,
                                    mortality_ratiostep,
                                    mortality_ratiofrom])

    if reqdata['update_trig_interval_in_hrs']['ifchecked'] is True:
        update_trig_interval_in_hrsfrom = float(reqdata['update_trig_interval_in_hrs']['fromval'])
        update_trig_interval_in_hrsto = float(reqdata['update_trig_interval_in_hrs']['toval'])
        update_trig_interval_in_hrsstep = float(reqdata['update_trig_interval_in_hrs']['step'])

        paramRollingCounter.append([('disease_model', 'update_trig_interval_in_hrs'),
                                    update_trig_interval_in_hrsfrom,
                                    update_trig_interval_in_hrsto,
                                    update_trig_interval_in_hrsstep,
                                    update_trig_interval_in_hrsfrom])

    if reqdata['graph_type']['ifchecked'] is True:
        graphtype = reqdata['graph_type']['type']

        # TODO add comment
        paramRollingCounter.append([('diffusion_model', 'graph_type'),
                                    graphtype,
                                    graphtype,
                                    None,
                                    graphtype])

    if reqdata['graph_params']['ifchecked'] is True:
        # print("graphparams", reqdata['graph_params']['fromval'])
        graph_param_Kfrom = float(reqdata['graph_params']['K_fromval'])  # .split(',')[0])
        graph_param_Kto = float(reqdata['graph_params']['K_toval'])  # .split(',')[0])
        graph_param_Kstep = float(reqdata['graph_params']['K_step'])  # .split(',')[0])
        graph_param_betafrom = float(reqdata['graph_params']['beta_fromval'])  # .split(',')[1])
        graph_param_betato = float(reqdata['graph_params']['beta_toval'])  # .split(',')[1])
        graph_param_betastep = float(reqdata['graph_params']['beta_step'])  # .split(',')[1])

        paramRollingCounter.append([('diffusion_model', 'graph_params_K'),
                                    graph_param_Kfrom,
                                    graph_param_Kto,
                                    graph_param_Kstep,
                                    graph_param_Kfrom])

        paramRollingCounter.append([('diffusion_model', 'graph_params_beta'),
                                    graph_param_betafrom,
                                    graph_param_betato,
                                    graph_param_betastep,
                                    graph_param_betafrom])

    if reqdata['diffusion_trig_interval_in_hrs']['ifchecked'] is True:
        diffusion_trig_interval_in_hrsfrom = float(reqdata['diffusion_trig_interval_in_hrs']['fromval'])
        diffusion_trig_interval_in_hrsto = float(reqdata['diffusion_trig_interval_in_hrs']['toval'])
        diffusion_trig_interval_in_hrsstep = float(reqdata['diffusion_trig_interval_in_hrs']['step'])

        paramRollingCounter.append([('diffusion_model', 'diffusion_trig_interval_in_hrs'),
                                    diffusion_trig_interval_in_hrsfrom,
                                    diffusion_trig_interval_in_hrsto,
                                    diffusion_trig_interval_in_hrsstep,
                                    diffusion_trig_interval_in_hrsfrom])

    if reqdata['avg_transport_speed']['ifchecked'] is True:
        avg_transport_speedfrom = float(reqdata['avg_transport_speed']['fromval'])
        avg_transport_speedto = float(reqdata['avg_transport_speed']['toval'])
        avg_transport_speedstep = float(reqdata['avg_transport_speed']['step'])

        paramRollingCounter.append([('diffusion_model', 'avg_transport_speed'),
                                    avg_transport_speedfrom,
                                    avg_transport_speedto,
                                    avg_transport_speedstep,
                                    avg_transport_speedfrom])

    if reqdata['max_diffusion_cnt']['ifchecked'] is True:
        max_diffusion_cntfrom = float(reqdata['max_diffusion_cnt']['fromval'])
        max_diffusion_cntto = float(reqdata['max_diffusion_cnt']['toval'])
        max_diffusion_cntstep = float(reqdata['max_diffusion_cnt']['step'])

        paramRollingCounter.append([('diffusion_model', 'max_diffusion_cnt'),
                                    max_diffusion_cntfrom,
                                    max_diffusion_cntto,
                                    max_diffusion_cntstep,
                                    max_diffusion_cntfrom])

    tweakfiles = []
    tweakfilesobj = []

    tweakFileIndex = 0

    # TODO explain!!
    while True:
        # create as many tweakfiles as 'taskCount' value (roughly, as many cpu cores)
        if tweakFileIndex > len(tweakfiles) - 1:
            tweakfiles.append(TWEAKFILES_DIR_PATH + "diseaseparamtweakfile_" + str(uuid.uuid4()))
            tweakfilesobj.append(open(tweakfiles[-1], "w"))

        # add next tweak-set to current index-ed tweakfile
        add_tweak_dict_to_file(get_tweak_dict())  # TODO change get_tweak_dict name

        # next tweak-set should go to the next tweakfile; reset index to loop to the first tweakfile
        tweakFileIndex += 1
        if tweakFileIndex == taskCount:
            tweakFileIndex = 0

        if roll_param_meter() == -1:
            break

    for tweakfileobj in tweakfilesobj:
        tweakfileobj.close()
    # sys.exit(-1)

    return tweakfiles

# to facilitate downloading of files (which files??) from UI
@route('/' + sim_result_files_dir + '<filename:re:.*>')
def downloadsimresultfile(filename):
    return static_file(filename, root=sim_result_files_dir, download=True)


@route('/<filename:re:.*\.css>')
def send_static(filename):
    return static_file(filename, root='css/')


@route('/<filename:re:.*\.js>')
def send_static(filename):
    return static_file(filename, root='js/')


@route('/<filename:re:.*\.png>')
def send_static(filename):
    return static_file(filename, root='static/')


def triggersimulation(pandemic_sim_driver_args):
    """
    TODO add comment
    """

    subprocessArgs = [pandemic_sim_driver_path, "--sim_start_date", pandemic_sim_driver_args['start_date'],
                      "--sim_runtime_days", pandemic_sim_driver_args['runtime_days'],
                      "--tweaked_params_file", pandemic_sim_driver_args['tweaked_params_file'],
                      "--sim_result_file", sim_result_files_dir + pandemic_sim_driver_args['sim_result_file']]

    # print("metric type", pandemic_sim_driver_args['dist_metrics'])
    # sys.exit(1)
    if 'dist_metrics' in pandemic_sim_driver_args:
        subprocessArgs += (["--use_metric"] + pandemic_sim_driver_args['dist_metrics'])

    res = subprocess.run(subprocessArgs, stderr=subprocess.PIPE, text=True)

    return res.returncode, res.stderr


def simulate_func(dictreq):
    """
    TODO purpose??
    """
    tweakfiles = create_paramtweaks_file(dictreq)  # expensive function

    # print("tweakfiles", tweakfiles)
    # sys.exit(1)

    listDistmetrics = []
    if dictreq['distmetrictype']['ifchecked'] == True:
        if dictreq['distmetrictype']['ifmetricwasschecked'] == True:
            listDistmetrics.append('wass')
        if dictreq['distmetrictype']['ifmetricjenshanchecked'] == True:
            listDistmetrics.append('jenshan')
        if dictreq['distmetrictype']['ifmetriceucdchecked'] == True:
            listDistmetrics.append('eucd')

    global simJobs
    global jobstatus
    global workerPool

    for tweakfile in tweakfiles:
        triggersimulationargs = {}

        triggersimulationargs = {
            'start_date': dictreq['start_date']['value'],
            'runtime_days': dictreq['runtime_days']['value'],
            'tweaked_params_file': tweakfile,
            'sim_result_file': dictreq['sim_result_file']['value'],
        }

        if listDistmetrics:
            triggersimulationargs['dist_metrics'] = listDistmetrics

        jobid = str(uuid.uuid4())  # create new job id
        simJobs[jobid] = ((workerPool.apply_async(triggersimulation, (triggersimulationargs,)),
                           dictreq['sim_result_file']['value'],
                           dt.datetime.now(dt.timezone.utc).astimezone()))

        jobstatus[jobid] = {"status": "jobadded"}


@route('/getstatus')
def getstatus():
    """

    :return:
    """
    # check if tweakfiles got created and new jobs got added
    if simulateFuncjob is not None and simulateFuncjob.is_alive():
        return json.dumps({})

    # NOTE potential bug?? if /simulate gets called while getstatus() is executing, simJobs may get updated if
    # multithreading runs truly parallelly, but python GIL's enforced sequential operation will possibly avert the
    # bug (as an unintentional consequence)

    # print("simulateFuncjob", simulateFuncjob)

    dictToSend = {}

    # TODO add comment
    dictToSend["statusmsg"] = ""

    global jobstatus
    # print("getstatus simjobs:", simJobs)

    for jobid in simJobs.keys():

        # if jobstatus[jobid] is None:  ??????????
        #     jobstatus[jobid] = {}

        result = simJobs[jobid][0]
        try:
            # check if the task has ended
            result.successful()
        except ValueError:
            # task is running

            # to check if job status was "running" already, from a previous /getstatus call
            if jobstatus[jobid]["status"] == "running":
                jobstatus[jobid]["updatebanner"] = "no"  # the banner on frontend already shows
                # "running", so no need to update
                continue

            # setting status to "running" the first time
            jobstatus[jobid]["status"] = "running"
            jobstatus[jobid]["bannertext"] = "[<b>RUNNING</b>] job_{} added on {}".format(
                jobid,
                simJobs[jobid][2].strftime("%b %d, %Y <%H:%M:%S %Z>")
            )
            jobstatus[jobid]["updatebanner"] = "yes"

            # print(jobid, "is running")
            continue

        # job has already finished

        # TODO potential bug??
        # if *success*/*failure* status was set in a previous /getstatus call, set "updatebanner" to no
        if jobstatus[jobid]["status"] == "success" or jobstatus[jobid]["status"] == "failure":
            jobstatus[jobid]["updatebanner"] = "no"
            continue

        # encountering a finished job, for the first time
        # print(jobid, "finished")  # finished with success / failure
        if result.get()[0] == 0:
            timeJobFinished = dt.datetime.now(dt.timezone.utc).astimezone()
            timeElapsed = timeJobFinished - simJobs[jobid][2]

            jobstatus[jobid]["status"] = "success"
            jobstatus[jobid]["filename"] = sim_result_files_dir + simJobs[jobid][1]
            # HACK adding markup tags
            jobstatus[jobid]["bannertext"] = \
                "[<b>SUCCESS</b>] job_{} :: [<b>{}</b>] simulations finished successfully on {} :: {} seconds " \
                "elapsed".format(
                    jobid,
                    re.search('CDRQM(.*)PAMCU', result.get()[1]).group(1),  # get stderr
                    timeJobFinished.strftime("%b %d, %Y <%H:%M:%S %Z>"),
                    timeElapsed.seconds)
            jobstatus[jobid]["updatebanner"] = "yes"

        else:  # job failed
            jobstatus[jobid]["status"] = "failure"
            jobstatus[jobid]["bannertext"] = "[<b>FAILURE</b>] job_{} was started at {}".format(
                jobid,
                simJobs[jobid][2].strftime("%b %d, %Y <%H:%M:%S %Z>"))
            jobstatus[jobid]["updatebanner"] = "yes"

    response.content_type = 'application/json'

    dictToSend["jobstatus"] = jobstatus

    return json.dumps(dictToSend)


@post('/simulate')
def simulate():
    # print("got post!")

    dictreq = json.loads(request.forms.get('data'))
    print("post data", dictreq)

    # sys.exit(1)

    response.content_type = 'application/json'

    global simulateFuncjob

    if simulateFuncjob is None or simulateFuncjob.is_alive() is False:
        # add new job
        simulateFuncjob = threading.Thread(target=simulate_func, args=(dictreq,))
        simulateFuncjob.start()
        return json.dumps({"statusmsg": "Preparing tweakfiles..."})
    else:
        return json.dumps({"statusmsg": "Wait until this message disappears and TRY again, previous simulate jobs "
                                        "aren't scheduled yet"})


def check_user(username, password):
    if username == "scooby" and password == "doobyd00":
        return True
    else:
        return False


@route('/')
@auth_basic(check_user)
def home():
    with open('pandemicindex.html', 'r') as myfile:
        html_string = myfile.read()

    return template(html_string)  # , select_opts=get_select_opts_genre())


init()
# app.run(host='localhost', port=8080, debug=True)
run(host='localhost', port=8080, reloader=True, debug=True)
# run(host='localhost', port=8080, debug=True)
