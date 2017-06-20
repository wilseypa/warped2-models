#FOREST: A WARPED simulation kernel application

Authors:Sounak Gupta, Caleb van Haaren, Mark Minutolo <br>
Experimental Computing Laboratory <br>
Dept of ECECS, PO Box 210030 <br>
Cincinnati, OH  45221--0030 <br>

##Introduction :

This simulation model simulates the spread of wildfire in a forest. This new model of the traditional
forest fire mode was developed using heat diffusion, with the idea that each LP or cell on a 
map will ignite only when its heat content reaches a certain threshold determined by pixel color in 
the susceptibility map that correlates to the respective LP.

1. Wildfire Susceptibility maps
    - Reading the pixel value and assigning its attributes to an LP
    Pixel values will affect:
    + Ignition threshold -  the amount of heat needed by an LP to ignite, the lower the 
      susceptibility the higher the threshold
    + Peak threshold - the amount of heat the LP needs to reach before it starts to decay

2. Starting the fire/simulation at a cluster of specific LPs(To ensure the fire spreads)
3. After a cell is ignited it increases its own heat content until it reaches its peak threshold
4. Then the LP sets off an internal Peak event signifying that the fire is fully developed
5. Once the fire is fully developed it begins to decay and lose heat content by diffusing its heat
   with Radiation events to the eight adjacent cells.

   Conditional - asking if the adjacent cells have recieved enough heat from the burning cells 
   to ignite
    + No - Then the state of that LP remains in - UNBURNT
    + Yes - Then an internal Ignition event is triggered

6. The LP continues to send out Radiation events to other cells until an internal event is
triggered signifying that the fire has burned out.


##Configuration :

##References :

