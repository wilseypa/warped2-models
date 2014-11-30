#ifndef DIFFUSION_NETWORK_HPP
#define DIFFUSION_NETWORK_HPP

#include "memory.hpp"
#include "Person.hpp"
#include "RandomNumGenerator.hpp"

class DiffusionNetwork {
public:

    DiffusionNetwork(unsigned int seed, unsigned int travel_time_to_hub) 
        : travel_time_to_hub_(travel_time_to_hub) {
    
        rand_num_gen_ = warped::make_unique<RandomNumGenerator>(seed);
    }

    std::string pickLocation() {

        std::string location_name = "";
        unsigned int location_num = travel_time_chart_.size();

        if(location_num) {
            unsigned int location_id = rand_num_gen_->generateRandNum(location_num);
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

    std::shared_ptr<Person> pickPerson(
            std::map<unsigned int, std::shared_ptr<Person>> population) {

        std::shared_ptr<Person> person = nullptr;
        unsigned int person_count = population.size();
        if(person_count) {
            unsigned int person_id = rand_num_gen_->generateRandNum(person_count);   
            person = population[person_id];
        }
        return person;
    }

    void populateTravelChart(std::map<std::string, unsigned int> travel_chart) {

        travel_time_chart_ = travel_chart;
    }

private:
    std::unique_ptr<RandomNumGenerator> rand_num_gen_;
    unsigned int travel_time_to_hub_;
    std::map<std::string, unsigned int> travel_time_chart_;
};

#endif
