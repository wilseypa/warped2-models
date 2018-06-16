## Simulate and Plot a Simulation Model ##

__WARNING:__ This is just a quick start guide without detailed explanations. Please
read the scripts if you need more details.

### Generate data ###

* Read the [configuration](config/local.config). There are many options available
and most options are self-explanatory:

    * Few model configurations available. One can choose one of them by setting
    some flag values in the script. More model configurations can be added too.

    * Comment out the type of tests that are not required.

* All generated data is recorded inside [logs](logs/)

* After editing the coniguration, start simulation using __./configSimulate.sh config/local.config__

### Generate Plots ###

* After collecting the simulation data, move all generated csv files to a directory

    * For effective use of this script, use model name and lp count as directory name.
    __e.g. logs/traffic_10k/__

    * This directory path will be used for generating the plots

* To generate plots, run __./buildPlots.sh <directory>__

* If modifications or additions are needed, read the __buildPlots.sh__ in order to
understand which plot scripts to edit. All plot scripts are heavily configurable.


__Good Luck !!!__

