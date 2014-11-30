#!/usr/bin/python

# Configuration file generator for epidemic model

import xml.etree.ElementTree as ET
import xml.dom.minidom as minidom
import sys
import random

###### Settings go here ######

# Diffusion parameters
# model - "FullyConnected" or "WattsStrogatz"
MODEL                           = "WattsStrogatz"
DIFFUSION_SEED                  = 101

# Watts-Strogatz model parameters
K                               = 8
BETA                            = 0.1

# Disease parameters
TRANSMISSIBILITY                = 0.12
LATENT_DWELL_TIME               = 200
LATENT_INFECTIVITY              = 0
INCUBATING_DWELL_TIME           = 100
INCUBATING_INFECTIVITY          = 0.3
INFECTIOUS_DWELL_TIME           = 400
INFECTIOUS_INFECTIVITY          = 1.0
ASYMPT_DWELL_TIME               = 200
ASYMPT_INFECTIVITY              = 0.5
PROB_UL_U                       = 0.2
PROB_UL_V                       = 0.9
PROB_UR_V                       = 0.5
PROB_UI_V                       = 0.1
PROB_UI_U                       = 0.3
LOCATION_STATE_REFRESH_INTERVAL = 50
DISEASE_SEED                    = 90

# Data capture
IS_DATA_CAPTURE_NEEDED          = "no"
DATA_CAPTURE_FILENAME           = "epidemic_data.csv"

# Regions
NUMBER_OF_REGIONS               = 50
MIN_NUM_OF_LOCS_PER_REGION      = 100
MAX_NUM_OF_LOCS_PER_REGION      = 120

# Location
MIN_NUM_OF_PERSONS_PER_LOC      = 10
MAX_NUM_OF_PERSONS_PER_LOC      = 30
MIN_TRAVEL_TIME_TO_CENTRAL_HUB  = 50
MAX_TRAVEL_TIME_TO_CENTRAL_HUB  = 400
MIN_DIFFUSION_TRIG_INTERVAL     = 500
MAX_DIFFUSION_TRIG_INTERVAL     = 1500

# Person
SUSCEPTIBILITY_PRECISION        = 100.0
INFECTION_STATES                = ["uninfected", "latent", "incubating", "infectious", "asympt", "recovered"]
VACCINATION_STATUS              = ["yes", "no"]


###### Don't edit below here ######

def main():
    outFileName = sys.argv[1]

    root = ET.Element("epidemic_configuration")

    doc = ET.SubElement(root, "diffusion")
    field = ET.SubElement(doc, "model")
    field.text = str(MODEL)
    field = ET.SubElement(doc, "seed")
    field.text = str(DIFFUSION_SEED)

    if MODEL == "WattsStrogatz":
        doc = ET.SubElement(doc, "watts_strogatz")
        field = ET.SubElement(doc, "k")
        field.text = str(K)
        field = ET.SubElement(doc, "beta")
        field.text = str(BETA)


    doc = ET.SubElement(root, "disease")
    field = ET.SubElement(doc, "transmissibility")
    field.text = str(TRANSMISSIBILITY)
    field = ET.SubElement(doc, "latent_dwell_time")
    field.text = str(LATENT_DWELL_TIME)
    field = ET.SubElement(doc, "latent_infectivity")
    field.text = str(LATENT_INFECTIVITY)
    field = ET.SubElement(doc, "incubating_dwell_time")
    field.text = str(INFECTIOUS_DWELL_TIME)
    field = ET.SubElement(doc, "incubating_infectivity")
    field.text = str(INCUBATING_INFECTIVITY)
    field = ET.SubElement(doc, "infectious_dwell_time")
    field.text = str(INFECTIOUS_DWELL_TIME)
    field = ET.SubElement(doc, "infectious_infectivity")
    field.text = str(INFECTIOUS_INFECTIVITY)
    field = ET.SubElement(doc, "asympt_dwell_time")
    field.text = str(ASYMPT_DWELL_TIME)
    field = ET.SubElement(doc, "asympt_infectivity")
    field.text = str(ASYMPT_INFECTIVITY)
    field = ET.SubElement(doc, "prob_ul_u")
    field.text = str(PROB_UL_U)
    field = ET.SubElement(doc, "prob_ul_v")
    field.text = str(PROB_UL_V)
    field = ET.SubElement(doc, "prob_ur_v")
    field.text = str(PROB_UR_V)
    field = ET.SubElement(doc, "prob_ui_v")
    field.text = str(PROB_UI_V)
    field = ET.SubElement(doc, "prob_ui_u")
    field.text = str(PROB_UI_U)
    field = ET.SubElement(doc, "location_state_refresh_interval")
    field.text = str(LOCATION_STATE_REFRESH_INTERVAL)
    field = ET.SubElement(doc, "seed")
    field.text = str(DISEASE_SEED)

    doc = ET.SubElement(root, "population")

    pid = 1
    for regionNum in range(NUMBER_OF_REGIONS):
        regionName = "region_"+str(regionNum)
        field = ET.SubElement(doc, regionName)
        numberOfLocations = random.randrange(MIN_NUM_OF_LOCS_PER_REGION, MAX_NUM_OF_LOCS_PER_REGION+1)
        for locationNum in range(numberOfLocations):
            locName = "location_"+str(locationNum)
            field1 = ET.SubElement(field, locName)
            numberOfPersons = random.randrange(MIN_NUM_OF_PERSONS_PER_LOC, MAX_NUM_OF_PERSONS_PER_LOC+1)
            field2 = ET.SubElement(field1, "travel_time_to_central_hub")
            field2.text = str(random.randrange(MIN_TRAVEL_TIME_TO_CENTRAL_HUB, MAX_TRAVEL_TIME_TO_CENTRAL_HUB+1))
            field2 = ET.SubElement(field1, "diffusion_trigger_interval")
            field2.text = str(random.randrange(MIN_DIFFUSION_TRIG_INTERVAL, MAX_DIFFUSION_TRIG_INTERVAL+1))
            for numPerson in range(numberOfPersons):
                personName = "person_"+str(pid)
                field2 = ET.SubElement(field1, personName)
                pid += 1
                field3 = ET.SubElement(field2, "susceptibility")
                field3.text = str(random.randrange(0, SUSCEPTIBILITY_PRECISION)/SUSCEPTIBILITY_PRECISION)
                field3 = ET.SubElement(field2, "is_vaccinated")
                field3.text = str(VACCINATION_STATUS[random.randrange(0, len(VACCINATION_STATUS))])
                field3 = ET.SubElement(field2, "infection_state")
                field3.text = str(INFECTION_STATES[random.randrange(0, len(INFECTION_STATES))])

    with open(outFileName, 'w') as f:
        s = ET.tostring(root)
        minidom.parseString(s).writexml(f, addindent='  ' * 4, newl='\n')

if __name__ == "__main__":
    main()

