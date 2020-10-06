#!/usr/bin/env python3

try:
    import os
    import json
    import requests
    import json
    import uuid
    # import bottle
    import subprocess
    import datetime as dt
    import multiprocessing as mp
    from bottle import auth_basic, run, route, default_app, template, get, post, request, static_file, response, debug

    # import helper
except Exception as e:
    print(str(type(e).__name__) + ": " + str(e), file=sys.stderr)
    sys.exit(1)

pandemic_sim_driver_path = "./pandemic-sim-driver.py"
workerPool = None
initCalled = False
jobstatus = {}
sim_result_files_dir = 'simresults/'

# print("here_h1")
simJobs = []


def init():
    """

    :return:
    """
    global initCalled
    global workerPool

    if not initCalled:
        workerPool = mp.Pool()

    initCalled = True


def create_paramtweaks_file(reqdata):
    """
    :param reqdata:
    :return:
    """
    tweakfile = "diseaseparamtweakfile_" + str(uuid.uuid4())

    tweakfileobj = open(tweakfile, "w")

    tweakscount = int(reqdata['tweakscount']['value'])

    dicttweakline = {}

    if reqdata['transmissibility']['ifchecked'] == True \
            or reqdata['mean_incubation_duration_in_days']['ifchecked'] == True \
            or reqdata['mean_infection_duration_in_days']['ifchecked'] == True \
            or reqdata['mortality_ratio']['ifchecked'] == True \
            or reqdata['update_trig_interval_in_hrs']['ifchecked'] == True:
        dicttweakline["disease_model"] = {}

    if reqdata['graph_type']['ifchecked'] == True \
            or reqdata['graph_params']['ifchecked'] == True \
            or reqdata['diffusion_trig_interval_in_hrs']['ifchecked'] == True \
            or reqdata['diffusion_trig_interval_in_hrs']['ifchecked'] == True \
            or reqdata['avg_transport_speed']['ifchecked'] == True \
            or reqdata['max_diffusion_cnt']['ifchecked'] == True:
        dicttweakline["diffusion_model"] = {}

    for i in range(tweakscount):
        # clear dict

        if reqdata['transmissibility']['ifchecked'] == True:
            transmissibilityfrom = float(reqdata['transmissibility']['fromval'])
            transmissibilityto = float(reqdata['transmissibility']['toval'])

            dicttweakline["disease_model"]['transmissibility'] = round(transmissibilityfrom + i * ((transmissibilityto -
                                                                                                    transmissibilityfrom) / (
                                                                                                           tweakscount - 1)),
                                                                       3)

        if reqdata['mean_incubation_duration_in_days']['ifchecked'] == True:
            mean_incubation_duration_in_daysfrom = \
                float(reqdata['mean_incubation_duration_in_days']['fromval'])
            mean_incubation_duration_in_daysto = \
                float(reqdata['mean_incubation_duration_in_days']['toval'])

            dicttweakline["disease_model"]['mean_incubation_duration_in_days'] = round(
                mean_incubation_duration_in_daysfrom + \
                i * ((mean_incubation_duration_in_daysto -
                      mean_incubation_duration_in_daysfrom) / (
                             tweakscount - 1)), 3)
        if reqdata['mean_infection_duration_in_days']['ifchecked'] == True:
            mean_infection_duration_in_daysfrom = \
                float(reqdata['mean_infection_duration_in_days']['fromval'])
            mean_infection_duration_in_daysto = \
                float(reqdata['mean_infection_duration_in_days']['toval'])

            dicttweakline["disease_model"]['mean_infection_duration_in_days'] = round(
                mean_infection_duration_in_daysfrom + \
                i * ((mean_infection_duration_in_daysto -
                      mean_infection_duration_in_daysfrom) / (
                             tweakscount - 1)), 3)

        if reqdata['mortality_ratio']['ifchecked'] == True:
            mortality_ratiofrom = float(reqdata['mortality_ratio']['fromval'])
            mortality_ratioto = float(reqdata['mortality_ratio']['toval'])

            dicttweakline["disease_model"]['mortality_ratio'] = round(mortality_ratiofrom + \
                                                                      i * ((mortality_ratioto -
                                                                            mortality_ratiofrom) / (
                                                                                   tweakscount - 1)), 3)

        if reqdata['update_trig_interval_in_hrs']['ifchecked'] == True:
            update_trig_interval_in_hrsfrom = float(reqdata['update_trig_interval_in_hrs']['fromval'])
            update_trig_interval_in_hrsto = float(reqdata['update_trig_interval_in_hrs']['toval'])

            dicttweakline["disease_model"]['update_trig_interval_in_hrs'] = round(update_trig_interval_in_hrsfrom + \
                                                                                  i * ((update_trig_interval_in_hrsto -
                                                                                        update_trig_interval_in_hrsfrom) / (
                                                                                               tweakscount - 1)), 3)

        if reqdata['graph_type']['ifchecked'] == True:
            graphtype1 = reqdata['graph_type']['type1']
            graphtype1count = float(reqdata['graph_type']['type1count'])
            graphtype2 = reqdata['graph_type']['type2']
            graphtype2count = float(reqdata['graph_type']['type2count'])

            if i < graphtype1count:
                dicttweakline["diffusion_model"]['graph_type'] = reqdata['graph_type']['type1']
            else:
                dicttweakline["diffusion_model"]['graph_type'] = reqdata['graph_type']['type2']

        if reqdata['graph_params']['ifchecked'] == True:
            print("graphparams", reqdata['graph_params']['fromval'])
            graph_param1from = float(reqdata['graph_params']['fromval'].split(',')[0])
            graph_param2from = float(reqdata['graph_params']['fromval'].split(',')[1])
            graph_param1to = float(reqdata['graph_params']['toval'].split(',')[0])
            graph_param2to = float(reqdata['graph_params']['toval'].split(',')[1])

            graph_param1 = graph_param1from + i * ((graph_param1to - graph_param1from) / (tweakscount - 1))
            graph_param2 = graph_param2from + i * ((graph_param2to - graph_param2from) / (tweakscount - 1))

            dicttweakline["diffusion_model"]['graph_params'] = str(round(graph_param1, 3)) + ',' + str(
                round(graph_param2, 3))

        if reqdata['diffusion_trig_interval_in_hrs']['ifchecked'] == True:
            diffusion_trig_interval_in_hrsfrom = float(reqdata['diffusion_trig_interval_in_hrs']['fromval'])
            diffusion_trig_interval_in_hrsto = float(reqdata['diffusion_trig_interval_in_hrs']['toval'])

            dicttweakline["diffusion_model"]['diffusion_trig_interval_in_hrs'] = round(
                diffusion_trig_interval_in_hrsfrom + \
                i * ((diffusion_trig_interval_in_hrsto -
                      diffusion_trig_interval_in_hrsfrom) / (
                             tweakscount - 1)), 3)

        if reqdata['avg_transport_speed']['ifchecked'] == True:
            avg_transport_speedfrom = float(reqdata['avg_transport_speed']['fromval'])
            avg_transport_speedto = float(reqdata['avg_transport_speed']['toval'])
            dicttweakline["diffusion_model"]['avg_transport_speed'] = round(avg_transport_speedfrom + \
                                                                            i * ((avg_transport_speedto -
                                                                                  avg_transport_speedfrom) / (
                                                                                         tweakscount - 1)), 3)

        if reqdata['max_diffusion_cnt']['ifchecked'] == True:
            max_diffusion_cntfrom = float(reqdata['max_diffusion_cnt']['fromval'])
            max_diffusion_cntto = float(reqdata['max_diffusion_cnt']['toval'])
            dicttweakline["diffusion_model"]['max_diffusion_cnt'] = round(max_diffusion_cntfrom + \
                                                                          i * ((max_diffusion_cntto -
                                                                                max_diffusion_cntfrom) / (
                                                                                       tweakscount - 1)), 3)

        print("tweakline", dicttweakline)
        tweakfileobj.write(json.dumps(dicttweakline) + '\n')

    tweakfileobj.close()

    return tweakfile


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
    :param:
    :return:
    """

    subprocessArgs = [pandemic_sim_driver_path, "--sim_start_date", pandemic_sim_driver_args['start_date'],
                      "--sim_runtime_days", pandemic_sim_driver_args['runtime_days'],
                      "--tweaked_params_file", pandemic_sim_driver_args['tweaked_params_file'],
                      "--sim_result_file", sim_result_files_dir + pandemic_sim_driver_args['sim_result_file']]

    if 'dist_metrics' in pandemic_sim_driver_args:
        subprocessArgs += (["--use_metric"] + pandemic_sim_driver_args['dist_metrics'])

    # res = subprocess.run([pandemic_sim_driver_path, "--sim_start_date", pandemic_sim_driver_args['start_date'],
    #                       "--sim_runtime_days", pandemic_sim_driver_args['runtime_days'],
    #                       "--tweaked_params_file", pandemic_sim_driver_args['tweaked_params_file'],
    #                       "--sim_result_file", pandemic_sim_driver_args['sim_result_file']])

    res = subprocess.run(subprocessArgs)

    return res.returncode


@route('/getstatus')
def getstatus():
    """

    :return:
    """

    global jobstatus

    for jobid in range(len(simJobs)):

        if jobstatus[jobid] is None:
            jobstatus[jobid] = {}

        res = simJobs[jobid][0]
        try:
            res.successful()
        except ValueError:

            if jobstatus[jobid]["status"] == "running":
                jobstatus[jobid]["updatebanner"] = "no"
                continue

            jobstatus[jobid]["status"] = "running"
            jobstatus[jobid]["bannertext"] = "[RUNNING] job added at {}".format(
                simJobs[jobid][2].strftime("%b %d, %Y <%H:%M:%S %Z>")
            )

            jobstatus[jobid]["updatebanner"] = "yes"
            print(jobid, "is running")
            continue

        # job has finished
        if jobstatus[jobid]["status"] == "success" or jobstatus[jobid]["status"] == "failure":
            jobstatus[jobid]["updatebanner"] = "no"
            continue

        print(jobid, "finished")  # finished with success / failure
        if res.get() == 0:
            timeJobFinished = dt.datetime.now(dt.timezone.utc).astimezone()
            timeElapsed = timeJobFinished - simJobs[jobid][2]

            jobstatus[jobid]["status"] = "success"
            jobstatus[jobid]["filename"] = sim_result_files_dir + simJobs[jobid][1]
            jobstatus[jobid]["bannertext"] = \
                "[SUCCESS] job[{}] finished on {}, time elapsed: {} seconds".format(
                    jobid,
                    timeJobFinished.strftime("%b %d, %Y <%H:%M:%S %Z>"),
                    timeElapsed.seconds
                )
            jobstatus[jobid]["updatebanner"] = "yes"

        else:
            jobstatus[jobid]["status"] = "failure"
            jobstatus[jobid]["bannertext"] = "[FAILURE] job was started at {}".format(
                simJobs[jobid][2].strftime("%b %d, %Y <%H:%M:%S %Z>")
            )
            jobstatus[jobid]["updatebanner"] = "yes"

    response.content_type = 'application/json'

    return json.dumps(jobstatus)


@post('/simulate')
def simulate():
    print("got post!")

    dictreq = json.loads(request.forms.get('data'))

    print(dictreq)
    tweakfile = create_paramtweaks_file(dictreq)

    # add dist metrics
    listDistmetrics = []
    if dictreq['distmetrictype']['ifchecked'] == True:
        if dictreq['distmetrictype']['ifmetricwasschecked'] == True:
            listDistmetrics.append('wass')
        if dictreq['distmetrictype']['ifmetricshajenchecked'] == True:
            listDistmetrics.append('shajen')

    # runtime_days = dictreq['runtime_days']
    triggersimulationargs = {
        'start_date': dictreq['start_date']['value'],
        'runtime_days': dictreq['runtime_days']['value'],
        'tweaked_params_file': tweakfile,
        'sim_result_file': dictreq['sim_result_file']['value'],
    }

    if not listDistmetrics:
        triggersimulationargs['dist_metric'] = listDistmetrics

    jobAddedTime = dt.datetime.now(dt.timezone.utc).astimezone()

    global simJobs
    simJobs.append(
        (workerPool.apply_async(triggersimulation, (triggersimulationargs,)),
         dictreq['sim_result_file']['value'],
         jobAddedTime)
    )

    print("added job")

    response.content_type = 'application/json'

    global jobstatus
    jobstatus[len(simJobs) - 1] = {"status": "jobadded"}

    return json.dumps({
        'jobid': (len(simJobs) - 1),
        # 'bannertext': "[RUNNING] job[{}] added at {}".format(
        #     len(simJobs) - 1,
        #     jobAddedTime.strftime("%b %d, %Y <%H:%M:%S %Z>")
        # )
    })


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
