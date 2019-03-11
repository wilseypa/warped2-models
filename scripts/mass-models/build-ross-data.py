# Script for developing multiple scaled sizes of the ROSS simulation engine
# Sean Kane
# HPCL
# 3/7/19

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

def findROSSInstallation():
    # Find installation location given an argument
    if len(sys.argv) == 1:
        print("No ROSS location given")
        print("Use as python3 build-ross-data.py </location/to/ROSS>")
        quit()
    else:
        return sys.argv[1]

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
                
f = open("summary-file-ROSS.csv", "a")
f.write("Model, Total LPs, Total Events, Events/LP, Runtime, Output File Size, New Runtime")
f.write("\n")
f.close()

pwd = os.getcwd()
os.system("mkdir logs-ross")
logsDir = os.path.join(pwd, "logs-ross")

# Find the ROSS installation
ROSSDir = findROSSInstallation()
ROSSDir = os.path.join(ROSSDir, "models/ROSS-Models")
print(ROSSDir)

print("Building ROSS mass simulations")

models = ["traffic", "torus", "airport", "phold-delta", "wifi"]

copy = "cp " + ROSSDir + "/%s/%s ./logs-ross/%s/%s"

iterations = openConfig()
executables = []

findEXE("./")
findEXE(ROSSDir + "/traffic/")
#findEXE("/home/sean/ross-build/models/ROSS-Models/airport")
quit()

for m in models:
    # Create directory for each model
    os.system("mkdir logs-ross/%s" % m)
    # Copy executable to file
    os.system(copy % (m, m, m, m))
