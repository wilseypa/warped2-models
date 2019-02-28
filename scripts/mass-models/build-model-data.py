# Script for building mutliple scaled sizes of warped2, ross, ns3, etc.
# Developed by Sean Kane
# HPCL
# 2/19/19

# Add the DES Metrics-focussed new python script that you are building into the scripts directory and also add a section in the README describing what it does and how to run it. Try to store the results inside logs.
# Advice from Sounak ^^^

# local.config will store each of the simulation model desired run time flags

import os
import sys
import json
import datetime

def readTraceFile(file_name):
    data = []
    with open(file_name, 'r') as f:
        data = f.readlines()
    events = 0
    LPs = 0
    for d in data:
        if "LP count" in d:
            LPs = int(d.split(":")[-1])
        if "Events processed" in d:
            events = int(d.split(":")[-1])
    return events, LPs

def openConfig():
    # Removes the first line, which is a comment on the format for adding more
    models = []
    with open("local.config") as f:
        data = f.readlines()
    data = data[1:]
    models = [d.split(',') for d in data]
    for m in models:
        m[-1] = m[-1].replace("\n", "")
    return models

def writeJSON(file_name, json_data):
    with open(file_name, 'w') as outfile:
        print(json.dump(json_data, outfile, indent=4, sort_keys=False))

def defineModelSummaryJSON(model_name, capt_hist, file_name, format, events, LPs):
    m = {}
    m["simulator_name"] = "warped2-models"
    m["model_name"] = model_name
    m["original_capture_date"] = datetime.datetime.today().strftime("%d-%m-%y")
    m["capture_history"] = capt_hist
    m["total_lps"] = LPs
    m["event_data"] = {}
    m["event_data"]["file_name"] = file_name
    m["event_data"]["format"] = format
    m["event_data"]["total_events"] = events
    m["date_analyzed"] = ""
    return m
    
pwd = os.getcwd()
os.system("mkdir logs")
logsDir = os.path.join(pwd, "logs")

print("Building warped2-model mass simulations")

print("Arguments: %s" % sys.argv)

models = ["airport", "epidemic", "neuron", "pcs", "phold", "sandpile", "synthetic", "traffic", "volcano", "wildfire"]

copy = "cp ../../models/%s/%s_sim ./logs/%s/%s_sim"

sim_type = "--simulation-type sequential "
stats_file = "--statistics-file stats-%s.csv "
stats_type = "--statistics-type csv "
sim_time = "--max-sim-time "
trace_file = "%s%s-trace.txt "

iterations = openConfig()

for m in models:
    # Create directory w/in logs for each model
    os.system("mkdir logs/%s" % m)
    # Copy executable to file
    os.system(copy % (m, m, m, m))


for i in iterations:
    name = i[0]
    flags = i[1]
    size = i[2]
    runtime = i[3]
    os.chdir(os.path.join(os.getcwd(), "logs/%s" % name))
    newDir = name.strip() + "-" + size.strip()
    os.system("mkdir %s" % newDir)

    os.system("cp %s_sim %s" % (name, newDir))
    os.chdir(newDir)
    profileString = "CPUPROFILE=%s-%sProfile.out " % (name.strip(), size.strip())
    # Run Simulation
    sim_string = profileString + " ./%s_sim " % name
    sim_string += flags
    sim_string += " %s %s " % (sim_time, runtime)
    sim_string += sim_type
    sim_string += " --statistics-type csv "
    sim_string += stats_file % (name + "_" + size.strip())
    sim_string += " >> " + trace_file % (name.strip(), size.strip())
    os.system(sim_string)
    print(sim_string)
    events, LPs = readTraceFile("%s%s-trace.txt" % (name.strip(), size.strip()))
    j = defineModelSummaryJSON(name, name, "stats-" + name + "_" + size.strip() + ".csv", ".csv", events, LPs)
    writeJSON("modelSummary.json", j)

    os.chdir(pwd)

os.system("rm test.json")

    
