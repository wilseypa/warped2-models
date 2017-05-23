//Implementation of a Forest Fire Simulation

#include <cassert>
#include <random>
#include "forest.hpp"
#include "tclap/ValueArg.h"

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(AirportEvent)

std::vector<std::shared_ptr<warped::Event> > Forest::initializeLP() {

        std::vector<std::shared_ptr<warped::Event> > events;
            
        for (unsigned int i = 0; i < this->index_; i++){ //For all of the cells in the forest
                if(this->state.heat_content_ >= this->ignition_threshold){ //If heat content > ignition threshold
                        events.emplace_back(new ForestEvent {this->name_, IGNITION,ts_} // Then start an ignition event
                }
        }

        return events;
}


inline std::string Airport::lp_name(const unsigned int lp_index){

        return std::string("Forest_") + std::to_string(lp_index);
}



