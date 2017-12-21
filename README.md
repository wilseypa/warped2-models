# WAPRED Simulation Models
Simulation models for the WARPED Parallel & Distributed Discrete Simulation Library

# Building

The WARPED models are built with a C++11 compiler (see [here](http://lektiondestages.blogspot.de/2013/05/installing-and-switching-gccg-versions.html) for instructions about upgrading and switching GCC versions if you have an old version of GCC that does not support the required features.).  

These models require the WARPED library to be built and installed. The library can be found with build instructions [here](https://github.com/wilseypa/warped2). 

To build from the git repository, first clone a local copy.

	git clone https://github.com/wilseypa/warped2-models ~/warped2-models

You can run the Autotools build without any options. These models are not typically installed, so a prefix is optional. However, if the WARPED library is not install at the default location of `/usr`, then the location must be specified with the `--with-warped` configure option. For example, if the WARPED library is installed in `$HOME/lib/warped2`, then the following command would be used to build the models:

	autoreconf -i && ./configure --with-warped=$HOME/lib/warped2 && make

To run the GUI for Warped2-Models, you have to install python packages - Tkinter and Tix:

    sudo apt-get install python-tk
    sudo apt-get install tix-dev

Runs python GUI:

    ./gui.py

# License
The WARPED Models code in this repository is licensed under the MIT license, unless otherwise specified. The full text of the MIT license can be found in the `LICENSE.txt` file. 
