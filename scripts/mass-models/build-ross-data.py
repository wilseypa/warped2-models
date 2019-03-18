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
    with open("/home/kanesp/warped2-gprof/warped2-models/scripts/mass-models/new-local-ross.config", "a") as f:
        f.write(s)
        f.write("\n")
        f.close()
                
f = open("summary-file-ROSS.csv", "a")
f.write("Model, Total LPs, Total Events, Events/LP, Runtime, Output File Size, New Runtime")
f.write("\n")
f.close()

pwd = os.getcwd()
os.system("mkdir logs-ross")
logsDir = os.path.join(pwd, "logs-ross")

# Find the ROSS installation
ROSSDir = "/home/kanesp/ross-build/models/ROSS-Models/"

print("Building ROSS mass simulations")

modeldirs = ["torus", "wifi", "phold-delta", "srw", "suspend-test", "airport"]
executables = ["torus", "wifi", "dphold", "srw", "suspend", "airport"]

#copy = "cp " + ROSSDir + "/%s/%s ./logs-ross/%s/%s"
copy = "cp " + ROSSDir + "/%s/%s ./logs-ross/"

iterations = openConfig()


for i in range(len(modeldirs)):
    # Create directory for each model
#    os.system("mkdir logs-ross/%s" % modeldirs[i])
    # Copy executable to file
    os.system(copy % (modeldirs[i], executables[i]))#, modeldirs[i], executables[i]))

print(os.system("ls logs-ross"))

# Build models
goalSize = 3 * 1e9
create_stats_file = "--event-trace=1"
stats_path = "--stats-path=./"
stats_prefix = "stats-%s.csv"
trace_file = "%s%s-trace.txt"

for i in iterations:
    print(datetime.datetime.now())
    name = i[0]
    flags = i[1]
    size = i[2]
    runtime = i[3]
    testdir = os.path.join(os.getcwd(), "logs-ross/%s" % (name.strip() + size.strip()))
    os.system("mkdir %s" % testdir)

    os.chdir(testdir)
    print("Current DIR: " + os.getcwd())

    profileString = "CPUPROFILE=%s%sProfile.out " % (name.strip(), size.strip())

    sim_string = profileString + " ../%s " % name
    sim_string += " %s " % create_stats_file
    sim_string += " %s " % flags
#    sim_string += " %s " % (stats_prefix % (name.strip()+size.strip()))
#    sim_string += " %s " % stats_file
#    sim_string += " %s " % stats_path
    sim_string += " >> %s" % (trace_file % (name.strip(), size.strip())).strip()
    print(sim_string)
    os.system(sim_string)

    print("Current DIR: " + os.getcwd())
    os.chdir("stats-output")

    fileSize = 0
    try:
        fileSize = sum(os.path.getsize(f) for f in os.listdir('.') if os.path.isfile(f))
    except:
        pass
    print("FILESIZE: " + str(fileSize))
    newRuntime = goalSize / fileSize * int(runtime)
    
    if fileSize != 0:
        newConfig(name, flags, size, newRuntime)
    else:
        pass
    
    print(sim_string)
    os.chdir(pwd)
    
    
