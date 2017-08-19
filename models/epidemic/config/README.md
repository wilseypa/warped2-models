##EPIDEMIC: Configuration File Creator:

User can adjust the following parameters inside the macro section of 
[Config creator](createConfig.cpp) and build the config file using the 
following command:

    g++ --std=c++11 createConfig.cpp;./a.out <config_filename>


1. Diffusion parameters
    1. Graph Type can be either Watts-Strogatz or Barabasi-Albert
    2. Param 1 - default value is 8.
         Watts-Strogatz  : Number of links per node (K).
         Barabasi-Albert : Maximum degree of each node (m).
    3. Param 2 - default value is 0.1
         Watts-Strogatz  : Probability of link swaps (BETA). Small world networks have values <= 0.1.
         Barabasi-Albert : Probability Exponent (a).
2. Disease parameters
    1. Disease transmission probability. Default value is 0.12.
    2. Dwell time in Latent state. Default value is 200 timestamps.
    3. Infectivity probability of Latent state. Default value is 0.
    4. Dwell time in Incubating state. Default value is 100 timestamps.
    5. Infectivity probability of Incubating state. Default value is 0.3.
    6. Dwell time in Infectious state. Default value is 400 timestamps.
    7. Infectivity probability of Infectious state. Default value is 1.
    8. Dwell time in Asympt state. Default value is 200 timestamps.
    9. Infectivity probability of Asympt state. Default value is 0.5.
    10. Probability of disease progress (refer to Legend-1)
    11. Location state refresh interval - intervals at which the disease 
        spread inside a location is re-computed. Default value is 50 timestamps.
3. Region parameters (refer to Legend-2)
    1. Number of regions. Default value is 1000.
    2. Minimum number of locations per region. Default value is 10.
    3. Maximum number of locations per region. Default value is 10.
4. Location parameters (refer to Legend-3)
    1. Minimum population of a location. Default value is 100.
    2. Maximum population of a location. Default value is 100.
    3. Minimum travel time to Hub. Default value is 50 timestamps. Refer to Note.
    4. Maximum travel time to Hub. Default value is 400 timestamps. Refer to Note.
    5. Minimum location diffusion interval. Default value is 200 timestamps.
    6. Maximum location diffusion interval. Default value is 500 timestamps.

Note:
Travel Time<sub> location 1 => location 2</sub> = 
Travel Time<sub> location 1 => hub</sub> + 
Travel Time<sub> hub => location 2</sub>


###Legend-1 :

	Refer to Figure 2 of ref[2] for the disease model
	Transition Probabilities:
		prob_ulu : uninfected->latent (untreated). Default value is 0.2
		prob_ulv : uninfected->latent (vaccinated). Default value is 0.9.
		prob_urv : uninfected->recovered (vaccinated). Default value is 0.5.
		prob_uiv : uninfected->incubating (vaccinated). Default value is 0.1.
		prob_uiu : uninfected->incubating (untreated). Default value is 0.3.

###Legend-2 :
    Total number of simulation objects = 
            Number of regions * Number of locations per region
    Maximum and minimum location count values are used when location count per 
    region is randomly generated.

###Legend-3 :
    Maximum and minimum population counts are used when population per location 
    is randomly generated.

    Network graph design is diffusion model-specific. But to simplify distance 
    calculation  between  locations,  it  has  been  assumed that locations are 
    connected to each  other via a virtual central hub. This has been done to 
    reduce  complexity in management of the inter-location distances.

    Diffusion interval refers to the time interval after which a person is sent 
    out from a particular location. Minimum and maximum values are used to 
    randomly generate diffusion interval for each location.

