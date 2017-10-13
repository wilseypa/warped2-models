# Synthetic Simulation Model #

Authors : Jingbin Yang, Sounak Gupta <br>
High Performance Computing Laboratory <br>
Dept of ECECS, PO Box 210030 <br>
Cincinnati, OH  45221--0030 <br>


## Introduction : ##

This simulation model allows the user to create synthetic approximation of different
categories of simulation models. A configurable set of simulation parameters allows
the user to manipulate the model's underlying network structure as well as rate and
distribution of event flow.


## Features : ##

Individual or combined effect(s) of the following features on simulation can be
studied using different configurations of this model :

1. Total number of nodes (entites) which collectively form the simulation space.

2. Structure of network connectivity between nodes.

3. Rate of events sent from a node to its adjacent nodes.

4. Distribution of events sent from a node to its adjacent nodes.

5. Time to process an event.

6. Size of the LP state.


## Configuration : ##

User can adjust the following parameters:

    1.  Number of nodes(entities). (Default: 100000)

    2.  Network, the network connectiveities between the LPs (Default: Watts-Strogatz)

    3.  Node selection distribution, using distribution to selete index of the LP.
        (Default: exponential)

    4.  Floating point operation counts, using floating point calculation to delay the run time.
        (Default: 10000)

    5.  State size, size of LP state (DefaultL 100)

    6.  Event send distribution, event send time delta determent by the distribution
        (Default: geometric)


## References : ##

