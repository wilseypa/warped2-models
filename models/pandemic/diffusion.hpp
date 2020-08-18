#ifndef DIFFUSION_HPP
#define DIFFUSION_HPP

#include "memory.hpp"
#include "corona.hpp"
#include <random>

#include AVG_TRANSPORT_SPEED 10

class Diffusion {
public:

    Diffusion( unsigned int max_population_size, std::shared_ptr<std::default_random_engine> rng )
        :   max_population_size_(max_population_size),
            rng_(rng) {}

    std::string pickLocation(std::string curr_loc) {
        unsigned int num_locations = CONFIG->locations_.size();
        if(!num_locations) return "";
        std::uniform_int_distribution<int> distribution(0, num_locations-1);
        auto location_id = (unsigned int) distribution(*rng_);
        auto it = std::next(std::begin(CONFIG->locations_), location_id);
        return it->first;
    }

    unsigned int populationSize() {
        std::uniform_int_distribution<int> distribution(0,
                std::min(max_population_size_, CONFIG->locations_.size()-1));
        return distribution(*rng_);
    }

    unsigned int travelTime(std::string curr_loc, std::string target_loc) {
        auto curr_it = CONFIG->locations_.find(curr_loc);
        assert(curr_it != CONFIG->locations_.end());
        auto target_it = CONFIG->locations_.find(target_loc);
        assert(target_it != CONFIG->locations_.end());

        unsigned int distance = 0;
        //TODO: Use curr_it->second and target_it->second to calculate distance.
        return distance / AVG_TRANSPORT_SPEED;
    }

private:
    unsigned int max_population_size_ = 0;
    std::shared_ptr<std::default_random_engine> rng_;
};

#endif
