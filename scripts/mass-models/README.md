## Scripts for Building Warped2 Models in Mass


### Overview
The build-model-data.py file will run multiple configurations for the warped2 models. Each of the
models can be seen in the source code. The resulting data will be stored in the logs directory, with
each of the different models having their own directory. In this directory one can find the various
sizes, along with the modelSummary.json file for each iteration. The modelSummary.json file is used
by the desMetrics project for sampling, analysis, and visualizing.

### Flags

--clean
* Removes the executables from each of the directories

--<model>
* Only runs through iterations of that simulation type


### Adding Configurations
Simply add to the struct list `Model_Instance` for each configuration that you would like. The
Model_Instance struct takes 4 arguments: name, flags, size, and runtime. The rest is taken care of
by the script. 