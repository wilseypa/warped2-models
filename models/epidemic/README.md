#EPIDEMIC: A WARPED simulation kernel application

Author: Sounak Gupta <br>
Experimental Computing Laboratory <br>
Dept of ECECS, PO Box 210030 <br>
Cincinnati, OH  45221--0030 <br>

##Introduction :

This simulation model simulates the epidemic outbreak phenomena. A new 
parallel discrete event formulation of the reaction-diffusion model was 
developed.  It is motivated by the one reported in [2]. The epidemic is 
modeled using a combination of reaction  and diffusion  processes.  The 
reaction  is defined as  a result of  inter-entity  interactions, (e.g. 
influenza or physical proximity). The mobility of the entities leads to 
the  dynamic chances for interactions (e.g., watching movie in a cinema 
and  other co-located  activities). Geographical diffusion  of entities 
provides  chances for  interactions  among  different  sample  sets  of 
entities.

This model contains four parts [1]:

+ A set of interacting entities.

+ A network graph that shows the interaction structure of entities. 
  Currently 'FullyConnected' and 'WattsStrogatz' models are supported.

+ A reaction process, called within-host progression, which models the 
  progress of the disease in an individual entity.

+ A diffusion process, called between-host transmission, which models 
  the transmission of the disease among individual entities.

NOTE: Network graph design is diffusion model-specific. But to simplify 
distance  calculation  between  locations,  it  has  been  assumed that 
locations are  connected to each  other via a virtual central hub. This 
has been done to reduce  complexity in management of the inter-location 
distances.

Travel time from location 1 to location 2 = <br>
                     Travel time from location 1 to central hub + 
                     Travel time from central hub to location 2
<br>

###Important Information :

The  config  file can  be auto-generated  with  random  region  details 
using the 'createXmlConfig.py'  script. Type ***python createXmlConfig.py 
'config file name'*** to create the config file.


##Configuration File :

The model generator program lets the user adjust the following parameters
before writing out the model:

1. Diffusion  : model, seed, model parameters
    a. Model Parameters for Watts-Strogatz: K, Beta

2. Disease    : transmissiblity, latent dwell time, latent infectivity, 
                incubating dwell time, incubating infectivity, 
                infectious dwell time, infectious infectivity, 
                asympt dwell time, asympt infectivity, 
                prob_ul_u, prob_ul_v, prob_ur_v, prob_ui_v, prob_ui_u, 
                location state refresh interval, seed

3. Population : For each region => For each location 
                (Travel time to central hub, Diffusion trigger interval, 
                For each person => Susceptibility, Is Vaccinated, 
                Infection State)

###Legend :

	Refer to Figure 2 of ref[2] for the disease model
	Transition Probabilities:
		prob_ulu : uninfected->latent (untreated)
		prob_ulv : uninfected->latent (vaccinated)
		prob_urv : uninfected->recovered (vaccinated)
		prob_uiv : uninfected->incubating (vaccinated)
		prob_uiu : uninfected->incubating (untreated)

	Location state refresh interval:
		Intervals at which the disease spread at any location is re-computed


##References :

[1] Kalyan S Perumalla and Sudip K Seal. 2012. Discrete event modeling 
and massively parallel execution of epidemic outbreak phenomena. 
Simulation 88, 7 (July 2012), 768-783.

[2] Christopher L. Barrett, Keith R. Bisset, Stephen G. Eubank, Xizhou Feng, 
and Madhav V. Marathe. 2008. EpiSimdemics: an efficient algorithm for 
simulating the spread of infectious disease over large realistic social networks. 
In Proceedings of the 2008 ACM/IEEE conference on Supercomputing (SC '08). 
IEEE Press, Piscataway, NJ, USA, , Article 37 , 12 pages.

[3] D.J. Watts and S.H. Strogatz. Collective dynamics of 'small-world' networks. 
Nature 393, 440-442 (1998).


