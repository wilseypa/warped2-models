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
    m["simulator_name"] = str("warped2-models")
    m["model_name"] = str(model_name)
    m["original_capture_date"] = str(datetime.datetime.today().strftime("%d-%m-%y"))
    m["capture_history"] = [str(capt_hist)]
    m["total_lps"] = int(LPs)
    m["event_data"] = {}
    m["event_data"]["file_name"] = str(file_name)
    m["event_data"]["format"] = str(format)
    m["event_data"]["total_events"] = int(events)
    m["date_analyzed"] = str("")
    m["event_data"]["format"] = ["sLP", "rLP", "sTS", "rTS", "sz"]
    return m

def writeSummary(model, LPs, events, runtime, outputFileSize):
    s = [model, LPs, events, float(events)/float(LPs), int(runtime), outputFileSize, 5.0 * float(runtime)/float(outputFileSize)]
    s = str(s).replace("[", "")
    s = str(s).replace("]", "")
    with open("../../../summary-file.csv", "a") as f:
        f.write(s)
        f.write("\n")
        f.close()

f = open("summary-file.csv", "a")
f.write("Model, Total LPs, Total Events, Events/LP, Runtime, Output File Size, New Runtime")
f.write("\n")
f.close()

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
    print(i)
    print(datetime.datetime.now())
    name = i[0]
    flags = i[1]
    size = i[2]
    runtime = i[3]
    os.chdir(os.path.join(os.getcwd(), "logs/%s" % name))
    newDir = name.strip() + "-" + size.strip()
    os.system("mkdir %s" % newDir)

    os.system("cp %s_sim %s" % (name, newDir))
    os.chdir(newDir)
    profileString = "CPUPROFILE=%s%sProfile.out " % (name.strip(), size.strip())
    print("CPU PROFILE FILE NAME: %s" % profileString)
    print(f"\n\nSimulation: {name}\t Size: {size}\n")
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

    model = "warped2-" + name.strip() + "-" + size.strip()
    fileSize = 1
    try:
        fileSize = os.path.getsize("stats-" + name + "_" + size.strip() + ".csv")/1e9
    except:
        pass
    if LPs == 0:
        LPs = 1
    if fileSize == 0:
        fileSize = 1
    if events == 0:
        events = 1

    writeSummary(model, LPs, events, runtime, fileSize)
    os.chdir(pwd)
