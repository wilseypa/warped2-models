#ifndef DIFFUSION_NETWORK_HPP
#define DIFFUSION_NETWORK_HPP

#include "memory.hpp"
#include "Person.hpp"
#include <random>

class DiffusionNetwork {
public:

    DiffusionNetwork(   unsigned int travel_time_to_hub,
                        std::shared_ptr<std::default_random_engine> rng )
        : travel_time_to_hub_(travel_time_to_hub), rng_(rng) {}

    std::string pickLocation() {

        std::string location_name = "";
        unsigned int location_num = travel_time_chart_.size();

        if(location_num) {
            std::uniform_int_distribution<int> distribution(0, location_num-1);
            auto location_id = (unsigned int) distribution(*rng_);
            auto map_iter = travel_time_chart_.begin();
            for(unsigned int count = 0; count < location_id; count++) {
                map_iter++;
            }
            location_name = map_iter->first;
        }
        return location_name;
    }

    unsigned int travelTimeToLocation(std::string location_name) {

        return (travel_time_chart_[location_name] + travel_time_to_hub_);
    }

    unsigned int pickPerson(unsigned int person_count) {

        unsigned int person_id = 0;
        if(person_count) {
            std::uniform_int_distribution<int> distribution(0, person_count-1);
            person_id = (unsigned int) distribution(*rng_); 
        }
        return person_id;
    }

    void populateTravelChart(std::map<std::string, unsigned int> travel_chart) {

        travel_time_chart_ = travel_chart;
    }

private:
    unsigned int travel_time_to_hub_;
    std::shared_ptr<std::default_random_engine> rng_;
    std::map<std::string, unsigned int> travel_time_chart_;
};

#endif
