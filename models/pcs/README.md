# PCS: A WARPED simulation kernel application

Experimental Computing Laboratory <br>
Dept of ECECS, PO Box 210030 <br>
Cincinnati, OH  45221--0030 <br>

## Introduction :

This simulation model simulates the circuit-switched network on a
cellular grid. Each cell is a simulation object which has a fixed
place on a cellular grid. Portables can move from one cell to the
other - this is simulated via event diffusion from one simulation
object to the other on the cellular grid. [1]

Poisson distribution has been used to model the event arrival. [2]

## Configuration :

User can adjust the following parameters :

1. Width of the cellular grid (Default: 100)
2. Height of the cellular grid (Default: 100)
3. Number of channels per cell (Default: 15)
4. Mean call interval (Default: 200)
5. Mean call duration (Default: 50)
6. Mean move interval (Default: 100)
7. Number of portacles initially at each cell (Default: 50)

## References :

[1] Lin, Y.-B., P. A. Fishwick, and S. Member. 1996. Asynchronous
parallel discrete event simulation. IEEE Transactions on Systems,
Man and Cybernetics 26 (4): 397–412.
[http://ieeexplore.ieee.org/xpl/articleDetails.jsp?arnumber=508819]

[2] Eytan Modiano. Introduction to Queueing Theory
[http://web.mit.edu/modiano/www/6.263/lec5-6.pdf]
