// Ported from the ROSS traffic model 
// https://github.com/carothersc/ROSS-Models/blob/master/traffic/

#include <vector>
#include <memory>
#include <random>

#include "warped.hpp"
#include "traffic.hpp"

#include "MLCG.h"

#include "tclap/ValueArg.h"


WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(TrafficEvent)

std::vector<std::shared_ptr<warped::Event> > Intersection::createInitialEvents() {

    std::vector<std::shared_ptr<warped::Event> > events;
    return events;
}

inline std::string Intersection::object_name(const unsigned int object_index) {

    return std::string("Intersection_") + std::to_string(object_index);
}

std::vector<std::shared_ptr<warped::Event> > 
                Intersection::receiveEvent(const warped::Event& event) {

    std::vector<std::shared_ptr<warped::Event> > response_events;
    auto received_event = static_cast<const TrafficEvent&>(event);

    switch (received_event.type_) {

        case ARRIVAL: {

        } break;

        case DEPARTURE: {

        } break;

            
        case DIRECTION_SELECT: {

        } break;

        default: {}
    }
    return response_events;
}

std::string Intersection::compute_move(direction_t direction) {

    switch (direction) {
        case WEST: {
        } break;

        case EAST: {
        } break;

        case SOUTH: {
        } break;

        case NORTH: {
        } break;

        default: {
            std::cerr << "Invalid move direction " << direction << std::endl;
            assert(0);
        }
    }
    return object_name(0);
}

std::string Intersection::random_move() {

    std::default_random_engine gen;
    std::uniform_int_distribution<unsigned int> rand_direction(0,3);
    return this->compute_move((direction_t)rand_direction(gen));
}

int main(int argc, const char** argv) {

    std::cout << argc << argv[1];
    return 0;
}

