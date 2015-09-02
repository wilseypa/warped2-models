// An implementation of Fujimoto's airport model
// Ported from the ROSS airport model (https://github.com/carothersc/ROSS/blob/master/ross/models/airport)

#include <cassert>
#include <random>
#include "airport.hpp"
#include "tclap/ValueArg.h"

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(AirportEvent)

std::vector<std::shared_ptr<warped::Event> > Airport::initializeLP() {

    // Register random number generator
    this->registerRNG<std::default_random_engine>(this->rng_);

    std::exponential_distribution<double> depart_expo(1.0/depart_mean_);
    std::vector<std::shared_ptr<warped::Event> > events;

    for (unsigned int i = 0; i < this->num_planes_; i++) {
        unsigned int departure = (unsigned int)std::ceil(depart_expo(*this->rng_));
        events.emplace_back(new AirportEvent {this->name_, DEPARTURE, departure});
    }
    return events;
}

inline std::string Airport::lp_name(const unsigned int lp_index) {

    return std::string("Airport_") + std::to_string(lp_index);
}

std::vector<std::shared_ptr<warped::Event> > Airport::receiveEvent(const warped::Event& event) {

    std::vector<std::shared_ptr<warped::Event> > response_events;
    auto received_event = static_cast<const AirportEvent&>(event);

    std::exponential_distribution<double> depart_expo(1.0/depart_mean_);
    std::exponential_distribution<double> arrive_expo(1.0/arrive_mean_);

    switch (received_event.type_) {

        case DEPARTURE: {
            this->state_.planes_grounded_--;
            this->state_.departures_++;
            // Schedule an arrival at a random airport
            unsigned int arrival_time = received_event.ts_ + (unsigned int)std::ceil(arrive_expo(*this->rng_));
            response_events.emplace_back(new AirportEvent { 
                                                    random_move(), ARRIVAL, arrival_time });
            break;
        }

        case ARRIVAL: {
            this->state_.arrivals_++;
            this->state_.planes_grounded_++;
            // Schedule a departure
            unsigned int departure_time = received_event.ts_ + (unsigned int)std::ceil(depart_expo(*this->rng_));
            response_events.emplace_back(new AirportEvent { this->name_, DEPARTURE, 
                                                                            departure_time });
            break;
        }
    }
    return response_events;
}

std::string Airport::compute_move(direction_t direction) {

    unsigned int new_x = 0, new_y = 0;
    unsigned int current_y = index_ / num_airports_x_;
    unsigned int current_x = index_ % num_airports_x_;

    switch (direction) {

        case LEFT: {
            new_x = (current_x + num_airports_x_ - 1) % num_airports_x_;
            new_y = current_y;
        } break;

        case RIGHT: {
            new_x = (current_x + 1) % num_airports_x_;
            new_y = current_y;
        } break;

        case DOWN: {
            new_x = current_x;
            new_y = (current_y + num_airports_y_ - 1) % num_airports_y_;
        } break;

        case UP: {
            new_x = current_x;
            new_y = (current_y + 1) % num_airports_y_;
        } break;

        default: {
            std::cerr << "Invalid move direction " << direction << std::endl;
            assert(0);
        }
    }

    return lp_name(new_x + new_y * num_airports_x_);
}

std::string Airport::random_move() {

    std::uniform_int_distribution<unsigned int> rand_direction(0,3);
    return this->compute_move((direction_t)rand_direction(*this->rng_));
}

int main(int argc, const char** argv) {

    unsigned int num_airports_x     = 50;
    unsigned int num_airports_y     = 50;
    unsigned int mean_ground_time   = 50;
    unsigned int mean_flight_time   = 200;
    unsigned int num_planes         = 50;

    TCLAP::ValueArg<unsigned int> num_airports_x_arg("x", "num-airports-x", "Width of airport grid",
                                                            false, num_airports_x, "unsigned int");
    TCLAP::ValueArg<unsigned int> num_airports_y_arg("y", "num-airports-y", "Height of airport grid",
                                                            false, num_airports_y, "unsigned int");
    TCLAP::ValueArg<unsigned int> mean_ground_time_arg("g", "ground-time", 
                "Mean time of planes waiting to depart", false, mean_ground_time, "unsigned int");
    TCLAP::ValueArg<unsigned int> mean_flight_time_arg("f", "flight-time", "Mean flight time",
                                                        false, mean_flight_time, "unsigned int");
    TCLAP::ValueArg<unsigned int> num_planes_arg("p", "num-planes", "Number of planes per airport",
                                                                false, num_planes, "unsigned int");

    std::vector<TCLAP::Arg*> args = {&num_airports_x_arg, &num_airports_y_arg, &mean_ground_time_arg, 
                                                                &mean_flight_time_arg, &num_planes_arg};

    warped::Simulation airport_sim {"Airport Simulation", argc, argv, args};

    num_airports_x      = num_airports_x_arg.getValue();
    num_airports_y      = num_airports_y_arg.getValue();
    mean_ground_time    = mean_ground_time_arg.getValue();
    mean_flight_time    = mean_flight_time_arg.getValue();
    num_planes          = num_planes_arg.getValue();

    std::vector<Airport> lps;

    for (unsigned int i = 0; i < num_airports_x*num_airports_y; i++) {
        std::string name = Airport::lp_name(i);
        lps.emplace_back(name, num_airports_x, num_airports_y, num_planes, 
                                                mean_flight_time, mean_ground_time, i);
    }

    std::vector<warped::LogicalProcess*> lp_pointers;
    for (auto& lp : lps) {
        lp_pointers.push_back(&lp);
    }

    airport_sim.simulate(lp_pointers);

    unsigned int arrivals = 0;
    unsigned int departures = 0;
    unsigned int planes_grounded = 0;
    for (auto& lp : lps) {
        arrivals += lp.state_.arrivals_;
        departures += lp.state_.departures_;
        planes_grounded += lp.state_.planes_grounded_;
    }
    std::cout << departures << " total departures" << std::endl;
    std::cout << arrivals << " total arrivals" << std::endl;
    std::cout << planes_grounded << " of "  << num_airports_x*num_airports_y*num_planes 
                                            << " planes grounded" << std::endl;

    return 0;
}

