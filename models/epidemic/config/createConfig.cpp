#include <iostream>
#include <fstream>
#include <string>

/** Config Settings - edit to get the desired settings **/

/* Diffusion parameter */
// Graph type - Watts-Strogatz, Barabasi-Albert
#define GRAPH_TYPE                          "Watts-Strogatz"

// Watts-Strogatz graph parameters  - k (param 1), beta (param 2)
// Barabasi-Albert graph parameters - m (param 1),    a (param 2)
#define PARAM_1                             8
#define PARAM_2                             0.1

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

/* Regions */
#define NUM_REGIONS                         25000
#define MIN_NUM_LOCATIONS_PER_REGION        10
#define MAX_NUM_LOCATIONS_PER_REGION        10

/* Location */
#define MIN_NUM_PERSONS_PER_LOCATION        100
#define MAX_NUM_PERSONS_PER_LOCATION        100
#define MIN_TRAVEL_TIME_TO_HUB              50
#define MAX_TRAVEL_TIME_TO_HUB              400
#define MIN_LOCATION_DIFFUSION_INTERVAL     200
#define MAX_LOCATION_DIFFUSION_INTERVAL     500



/** Config creator - don't edit anything here **/

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

    // Write the diffusion parameters
    config_stream << GRAPH_TYPE << ",";
    config_stream << PARAM_1    << ",";
    config_stream << PARAM_2    << std::endl;

    // Write the disease parameters
    config_stream << TRANSMISSIBILITY                   << std::endl;
    config_stream << LATENT_DWELL_TIME                  << ",";
    config_stream << LATENT_INFECTIVITY                 << std::endl;
    config_stream << INCUBATING_DWELL_TIME              << ",";
    config_stream << INCUBATING_INFECTIVITY             << std::endl;
    config_stream << INFECTIOUS_DWELL_TIME              << ",";
    config_stream << INFECTIOUS_INFECTIVITY             << std::endl;
    config_stream << ASYMPT_DWELL_TIME                  << ",";
    config_stream << ASYMPT_INFECTIVITY                 << std::endl;
    config_stream << PROB_ULU                           << ",";
    config_stream << PROB_ULV                           << ",";
    config_stream << PROB_URV                           << ",";
    config_stream << PROB_UIV                           << ",";
    config_stream << PROB_UIU                           << std::endl;
    config_stream << LOCATION_STATE_REFRESH_INTERVAL    << std::endl;

    // Write the population parameters
    unsigned long pid = 1;
    config_stream << NUM_REGIONS << std::endl;
    for (unsigned int region_id = 0; region_id < NUM_REGIONS; region_id++) {
        config_stream << "r" << region_id << ",";

        int diff_locations = MAX_NUM_LOCATIONS_PER_REGION - MIN_NUM_LOCATIONS_PER_REGION;
        unsigned int num_locations = (diff_locations <= 0) ? MIN_NUM_LOCATIONS_PER_REGION : 
                                    (rand() % diff_locations + MIN_NUM_LOCATIONS_PER_REGION);
        config_stream << num_locations << std::endl;

        for (unsigned int location_id = 0; location_id < num_locations; location_id++) {
            config_stream << "l" << location_id << ",";

            int diff_travel_time = MAX_TRAVEL_TIME_TO_HUB - MIN_TRAVEL_TIME_TO_HUB;
            unsigned int travel_time = (diff_travel_time <= 0) ? MIN_TRAVEL_TIME_TO_HUB : 
                                        (rand() % diff_travel_time + MIN_TRAVEL_TIME_TO_HUB);
            config_stream << travel_time << ",";

            int diff_diffusion_interval = 
                            MAX_LOCATION_DIFFUSION_INTERVAL - MIN_LOCATION_DIFFUSION_INTERVAL;
            unsigned int diffusion_interval = 
                    (diff_diffusion_interval <= 0) ? MIN_LOCATION_DIFFUSION_INTERVAL : 
                        (rand() % diff_diffusion_interval + MIN_LOCATION_DIFFUSION_INTERVAL);
            config_stream << diffusion_interval << ",";

            int diff_persons = MAX_NUM_PERSONS_PER_LOCATION - MIN_NUM_PERSONS_PER_LOCATION;
            unsigned int num_persons = (diff_persons <= 0) ? MIN_NUM_PERSONS_PER_LOCATION : 
                                        (rand() % diff_persons + MIN_NUM_PERSONS_PER_LOCATION);
            config_stream << num_persons << std::endl;

            for (unsigned int person_id = 0; person_id < num_persons; person_id++) {
                config_stream << pid++ << ",";
                config_stream << "0." << rand() % 100 << ",";
                config_stream << rand() % 2 << ",";
                config_stream << rand() % 6 << std::endl;
            }
        }
    }

    // Close the config stream
    config_stream.close();

    return 0;
}

