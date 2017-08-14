# Abelian Sandpile Simulation Model #

Authors : John Musgrave, Sounak Gupta <br>
High Performance Computing Laboratory <br>
Dept of ECECS, PO Box 210030 <br>
Cincinnati, OH  45221--0030 <br>

## Introduction : ##

The Abelian Sandpile model is a cellular automata problem where elements in a lattice are considered to be sites with a height value.  If the height is greater than a given threshold, the site will then topple, subtracting a value from the unstable site, and adding to each of the connected neighbors.  This will continue until all sites in the system are stable.  The model is considered Abelian because the final configuration is a function of the initial state, and is not affected by the order in which the operations occur.

## Details : ##

State interactions for variable z, when the value of z exceeds a given threshold, or critical value, k [1]:

    1.  z(x,y) -> z(x,y) - 4
    2.  z(x+1,y) -> z(x+1,y) + 1
    3.  z(x-1,y) -> z(x-1,y) + 1
    4.  z(x,y+1) -> z(x,y+1) + 1
    5.  z(x,y-1) -> z(x,y-1) + 1


## Configuration : ##

User can adjust the following parameters:

    1. Dimension of NxN Lattice (Default: 1000)

Outputs the resulting configuration in an image in .ppm format.


## References : ##

[1] Bak, P.; Tang, C.; Wiesenfeld, K. (1987). "Self-organized criticality: an explanation of 1/ƒ noise". Physical Review Letters. 59 (4): 381–384.
