# Synthetic Simulation Model #

Authors : Jingbin Yang, Sounak Gupta <br>
High Performance Computing Laboratory <br>
Dept of ECECS, PO Box 210030 <br>
Cincinnati, OH  45221--0030 <br>

## Introduction : ##

This model simulates the functionality of the other models. This synthetic model models
event or LPs starting from the internal. Calls new internal and external event with some delay
when name of both external event and internal event name are match. Otherwise handle the external.

## Details : ##

1. Total number of nodes to work on.

2. Choice of type of netwrok/graph to create the connectivities between events.

3. Select an external event (a node of LP) to handle.

4. Time to process external event.

5. Size range of the LPs.

6. The time delta of internal events.


## Configuration : ##

User can adjust the following parameters:

    1.  Number of nodes(entities) of the LPs. (Default: 100000)

    2.  Network, the network connectiveities between the LPs (Default: Watts-Strogatz)

    3.  Node selection distribution, using distribution to selete index of the LP.
        (Default: exponential)

    4.  Floating point operation counts, using floating point calculation to delay the run time.
        (Default: 10000)

    5.  State size, size range (in bytes) for LP state (DefaultL 100)

    6.  Event send distribution, event send time delta determent by the distribution
        (Default: geometric)

## References : ##

