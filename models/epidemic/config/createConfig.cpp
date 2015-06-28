#include <iostream>
#include <fstream>
#include <string>

/** Config Settings **/

/* Diffusion parameter */
#define DIFFUSION_SEED                      101

// Watts-Strogatz model parameters
#define K                                   8
#define BETA                                0.1

/* Disease parameters */
#define TRANSMISSIBILITY                    0.12
#define LATENT_DWELL_TIME                   200
#define LATENT_INFECTIVITY                  0
#define INCUBATING_DWELL_TIME               100
#define INCUBATING_INFECTIVITY              0.3
#define INFECTIOUS_DWELL_TIME               400
#define INFECTIOUS_INFECTIVITY              1.0
#define ASYMPT_DWELL_TIME                   200
#define ASYMPT_INFECTIVITY                  0.5
#define PROB_ULU                            0.2
#define PROB_ULV                            0.9
#define PROB_URV                            0.5
#define PROB_UIV                            0.1
#define PROB_UIU                            0.3
#define LOCATION_STATE_REFRESH_INTERVAL     50
#define DISEASE_SEED                        90

/* Regions */
#define NUM_REGIONS                         1000
#define MIN_NUM_LOCATIONS_PER_REGION        500
#define MAX_NUM_LOCATIONS_PER_REGION        1500

/* Location */
#define MIN_NUM_PERSONS_PER_LOCATION        500
#define MAX_NUM_PERSONS_PER_LOCATION        1000
#define MIN_TRAVEL_TIME_TO_HUB              50
#define MAX_TRAVEL_TIME_TO_HUB              400
#define MIN_LOCATION_DIFFUSION_INTERVAL     200
#define MAX_LOCATION_DIFFUSION_INTERVAL     500


int main( int argc, char *argv[] ) {

    // Check the number of arguments
    if (argc != 2) {
        std::cerr << "Invalid number of arguments" << std::endl;
        return 0;
    }

    // Read the config filename
    std::string config_filename(argv[1]);

    // Create the config stream
    std::ofstream config_stream;
    config_stream.open(config_filename, std::ios::out);
    if (!config_stream.is_open()) {
        std::cerr << "Could not create the metrics file." << std::endl;
        return 0;
    }

    config_stream << NUM_REGIONS << std::endl;

    for (unsigned int region_id = 0; region_id < NUM_REGIONS; region_id++) {
        config_stream << "region_" << region_id << std::endl;
        unsigned int num_locations = 
            rand() % (MAX_NUM_LOCATIONS_PER_REGION - MIN_NUM_LOCATIONS_PER_REGION) + 
                                                            MIN_NUM_LOCATIONS_PER_REGION;
        config_stream << num_locations << std::endl;

        for (unsigned int location_id = 0; location_id < num_locations; location_id++) {
            config_stream << "location_" << location_id << std::endl;
        }
    }

    // Close the config stream
    config_stream.close();

    return 0;
}

