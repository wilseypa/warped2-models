# WILDFIRE : A WARPED simulation kernel application #

Authors : Caleb van Haaren, Mark Minutolo, Sounak Besu <br>
High Performance Computing Laboratory <br>
Dept of ECECS, PO Box 210030 <br>
Cincinnati, OH  45221--0030 <br>

## Introduction : ##

This model simulates the spread of wildfire in a forest. This wildfire spread models 
heat diffusion across cells or LPs starting from the origin of the fire. Cells ignite 
when its heat content reaches a certain ignition threshold (calculated using pixel 
color of that cell on the vegetation map).

The model is loosely-based on the Rothermel Wildfire Spread Model [1].

## Details : ##

1. Vegetation Map
    - Reads the pixel value and assigns burn attributes to the corresponding cell
    Pixel values will affect:
    + Ignition Threshold -  amount of heat needed by a cell to ignite; drier vegetation 
    have lower ignition threshold.
    + Peak Threshold - heat content of a cell beyond which it starts radiating heat to 
    neighboring LPs.

2. Choose a cluster of cells that would be the origin for this wildfire.

3. Any cell, on getting ignited, increases its own heat content till it reaches its peak 
threshold.

4. Once a cell reaches peak heat content, it begins to decay and lose heat by diffusing 
its heat to the eight adjacent cells.

5. A cell catches fire only on reaching its own ignition threshold by receiving enough 
heat radiating out from its neighboring cells. It remains UNBURNT otherwise.

6. A burning cell keeps on radiating heat to adjacent cells till its heat content goes 
below a burnout threshold.


## Configuration : ##

User can adjust the following parameters:

    1. Wildfire susceptibility map (Default: "test_vegetation_map.ppm")
       Note: This input picture must be a P6 PPM Binary file.

    2. Heat rate, amount of heat that an LP increases every timestamp in its "Growth" state
       (Default: 15)

    3. Radiation fraction, the percentage of an LP's heat content that is released every 
       five timestamps during the "Decay" state (Default: 0.05)

    4. Ambient heat, the amount of heat each cell has at the beginning of the simulation. 
       This can be used to simulate a hot day (Default: 0)

    4. Burnout threshold, the threshold of heat that a burning LP must reach in order to be
       considered "Burnt out" (Default: 20)

    5. Fire origin x, the x cooridinate of the start of the fire (Default: 700)

    6. Fire origin y, the y cooridinate of the start of the fire (Default: 600)


## References : ##

[1] Scott, Joe H. 2012. Introduction to Wildfire Behavior Modeling. National Interagency
Fuels, Fire, & Vegetation Technology Transfer. Available: www.niftt.gov.

[2] PPM Read/Write libraries were taken from https://github.com/sol-prog/Perlin_Noise
(released under GPL v3 license)

