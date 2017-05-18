#FOREST: A WARPED simulation kernel application

Authors:Sounak Gupta,Caleb van Haaren, Mark Minutolo <br>
Experimental Computing Laboratory <br>
Dept of ECECS, PO Box 210030 <br>
Cincinnati, OH  45221--0030 <br>

##Introduction :

This simulation model simulates the spread of fire in a forest. A new model of the traditional
forest fire mode was developed using heat diffusion, with the idea that each LP or cell on a 
map will ignite only when its heat content reaches a certain threshold determined by pixel in 
the vegetetation that correlates to the respective LP.

1. Vegetation maps
    - Reading the pixel value and assigning its attributes to an LP
    Pixel values will affect
    + Ignition threshold -  the amount of heat needed by an LP to ignite
    + Peak threshold - the amount of heat the LP needs to reach before it starts to decay

2. Starting the fire/simulation at a specific LP
3. After a cell is ignited it increases in heat content unti it reaches a certain threshold
4. Then the LP sets off an internal Peak event signifying that the fire is fully developed
5. Now the LP is increasing the heat content in the 8 cells adjacent to it
6. The LP continues to send out Radiation events to other cells until an internal event is
triggered signifying that the fire has burned out.
4. Conditional - asking if the adjacent cells have reached enough heat to ignite
    + No - Then the state of that LP remains in - UNBURNT
    + Yes - Then an internal Ignition event is triggered
      



##Configuration :

##References :

