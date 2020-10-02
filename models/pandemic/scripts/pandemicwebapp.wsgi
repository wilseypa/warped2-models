#!/usr/bin/env python3

try:
    import os
    import json
    import requests
    import json
    import uuid
    # import bottle
    from bottle import run, route, default_app, template, get, post, request, static_file, response, debug
except Exception as e:
    print(str(type(e).__name__) + ": " + str(e), file=sys.stderr)
    sys.exit(1)


def create_paramtweaks_file(reqdata):
    """
    :param reqdata:
    :return:
    """
    tweakfile = "tweakfile_" + str(uuid.uuid4().hex)
    tweakfileobj = open(tweakfile, "w")

    tweakscount = int(reqdata['tweakscount']['value'])

    dicttweakline = {}

    for i in range(tweakscount):
        # clear dict

        if reqdata['transmissibility']['ifchecked'] == True:
            transmissibilityfrom = int(reqdata['transmissibility']['fromval'])
            transmissibilityto = int(reqdata['transmissibility']['toval'])

            dicttweakline['transmissibility'] = transmissibilityfrom + i * ((transmissibilityto -
                                                                             transmissibilityfrom) / (tweakscount - 1))

        if reqdata['mean_incubation_duration_in_days']['ifchecked'] == True:
            mean_incubation_duration_in_daysfrom = \
                int(reqdata['mean_incubation_duration_in_days']['fromval'])
            mean_incubation_duration_in_daysto = \
                int(reqdata['mean_incubation_duration_in_days']['toval'])

            dicttweakline['mean_incubation_duration_in_days'] = mean_incubation_duration_in_daysfrom + \
                                                                i * ((mean_incubation_duration_in_daysto -
                                                                      mean_incubation_duration_in_daysfrom) / (
                                                                             tweakscount - 1))
        if reqdata['mean_infection_duration_in_days']['ifchecked'] == True:
            mean_infection_duration_in_daysfrom = \
                int(reqdata['mean_incubation_duration_in_days']['fromval'])
            mean_infection_duration_in_daysto = \
                int(reqdata['mean_incubation_duration_in_days']['toval'])

            dicttweakline['mean_infection_duration_in_days'] = mean_infection_duration_in_daysfrom + \
                                                               i * ((mean_infection_duration_in_daysto -
                                                                     mean_infection_duration_in_daysfrom) / (
                                                                            tweakscount - 1))

        if reqdata['mortality_ratio']['ifchecked'] == True:
            mortality_ratiofrom = int(reqdata['mortality_ratio']['fromval'])
            mortality_ratioto = int(reqdata['mortality_ratio']['toval'])

            dicttweakline['mortality_ratio'] = mortality_ratiofrom + \
                                               i * ((mortality_ratioto -
                                                     mortality_ratiofrom) / (
                                                            tweakscount - 1))

        if reqdata['update_trig_interval_in_hrs']['ifchecked'] == True:
            update_trig_interval_in_hrsfrom = int(reqdata['update_trig_interval_in_hrs']['fromval'])
            update_trig_interval_in_hrsto = int(reqdata['update_trig_interval_in_hrs']['toval'])
            dicttweakline['update_trig_interval_in_hrs'] = update_trig_interval_in_hrsfrom + \
                                                           i * ((update_trig_interval_in_hrsto -
                                                                 update_trig_interval_in_hrsfrom) / (
                                                                        tweakscount - 1))

        if reqdata['graph_type']['ifchecked'] == True:
            graphtype1 = reqdata['graph_type']['type1']
            graphtype1count = int(reqdata['graph_type']['type1count'])
            graphtype2 = reqdata['graph_type']['type2']
            graphtype2count = int(reqdata['graph_type']['type2count'])

            if i < graphtype1count:
                dicttweakline['graph_type'] = reqdata['graph_type']['type1']
            else:
                dicttweakline['graph_type'] = reqdata['graph_type']['type2']

        if reqdata['graph_params']['ifchecked'] == True:
            graph_param1from = reqdata['graph_params']['fromval'].split(',')[0]
            graph_param2from = reqdata['graph_params']['fromval'].split(',')[1]
            graph_param1to = reqdata['graph_params']['toval'].split(',')[0]
            graph_param2to = reqdata['graph_params']['toval'].split(',')[1]

            graph_param1 = graph_param1from + i * ((graph_param1to - graph_param1from) / (tweakscount - 1))
            graph_param2 = graph_param2from + i * ((graph_param2to - graph_param2from) / (tweakscount - 1))

            dicttweakline['graph_params'] = graph_param1 + "," + graph_param2

        if reqdata['diffusion_trig_interval_in_hrs']['ifchecked'] == True:
            diffusion_trig_interval_in_hrsfrom = int(reqdata['diffusion_trig_interval_in_hrs']['fromval'])
            diffusion_trig_interval_in_hrsto = int(reqdata['diffusion_trig_interval_in_hrs']['toval'])

            dicttweakline['diffusion_trig_interval_in_hrs'] = diffusion_trig_interval_in_hrsfrom + \
                                                              i * ((diffusion_trig_interval_in_hrsto -
                                                                    diffusion_trig_interval_in_hrsfrom) / (
                                                                           tweakscount - 1))

        if reqdata['avg_transport_speed']['ifchecked'] == True:
            avg_transport_speedfrom = int(reqdata['avg_transport_speed']['fromval'])
            avg_transport_speedto = int(reqdata['avg_transport_speed']['toval'])
            dicttweakline['avg_transport_speed'] = avg_transport_speedfrom + \
                                                   i * ((avg_transport_speedto -
                                                         avg_transport_speedfrom) / (
                                                                tweakscount - 1))

        if reqdata['max_diffusion_cnt']['ifchecked'] == True:
            max_diffusion_cntfrom = int(reqdata['max_diffusion_cnt']['fromval'])
            max_diffusion_cntto = int(reqdata['max_diffusion_cnt']['toval'])
            dicttweakline['max_diffusion_cnt'] = max_diffusion_cntfrom + \
                                                 i * ((max_diffusion_cntto -
                                                       max_diffusion_cntfrom) / (
                                                              tweakscount - 1))

        if reqdata['distmetrictype']['ifchecked'] == True:
            if listdistmetrics:
                listdistmetrics.clear()
            else:
                listdistmetrics = []

            if reqdata['distmetrictype']['ifmetricwasschecked'] == True:
                listdistmetrics.append('wass')
            if reqdata['distmetrictype']['ifmetricshajenchecked'] == True:
                listdistmetrics.append('shaj')

            dicttweakline['max_diffusion_cnt'] = listdistmetrics

        print("tweakline", dicttweakline)


@route('/<filename:re:.*\.css>')
def send_static(filename):
    return static_file(filename, root='css/')


@route('/<filename:re:.*\.js>')
def send_static(filename):
    return static_file(filename, root='js/')


@route('/<filename:re:.*\.png>')
def send_static(filename):
    return static_file(filename, root='static/')


@post('/hello')
def printhello():
    response.content_type = "application/json"
    # return "hello"
    hellodict = {"hello": "world"}
    return json.dumps(hellodict)


@post('/simulate')
def simulate():
    print("got post!")

    dictreq = json.loads(request.forms.get('data'))

    print(dictreq)
    create_paramtweaks_file(dictreq)

    return "ok simulating."


@route('/')
def home():
    with open('pandemicindex.html', 'r') as myfile:
        html_string = myfile.read()

    return template(html_string)  # , select_opts=get_select_opts_genre())


run(host='localhost', port=8080, reloader=True, debug=True)
