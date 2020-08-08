#ifndef DIFFUSION_HPP
#define DIFFUSION_HPP

#include "memory.hpp"
#include "corona.hpp"
#include <random>

class Diffusion {
public:

    Diffusion(  unsigned int travel_time_to_hub,
                unsigned int max_diffusion_cnt,
                std::shared_ptr<std::default_random_engine> rng )
        :   travel_time_to_hub_(travel_time_to_hub),
            max_diffusion_cnt_(max_diffusion_cnt),
            rng_(rng) {}

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

    unsigned int diffusionCount(unsigned int population_size) {
        std::uniform_int_distribution<int> distribution(0,
                std::min(max_diffusion_cnt_, population_size-1));
        return distribution(*rng_); 
    }

    void populateTravelChart(std::map<std::string, unsigned int> travel_chart) {
        travel_time_chart_ = travel_chart;
    }

private:
    unsigned int travel_time_to_hub_    = 0;
    unsigned int max_diffusion_cnt_     = 0;
    std::shared_ptr<std::default_random_engine> rng_;
    std::map<std::string, unsigned int> travel_time_chart_;
};

#endif
