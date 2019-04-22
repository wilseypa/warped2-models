# Script for developing multiple scaled sizes of the ROSS simulation engine
# Sean Kane
# HPCL
# 4/16/19

# local.config will store each of the simulation model desired run time flags

import os
import sys
import json
import datetime
import stat

def openConfig():
    # Removes the first line, which is a comment on the format for adding more models
    models = []
    with open("local-ross.config") as f:
        data = f.readlines()
    data = data[1:]
    models = [d.split(',') for d in data]
    for m in models:
        m[-1] = m[-1].replace("\n", "")
    return models


def findEXE(loc):
    executable = stat.S_IEXEC | stat.S_IXGRP | stat.S_IXOTH
    for filename in os.listdir(loc): #'.'):
        if os.path.isfile(filename):
            st = os.stat(filename)
            mode = st.st_mode
            if mode & executable:
                return filename
    print("No EXE found in %s" % loc)
    return -1

def newConfig(name, flags, size, runtime):
#    f = open("/home/kanesp/warped2-gprof/warped2-models/scripts/mass-models/new-local-ross.config")
    s = [name.strip(), flags.strip(), size.strip(), int(runtime)]
    s = str(s).replace("[", "")
    s = str(s).replace("]", "")
    s = str(s).replace("'", "")
    with open("/testData/simulators/ross-master/new-local-ross.config", "a") as f:
        f.write(s)
        f.write("\n")
        f.close()

def writeJSON(file_name, json_data):
    with open(file_name, 'w') as outfile:
        print(json.dump(json_data, outfile, indent=4, sort_keys=False))
    print("JSON Written to: " + os.getcwd())
        
def defineModelSummaryJSON(model_name, capt_hist, file_name, format, events, LPs, run_command):
    m = {}
    m["simulator_name"] = str("ROSS")
    m["model_name"] = str(model_name)
    m["original_capture_date"] = str(datetime.datetime.today().strftime("%d-%m-%y"))
    m["capture_history"] = [str(capt_hist)]
    m["total_lps"] = int(LPs)
    m["event_data"] = {}
    m["event_data"]["file_name"] = str(file_name)
    m["event_data"]["format"] = str(format)
    m["event_data"]["total_events"] = int(events)
    m["event_data"]["format"] = ["sLP", "rLP", "sTS", "rTS", "et"]
    m["date_analyzed"] = str("")
    m["run_command"] = str(run_command)
    return m

def writeSummary(model, LPs, events, runtime, outputFileSize):
    s = [model, LPs, events, float(events)/float(LPs), int(runtime), outputFileSize, 5.0 * float(runtime)/float(outputFileSize)]
    s = str(s).replace("[", "")
    s = str(s).replace("]", "")
    with open("/testData/simulators/ross-master/summary-file-ROSS.csv", "a") as f:
        f.write(s)
        f.write("\n")
        f.close()

def readTrace(file_name):
    data = []
    with open(file_name, 'r') as f:
        data = f.readlines()
    events = 0
    LPs = 0
    for d in data:
        if "Total Events Processed" in d:
            events = int(d.split(" ")[-1])
        if "Total LP" in d:
            LPs = int(d.split(" ")[-1])
    return events, LPs, data[0]

f = open("/testData/simulators/ross-master/summary-file-ROSS.csv", "a")
f.write("Model, Total LPs, Total Events, Events/LP, Runtime, Output File Size, New Runtime")
f.write("\n")
f.close()

pwd = os.getcwd()
os.system("mkdir logs-ross")
logsDir = os.path.join(pwd, "logs-ross")

# Find the ROSS installation
ROSSDir = "/testData/simulators/ross-master/ross-build/models/ROSS-Models"

print("Building ROSS mass simulations")

modeldirs = ["airport",  "disksim", "dragonfly", "dphold", "Intersection", "olsr",\
             "olsr-j", "pcs", "qhold", "qhold_fp", "raid", \
             "rng", "srw", "suspend", "torus", "wifi"             
]
executables = ["airport", "disksim", "dragonfly", "dphold", "Intersection", "olsr", "olsr-j", \
               "pcs", "qhold", "qhold_fp", "raid", "rng", "srw", "suspend", \
               "torus", "wifi"
               ]

#copy = "cp " + ROSSDir + "/%s/%s ./logs-ross/%s/%s"
copy = "cp " + ROSSDir + "/%s/%s ./logs-ross/"

iterations = openConfig()


for i in range(len(modeldirs)):
    # Create directory for each model
#    os.system("mkdir logs-ross/%s" % modeldirs[i])
# /testData/simulators/ross-master/ross-build/models/ROSS-Models/wifi/wifi

    # Copy executable to file
    os.system(copy % (modeldirs[i], executables[i]))#, modeldirs[i], executables[i]))

print(os.system("ls logs-ross"))

# Build models
goalSize = 10 * 1e9 * 0.67
synch = "--sync=1"
create_stats_file = "--event-trace=1"
stats_path = "--stats-path=./"
stats_prefix = "stats-%s.csv"
end = "--end=%s"
trace_file = "trace.txt"

for i in iterations:
    print(datetime.datetime.now())
    name = i[0]
    flags = i[1]
    size = i[2]
    runtime = i[3]
    print(name)
    testdir = os.path.join(os.getcwd(), "logs-ross/%s" % name.upper()) #(name.strip() + size.strip()))
    os.system("mkdir %s" % testdir)

    os.chdir(testdir)
    

    profileString = "CPUPROFILE=%s%sProfile.out " % (name.strip(), size.strip())

    sim_string = profileString + " ../%s " % name
    sim_string += " %s " % synch
    sim_string += " %s " % create_stats_file
    sim_string += " %s " % flags
    sim_string += " %s " % (end % runtime)
    sim_string += " >> %s" % trace_file
    print(sim_string)
    os.system(sim_string)
    #try:
    os.chdir("stats-output")
    
    events, LPs, run_command = readTrace("../trace.txt")
    j = defineModelSummaryJSON(name, "", "ross-stats-evtrace.csv", "CSV", events, LPs, run_command)
    writeJSON("modelSummary.json", j)
    print("JSON File created...")
    
    fileSize = 0
    newRuntime = runtime
 #       try:
 #           fileSize = sum(os.path.getsize(f) for f in os.listdir('.') if os.path.isfile(f))
 #           newRuntime = goalSize / fileSize * int(runtime)
 #       except:
 #           pass

#        newConfig(name, flags, size, newRuntime)

    convert = "/testData/simulators/ross-master/ross-binary-reader/reader --filename=ross-stats-evtrace.bin --filetype=2"

    print("Converting binary to CSV...")
    os.system(convert)
    
    print(sim_string)
    print("\n\n")
    os.chdir(pwd)


    
