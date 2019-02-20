# Script for building mutliple scaled sizes of warped2, ross, ns3, etc.
# Developed by Sean Kane
# HPCL
# 2/19/19

# Add the DES Metrics-focussed new python script that you are building into the scripts directory and also add a section in the README describing what it does and how to run it. Try to store the results inside logs.
# Advice from Sounak ^^^

import os
import sys
import json
import datetime
from collections import namedtuple # Use for defining each model

Model_Instance = namedtuple("Model_Instance", "name flags size runtime")

all_instances = []

all_instances.append(Model_Instance("pcs", "-x 1024 -y 1024", "100k", "500"))
all_instances.append(Model_Instance("pcs", "-x 316 -y 316", "100k", "2500"))
all_instances.append(Model_Instance("pcs", "-x 707 -y 707", "500k", "500"))
all_instances.append(Model_Instance("pcs", "-x 1024 -y 1024", "1m", "500"))
all_instances.append(Model_Instance("pcs", "-x 1414 -y 1414", "2m", "125"))
all_instances.append(Model_Instance("pcs", "-x 2236 -y 2236", "5m", "80"))
all_instances.append(Model_Instance("pcs", "-x 3162 -y 3162", "10m", "50"))

def writeJSON(file_name, json_data):
    with open(file_name, 'w') as outfile:
        print(json.dump(json_data, outfile, indent=4, sort_keys=False))

def defineModelSummaryJSON(model_name, capt_hist, file_name, format):
    m = {}
    m["simulator_name"] = "warped2-models"
    m["model_name"] = model_name
    m["original_capture_date"] = datetime.datetime.today().strftime("%d-%m-%y")
    m["capture_history"] = capt_hist
    m["total_lps"] = 0
    m["event_data"] = {}
    m["event_data"]["file_name"] = file_name
    m["event_data"]["format"] = format
    m["event_data"]["total_events"] = 0
    m["date_analyzed"] = ""
    return m
    
pwd = os.getcwd()
logsDir = os.path.join(pwd, "logs")

print("Building warped2-model mass simulations")

print("Arguments: %s" % sys.argv)

models = ["airport", "epidemic", "neuron", "pcs", "phold", "sandpile", "synthetic", "traffic", "volcano", "wildfire"]
sizes = ["10k", "50k", "100k", "500k", "1m", "2m", "5m", "10m"]
run_times = [12000, 2500, 500, 250, 100, 80, 50]

copy = "cp /home/kanesp/warped2-models/models/%s/%s_sim ./%s/%s_sim"

sim_type = "--simulation-type sequential"
stats_file = "--statistics-file stats-%s.csv"
stats_type = "--statistics-type csv"
sim_time = "--max-sim-time"
trace_file = "%s%s-trace.txt"

for m in models:
    # Create directory w/in logs for each model
    print("mkdir %s" % m)
#    os.system("mkdir %s" % m)

    # Copy executable into the above created directory
    print(copy % (m, m, m, m))
#    os.system(copy % (m, m, m))
    modelJSON = defineModelSummaryJSON(m, m, m, m) # CHANGE THIS
    writeJSON("test.json", modelJSON) # CHANGE THIS
    i = 0
    for 
