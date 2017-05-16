//Implementation of a Forest Fire Simulation

#include <cassert>
#include <random>
#include "forest.hpp"
#include "tclap/ValueArg.h"

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(AirportEvent)

std::vector<std::shared_ptr<warped::Event> > Forest::initializeLP() {

        // Register random number generator
        this->registerRNG<std::default_random_engine>(this->rng_);

        std::exponential_distribution<double> depart_expo(1.0/depart_mean_);
        std::vector<std::shared_ptr<warped::Event> > events;
}

std::vector<std::shared_ptr<warped::Event> > Forest::receiveEvent(const warped::Event& event) {

        std::vector<std::shared_ptr<warped::Event> > response_events;
        auto received_event = static_cast<const ForestEvent&>(event);

       // std::exponential_distribution<double> depart_expo(1.0/depart_mean_);
       // std::exponential_distribution<double> arrive_expo(1.0/arrive_mean_);

        switch (received_event.type_) {

               case SPREADING : {
                     
                      break;
               }
                       
               case IGNITION : {
                     
                      break;
               }
        }
                            return response_events;
}
