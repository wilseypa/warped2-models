#TRAFFIC: A WARPED simulation kernel application

Experimental Computing Laboratory <br>
Dept of ECECS, PO Box 210030 <br>
Cincinnati, OH  45221--0030 <br>

##Introduction :

This simulation model simulates the traffic movement in a busy city. 
Each intersection is a simulation object which has a fixed place on 
a traffic grid. Vehicles can move from one intersection to the other 
- this is simulated via event diffusion from one simulation object 
to the other on the traffic grid. [1]

##Configuration :

User can adjust the following parameters :

1. Width of the traffic grid (Default: 100)
2. Height of the traffic grid (Default: 100)
3. Number of cars per intersection (Default: 25)
4. Mean interval (Default: 400)

##References :

[1] Ported from the ROSS traffic model 
(https://github.com/carothersc/ROSS-Models/tree/master/traffic)

