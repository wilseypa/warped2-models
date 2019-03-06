## Scripts for Building Warped2 Models in Mass


### Overview
The build-model-data.py file will run multiple configurations for the warped2 models. Each of the
models can be seen in the source code. The resulting data will be stored in the logs directory, with
each of the different models having their own directory. In this directory one can find the various
sizes, along with the modelSummary.json file for each iteration. The modelSummary.json file is used
by the desMetrics project for sampling, analysis, and visualizing.

### Running Simulations
To run the file, I suggest using the following command:
```
python3 build-model-data.py > <trace-file.txt> 2>&1 &
```
This runs the trace in the background and will create a trace file (<trace-file.txt>) which tracks standard output and standard error for review if something were to operate incorrectly. 

### Adding More Simulations
To add more simulations, simply append your simulation to the bottom of the local.config file. The order of the file is:
```
Simulation Type (Name), Flags, Size (ie. # of LPs), Runtime
```
The script will build a directory within logs for each of the simulations and within each of those,
a new directory for each of the sizes of the simulation. The outputs of the simulation are a trace
file (name + size + "-trace.txt"), the statistics for that simulation run (stats-<name>_<size>.csv),
and modelSummary.json. These directories are then suited for use with the desMetrics projects.


### Flags - Work in Progress

--clean
* Removes the executables from each of the directories

--<model>
* Only runs through iterations of that simulation type


### Adding Configurations
Add to the local.config file to add more simulation runs. The formatting of the file is indicated in the first line.