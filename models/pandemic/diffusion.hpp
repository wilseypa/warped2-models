#ifndef DIFFUSION_HPP
#define DIFFUSION_HPP

#include "memory.hpp"
#include "pandemic.hpp"
#include <random>
#include <tuple>

#define AVG_TRANSPORT_SPEED 10

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

    double calc_haversine_distance(double lat1, double long1, double lat2, double long2) {

        const double earth_radius = 6372.8;
        const double pi_val = 3.14159265358979323846;
        
        double lat1_rad = lat1 * pi_val / 180.0;
        double long1_rad = long1 * pi_val / 180.0;
        double lat2_rad = lat2 * pi_val / 180.0;
        double long2_rad = long2 * pi_val / 180.0;

        double diff_lat = lat2_rad - lat1_rad;
        double diff_long = long2_rad - long1_rad;

        double temp_val = std::asin(std::sqrt(
                                        std::sin(diff_lat / 2) * std::sin(diff_lat / 2) + std::cos(lat1_rad)
                                        * std::cos(lat2_rad) * std::sin(diff_long / 2) * std::sin(diff_long / 2)
                                        ));

        return 2.0 * earth_radius * temp_val;
    }

    unsigned int travelTime(std::string curr_loc, std::string target_loc) {

        auto curr_it = CONFIG->locations_.find(curr_loc);
        auto target_it = CONFIG->locations_.find(target_loc);

        assert(curr_it != CONFIG->locations_.end());        
        assert(target_it != CONFIG->locations_.end());

        double distance = calc_haversine_distance(std::get<3>(*target_it), std::get<4>(*target_it),
                                                  std::get<3>(*curr_it),
                                                  std::get<4>(*curr_it)) / AVG_TRANSPORT_SPEED;

        return (int)distance;
    }

private:
    unsigned int max_population_size_ = 0;
    std::shared_ptr<std::default_random_engine> rng_;
};

#endif
