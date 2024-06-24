#!/bin/bash

# we want this script to fail on any error
set -e
cd ../ #go back one directory
export CUR_LOC=`pwd`
echo "Building warped2 and warped2-models at: " $CUR_LOC

#
# warped2
#
ARGS=$1

echo "....warped2"
date
cd warped2
autoreconf -i

#
# configure with openmpi 
#
export MPI_LOC=/usr/lib/x86_64-linux-gnu/openmpi/include

# build warped2
./configure --with-mpi-includedir=$MPI_LOC --prefix=$CUR_LOC/warped2/local CXXFLAGS='-g -O3' $1
make -j 8 install
date
# return to original subdirectory
cd $CUR_LOC

#
# warped2-models
#

echo "....warped2-models"
date

cd warped2-models
autoreconf -i

# build warped2-models
./configure --with-warped=$CUR_LOC/warped2/local CXXFLAGS='-g -O3' CXX=mpicxx
make -j 8
date

# return to original subdirectory
cd $CUR_LOC

