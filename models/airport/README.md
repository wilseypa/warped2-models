# AIRPORT: A WARPED simulation kernel application

Experimental Computing Laboratory <br>
Dept of ECECS, PO Box 210030 <br>
Cincinnati, OH  45221--0030 <br>

## Introduction :

This simulation model simulates the flight management at different airports.
Each airport is a simulation object which has a fixed place on a virtual grid.
Planes can move from one airport to the other - this is simulated via event
diffusion from one simulation object to the other on the virtual grid. [1]

## Configuration :

User can adjust the following parameters :

1. Width of the airport grid (Default: 50)
2. Height of the airport grid (Default: 50)
3. Mean time planes have to wait before departure (Default: 50)
4. Mean flight time (Default: 200)
5. Number of planes initially at each airport (Default: 50)

## References :

[1] An implementation of Fujimoto's airport model. Ported from the ROSS airport
model (https://github.com/carothersc/ROSS-Models/tree/master/airport)
