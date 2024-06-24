// Ported from the ROSS traffic model 
// https://github.com/carothersc/ROSS-Models/blob/master/traffic/

#include <cassert>
#include <random>
#include "traffic.hpp"
#include "tclap/ValueArg.h"

#define MAX_CARS_ON_ROAD 5

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(TrafficEvent)

std::vector<std::shared_ptr<warped::Event> > Intersection::initializeLP() {

    this->registerRNG(this->rng_);

    std::exponential_distribution<double> interval_expo(1.0/this->mean_interval_);

    std::uniform_int_distribution<unsigned int> rand_car_direction(0,11);
    auto car_arrival = (car_direction_t) rand_car_direction(*this->rng_);
    auto car_current_lane = car_arrival;

    std::uniform_int_distribution<int> rand_x(-99,100);
    std::uniform_int_distribution<int> rand_y(-99,100);

    std::vector<std::shared_ptr<warped::Event> > events;
    for (unsigned int i = 0; i < this->num_cars_; i++) {
        events.emplace_back(new TrafficEvent {
                lp_name(this->index_), ARRIVAL, rand_x(*this->rng_), rand_y(*this->rng_), 
                car_arrival, car_current_lane, (unsigned int) std::ceil(interval_expo(*this->rng_))});
    }
    return events;
}

inline std::string Intersection::lp_name(const unsigned int lp_index) {

    return std::string("Intersection_") + std::to_string(lp_index);
}

std::vector<std::shared_ptr<warped::Event> > 
                Intersection::receiveEvent(const warped::Event& event) {

    std::vector<std::shared_ptr<warped::Event> > events;
    auto traffic_event = static_cast<const TrafficEvent&>(event);
    std::exponential_distribution<double> interval_expo(1.0/this->mean_interval_);

    switch (traffic_event.type_) {

        case ARRIVAL: {

            if (!traffic_event.x_to_go_ && !traffic_event.y_to_go_) {
                state_.total_cars_finished_++;
                break;
            }
            car_direction_t arrival_from = traffic_event.current_lane_;
            // std::cerr<< traffic_event.timestamp()<<" "<<traffic_event.receiverName()<<" "<<traffic_event.x_to_go_<<" "<<traffic_event.y_to_go_<<"\n";
            state_.total_cars_arrived_++;

            switch (traffic_event.current_lane_) {

                case WEST_LEFT: {
                    state_.num_in_east_left_++;
                    arrival_from = EAST_LEFT;
                } break;

                case WEST_STRAIGHT: {
                    state_.num_in_east_straight_++;
                    arrival_from = EAST_STRAIGHT;
                } break;

                case WEST_RIGHT: {
                    state_.num_in_east_right_++;
                    arrival_from = EAST_RIGHT;
                } break;

                case EAST_LEFT: {
                    state_.num_in_west_left_++;
                    arrival_from = WEST_LEFT;
                } break;

                case EAST_STRAIGHT: {
                    state_.num_in_west_straight_++;
                    arrival_from = WEST_STRAIGHT;
                } break;

                case EAST_RIGHT: {
                    state_.num_in_west_right_++;
                    arrival_from = WEST_RIGHT;
                } break;

                case NORTH_LEFT: {
                    state_.num_in_south_left_++;
                    arrival_from = SOUTH_LEFT;
                } break;

                case NORTH_STRAIGHT: {
                    state_.num_in_south_straight_++;
                    arrival_from = SOUTH_STRAIGHT;
                } break;

                case NORTH_RIGHT: {
                    state_.num_in_south_right_++;
                    arrival_from = SOUTH_RIGHT;
                } break;

                case SOUTH_LEFT: {
                    state_.num_in_north_left_++;
                    arrival_from = NORTH_LEFT;
                } break;

                case SOUTH_STRAIGHT: {
                    state_.num_in_north_straight_++;
                    arrival_from = NORTH_STRAIGHT;
                } break;

                case SOUTH_RIGHT: {
                    state_.num_in_north_right_++;
                    arrival_from = NORTH_RIGHT;
                } break;
            }

            auto timestamp = traffic_event.ts_ + (unsigned int) std::ceil(interval_expo(*this->rng_));
            events.emplace_back(new TrafficEvent {
                            this->name_, DIRECTION_SELECT, 
                            traffic_event.x_to_go_, traffic_event.y_to_go_, 
                            traffic_event.arrived_from_, arrival_from, timestamp});
        } break;


        case DEPARTURE: {

            direction_t departure_direction = NORTH;
            switch (traffic_event.current_lane_) {

                case WEST_LEFT: {
                    state_.num_out_west_left_--;
                    departure_direction = WEST;
                } break;

                case WEST_STRAIGHT: {
                    state_.num_out_west_straight_--;
                    departure_direction = WEST;
                } break;

                case WEST_RIGHT: {
                    state_.num_out_west_right_--;
                    departure_direction = WEST;
                } break;

                case EAST_LEFT: {
                    state_.num_out_east_left_--;
                    departure_direction = EAST;
                } break;

                case EAST_STRAIGHT: {
                    state_.num_out_east_straight_--;
                    departure_direction = EAST;
                } break;

                case EAST_RIGHT: {
                    state_.num_out_east_right_--;
                    departure_direction = EAST;
                } break;

                case NORTH_LEFT: {
                    state_.num_out_north_left_--;
                    departure_direction = NORTH;
                } break;

                case NORTH_STRAIGHT: {
                    state_.num_out_north_straight_--;
                    departure_direction = NORTH;
                } break;

                case NORTH_RIGHT: {
                    state_.num_out_north_right_--;
                    departure_direction = NORTH;
                } break;

                case SOUTH_LEFT: {
                    state_.num_out_south_left_--;
                    departure_direction = SOUTH;
                } break;

                case SOUTH_STRAIGHT: {
                    state_.num_out_south_straight_--;
                    departure_direction = SOUTH;
                } break;

                case SOUTH_RIGHT: {
                    state_.num_out_south_right_--;
                    departure_direction = SOUTH;
                } break;
            }

            auto timestamp = traffic_event.ts_ + (unsigned int) std::ceil(interval_expo(*this->rng_));
            events.emplace_back(new TrafficEvent {
                            this->compute_move(departure_direction), ARRIVAL, 
                            traffic_event.x_to_go_, traffic_event.y_to_go_, 
                            traffic_event.arrived_from_, traffic_event.current_lane_, timestamp});
        } break;


        case DIRECTION_SELECT: {

            int x_to_go = traffic_event.x_to_go_;
            int y_to_go = traffic_event.y_to_go_;
            car_direction_t current_lane = traffic_event.current_lane_;

            switch (traffic_event.current_lane_) {

                case EAST_LEFT: {
                    state_.num_in_east_left_--;
                    if ((y_to_go < 0) && 
                                (state_.num_out_south_straight_ < MAX_CARS_ON_ROAD)) {
                        current_lane = SOUTH_STRAIGHT;
                        state_.num_out_south_straight_++;
                        y_to_go++;

                    } else if ((x_to_go < 0) && 
                                (state_.num_out_south_right_ < MAX_CARS_ON_ROAD)){
                        current_lane = SOUTH_RIGHT;
                        state_.num_out_south_right_++;
                        x_to_go++;

                    } else if ((x_to_go > 0) && 
                                (state_.num_out_south_left_ < MAX_CARS_ON_ROAD)){
                        current_lane = SOUTH_LEFT;
                        state_.num_out_south_left_++;
                        x_to_go--;

                    } else {
                        if (traffic_event.arrived_from_ == SOUTH_LEFT) {
                            current_lane = EAST_RIGHT;
                            state_.num_out_east_right_++;

                        } else if (traffic_event.arrived_from_ == EAST_STRAIGHT) {
                            current_lane = EAST_STRAIGHT;
                            state_.num_out_east_straight_++;

                        } else if (traffic_event.arrived_from_ == NORTH_RIGHT) {
                            current_lane = EAST_LEFT;
                            state_.num_out_east_left_++;
                        }
                    }
                } break;

                case EAST_STRAIGHT: {
                    state_.num_in_east_straight_--;
                    if ((x_to_go < 0) && 
                                (state_.num_out_west_straight_ < MAX_CARS_ON_ROAD)) {
                        current_lane = WEST_STRAIGHT;
                        state_.num_out_west_straight_++;
                        x_to_go++;

                    } else if ((y_to_go < 0) && 
                                (state_.num_out_west_left_ < MAX_CARS_ON_ROAD)) {
                        current_lane = WEST_LEFT;
                        state_.num_out_west_left_++;
                        y_to_go++;

                    } else if ((y_to_go > 0) && 
                                (state_.num_out_west_right_ < MAX_CARS_ON_ROAD)) {
                        current_lane = WEST_RIGHT;
                        state_.num_out_west_right_++;
                        y_to_go--;

                    } else {
                        if (traffic_event.arrived_from_ == NORTH_RIGHT) {
                            current_lane = EAST_LEFT;
                            state_.num_out_east_left_++;

                        } else if (traffic_event.arrived_from_ == EAST_STRAIGHT) {
                            current_lane = EAST_STRAIGHT;
                            state_.num_out_east_straight_++;

                        } else if (traffic_event.arrived_from_ == SOUTH_LEFT) {
                            current_lane = EAST_RIGHT;
                            state_.num_out_east_right_++;
                        }
                    }
                } break;

                case EAST_RIGHT: {
                    state_.num_in_east_right_--;
                    if ((y_to_go > 0) && 
                                (state_.num_out_north_straight_ < MAX_CARS_ON_ROAD)) {
                        current_lane = NORTH_STRAIGHT;
                        state_.num_out_north_straight_++;
                        y_to_go--;

                    } else if ((x_to_go > 0) && 
                                (state_.num_out_north_right_ < MAX_CARS_ON_ROAD)) {
                        current_lane = NORTH_RIGHT;
                        state_.num_out_north_right_++;
                        x_to_go --;

                    } else if ((x_to_go < 0) && 
                                (state_.num_out_north_left_ < MAX_CARS_ON_ROAD)) {
                        current_lane = NORTH_LEFT;
                        state_.num_out_north_left_++;
                        x_to_go++;

                    } else {
                        if (traffic_event.arrived_from_ == SOUTH_LEFT) {
                            current_lane = EAST_RIGHT;
                            state_.num_out_east_right_++;

                        } else if (traffic_event.arrived_from_ == EAST_STRAIGHT) {
                            current_lane = EAST_STRAIGHT;
                            state_.num_out_east_straight_++;

                        } else if (traffic_event.arrived_from_ == NORTH_RIGHT) {
                            current_lane = EAST_LEFT;
                            state_.num_out_east_left_++;
                        }
                    }
                } break;

                case WEST_LEFT: {
                    state_.num_in_west_left_--;
                    if ((y_to_go > 0) && 
                                (state_.num_out_north_straight_ < MAX_CARS_ON_ROAD)) {
                        current_lane = NORTH_STRAIGHT;
                        state_.num_out_north_straight_++;
                        y_to_go--;

                    } else if ((x_to_go > 0) && 
                                (state_.num_out_north_right_ < MAX_CARS_ON_ROAD)) {
                        current_lane = NORTH_RIGHT;
                        state_.num_out_north_right_++;
                        x_to_go--;

                    } else if ((x_to_go < 0) && 
                                (state_.num_out_north_left_ < MAX_CARS_ON_ROAD)) {
                        current_lane = NORTH_LEFT;
                        state_.num_out_north_left_++;
                        x_to_go++;

                    } else {
                        if (traffic_event.arrived_from_ == SOUTH_RIGHT) {
                            current_lane = WEST_LEFT;
                            state_.num_out_west_left_++;

                        } else if (traffic_event.arrived_from_ == WEST_STRAIGHT) {
                            current_lane = WEST_STRAIGHT;
                            state_.num_out_west_straight_++;

                        } else if (traffic_event.arrived_from_ == NORTH_LEFT) {
                            current_lane = WEST_RIGHT;
                            state_.num_out_west_right_++;
                        }
                    }
                } break;

                case WEST_STRAIGHT: {
                    state_.num_in_west_straight_--;
                    if ((x_to_go > 0) && 
                                (state_.num_out_east_straight_ < MAX_CARS_ON_ROAD)) {
                        current_lane = EAST_STRAIGHT;
                        state_.num_out_east_straight_++;
                        x_to_go--;

                    } else if ((y_to_go > 0) && 
                                (state_.num_out_east_left_ < MAX_CARS_ON_ROAD)) {
                        current_lane = EAST_LEFT;
                        state_.num_out_east_left_++;
                        y_to_go --;

                    } else if ((y_to_go < 0) && 
                                (state_.num_out_east_right_ < MAX_CARS_ON_ROAD)) {
                        current_lane = EAST_RIGHT;
                        state_.num_out_east_right_++;
                        y_to_go++;

                    } else {
                        if (traffic_event.arrived_from_ == SOUTH_RIGHT) {
                            current_lane = WEST_LEFT;
                            state_.num_out_west_left_++;

                        } else if (traffic_event.arrived_from_ == WEST_STRAIGHT) {
                            current_lane = WEST_STRAIGHT;
                            state_.num_out_west_straight_++;

                        } else if (traffic_event.arrived_from_ == NORTH_LEFT) {
                            current_lane = WEST_RIGHT;
                            state_.num_out_west_right_++;
                        }
                    }
                } break;

                case WEST_RIGHT: {
                    state_.num_in_west_right_--;
                    if ((y_to_go < 0) && 
                                (state_.num_out_south_straight_ < MAX_CARS_ON_ROAD)) {
                        current_lane = SOUTH_STRAIGHT;
                        state_.num_out_south_straight_++;
                        y_to_go++;

                    } else if ((x_to_go > 0) && 
                                (state_.num_out_south_left_ < MAX_CARS_ON_ROAD)) {
                        current_lane = SOUTH_LEFT;
                        state_.num_out_south_left_++;
                        x_to_go--;

                    } else if ((x_to_go < 0) && 
                                (state_.num_out_south_right_ < MAX_CARS_ON_ROAD)) {
                        current_lane = SOUTH_RIGHT;
                        state_.num_out_south_right_++;
                        x_to_go++;

                    } else {
                        if (traffic_event.arrived_from_ == SOUTH_RIGHT) {
                            current_lane = WEST_LEFT;
                            state_.num_out_west_left_++;

                        } else if (traffic_event.arrived_from_ == WEST_STRAIGHT) {
                            current_lane = WEST_STRAIGHT;
                            state_.num_out_west_straight_++;

                        } else if (traffic_event.arrived_from_ == NORTH_LEFT) {
                            current_lane = WEST_RIGHT;
                            state_.num_out_west_right_++;
                        }
                    }
                } break;

                case NORTH_LEFT: {
                    state_.num_in_north_left_--;
                    if ((x_to_go > 0) && 
                                (state_.num_out_east_straight_ < MAX_CARS_ON_ROAD)) {
                        current_lane = EAST_STRAIGHT;
                        state_.num_out_east_straight_++;
                        x_to_go--;

                    } else if ((y_to_go > 0) && 
                                (state_.num_out_east_left_ < MAX_CARS_ON_ROAD)) {
                        current_lane = EAST_LEFT;
                        state_.num_out_east_left_++;
                        y_to_go--;

                    } else if ((y_to_go < 0) && 
                                (state_.num_out_east_right_ < MAX_CARS_ON_ROAD)) {
                        current_lane = EAST_RIGHT;
                        state_.num_out_east_right_++;
                        y_to_go++;

                    } else {
                        if (traffic_event.arrived_from_ == WEST_RIGHT) {
                            current_lane = NORTH_LEFT;
                            state_.num_out_north_left_++;

                        } else if (traffic_event.arrived_from_ == NORTH_STRAIGHT) {
                            current_lane = NORTH_STRAIGHT;
                            state_.num_out_north_straight_++;

                        } else if (traffic_event.arrived_from_ == EAST_LEFT) {
                            current_lane = NORTH_RIGHT;
                            state_.num_out_north_right_++;
                        }
                    }
                } break;

                case NORTH_STRAIGHT: {
                    state_.num_in_north_straight_--;
                    if ((y_to_go < 0) && 
                                (state_.num_out_south_straight_ < MAX_CARS_ON_ROAD)) {
                        current_lane = SOUTH_STRAIGHT;
                        state_.num_out_south_straight_++;
                        y_to_go++;

                    } else if ((x_to_go > 0) && 
                                (state_.num_out_south_left_ < MAX_CARS_ON_ROAD)) {
                        current_lane = SOUTH_LEFT;
                        state_.num_out_south_left_++;
                        x_to_go--;

                    } else if ((x_to_go < 0) && 
                                (state_.num_out_south_right_ < MAX_CARS_ON_ROAD)) {
                        current_lane = SOUTH_RIGHT;
                        state_.num_out_south_right_++;
                        x_to_go++;

                    } else {
                        if (traffic_event.arrived_from_ == WEST_RIGHT) {
                            current_lane = NORTH_LEFT;
                            state_.num_out_north_left_++;

                        } else if (traffic_event.arrived_from_ == NORTH_STRAIGHT) {
                            current_lane = NORTH_STRAIGHT;
                            state_.num_out_north_straight_++;

                        } else if (traffic_event.arrived_from_ == EAST_LEFT) {
                            current_lane = NORTH_RIGHT;
                            state_.num_out_north_right_++;
                        }
                    }
                } break;

                case NORTH_RIGHT: {
                    state_.num_in_north_right_--;
                    if ((x_to_go < 0) && 
                                (state_.num_out_west_straight_ < MAX_CARS_ON_ROAD)) {
                        current_lane = WEST_STRAIGHT;
                        state_.num_out_west_straight_++;
                        x_to_go++;

                    } else if ((y_to_go < 0) && 
                                (state_.num_out_west_left_ < MAX_CARS_ON_ROAD)) {
                        current_lane = WEST_LEFT;
                        state_.num_out_west_left_++;
                        y_to_go++;

                    } else if ((y_to_go > 0) && 
                                (state_.num_out_west_right_ < MAX_CARS_ON_ROAD)) {
                        current_lane = WEST_RIGHT;
                        state_.num_out_west_right_++;
                        y_to_go--;

                    } else {
                        if (traffic_event.arrived_from_ == WEST_RIGHT) {
                            current_lane = NORTH_LEFT;
                            state_.num_out_north_left_++;

                        } else if (traffic_event.arrived_from_ == NORTH_STRAIGHT) {
                            current_lane = NORTH_STRAIGHT;
                            state_.num_out_north_straight_++;

                        } else if (traffic_event.arrived_from_ == EAST_LEFT) {
                            current_lane = NORTH_RIGHT;
                            state_.num_out_north_right_++;
                        }
                    }
                } break;

                case SOUTH_LEFT: {
                    state_.num_in_south_left_--;
                    if ((x_to_go < 0) && 
                                (state_.num_out_west_straight_ < MAX_CARS_ON_ROAD)) {
                        current_lane = WEST_STRAIGHT;
                        state_.num_out_west_straight_++;
                        x_to_go++;

                    } else if ((y_to_go < 0) && 
                                (state_.num_out_west_left_ < MAX_CARS_ON_ROAD)) {
                        current_lane = WEST_LEFT;
                        state_.num_out_west_left_++;
                        y_to_go++;

                    } else if ((y_to_go > 0) && 
                                (state_.num_out_west_right_ < MAX_CARS_ON_ROAD)) {
                        current_lane = WEST_RIGHT;
                        state_.num_out_west_right_++;
                        y_to_go--;

                    } else {
                        if (traffic_event.arrived_from_ == WEST_LEFT) {
                            current_lane = SOUTH_RIGHT;
                            state_.num_out_south_right_++;

                        } else if (traffic_event.arrived_from_ == SOUTH_STRAIGHT) {
                            current_lane = SOUTH_STRAIGHT;
                            state_.num_out_south_straight_++;

                        } else if (traffic_event.arrived_from_ == EAST_RIGHT) {
                            current_lane = SOUTH_LEFT;
                            state_.num_out_south_left_++;
                        }
                    }
                } break;

                case SOUTH_STRAIGHT: {
                    state_.num_in_south_straight_--;
                    if ((y_to_go > 0) && 
                                (state_.num_out_north_straight_ < MAX_CARS_ON_ROAD)) {
                        current_lane = NORTH_STRAIGHT;
                        state_.num_out_north_straight_++;
                        y_to_go--;

                    } else if ((x_to_go < 0) && 
                                (state_.num_out_north_left_ < MAX_CARS_ON_ROAD)) {
                        current_lane = NORTH_LEFT;
                        state_.num_out_north_left_++;
                        x_to_go++;

                    } else if ((x_to_go > 0) && 
                                (state_.num_out_north_right_ < MAX_CARS_ON_ROAD)) {
                        current_lane = NORTH_RIGHT;
                        state_.num_out_north_right_++;
                        x_to_go --;

                    } else {
                        if (traffic_event.arrived_from_ == EAST_RIGHT) {
                            current_lane = SOUTH_LEFT;
                            state_.num_out_south_left_++;

                        } else if (traffic_event.arrived_from_ == SOUTH_STRAIGHT) {
                            current_lane = SOUTH_STRAIGHT;
                            state_.num_out_south_straight_++;

                        } else if (traffic_event.arrived_from_ == WEST_LEFT) {
                            current_lane = SOUTH_RIGHT;
                            state_.num_out_south_right_++;
                        }
                    }
                } break;

                case SOUTH_RIGHT: {
                    state_.num_in_south_right_--;
                    if ((x_to_go > 0) && 
                                (state_.num_out_east_straight_ < MAX_CARS_ON_ROAD)) {
                        current_lane = EAST_STRAIGHT;
                        state_.num_out_east_straight_++;
                        x_to_go--;

                    } else if ((y_to_go > 0) && 
                                (state_.num_out_east_left_ < MAX_CARS_ON_ROAD)) {
                        current_lane = EAST_LEFT;
                        state_.num_out_east_left_++;
                        y_to_go--;

                    } else if ((y_to_go < 0) && 
                                (state_.num_out_east_right_ < MAX_CARS_ON_ROAD)) {
                        current_lane = EAST_RIGHT;
                        state_.num_out_east_right_++;
                        y_to_go++;

                    } else {
                        if (traffic_event.arrived_from_ == EAST_RIGHT) {
                            current_lane = SOUTH_LEFT;
                            state_.num_out_south_left_++;

                        } else if (traffic_event.arrived_from_ == SOUTH_STRAIGHT) {
                            current_lane = SOUTH_STRAIGHT;
                            state_.num_out_south_straight_++;

                        } else if (traffic_event.arrived_from_ == WEST_LEFT) {
                            current_lane = SOUTH_RIGHT;
                            state_.num_out_south_right_++;
                        }
                    }
                } break;
            }

            auto timestamp = traffic_event.ts_ + (unsigned int) std::ceil(interval_expo(*this->rng_));
            events.emplace_back(new TrafficEvent {
                            this->name_, DEPARTURE, x_to_go, y_to_go, 
                            traffic_event.current_lane_, current_lane, timestamp});
        } break;

        default: {
            assert(0);
        }
    }
    return events;
}

std::string Intersection::compute_move(direction_t direction) {

    unsigned int new_x = 0, new_y = 0;
    unsigned int current_y = this->index_ / num_intersections_x_;
    unsigned int current_x = this->index_ % num_intersections_x_;

    switch (direction) {
        case WEST: {
            new_x = (current_x + num_intersections_x_ - 1) % num_intersections_x_;
            new_y = current_y;
        } break;

        case EAST: {
            new_x = (current_x + 1) % num_intersections_x_;
            new_y = current_y;
        } break;

        case SOUTH: {
            new_x = current_x;
            new_y = (current_y + num_intersections_y_ - 1) % num_intersections_y_;
        } break;

        case NORTH: {
            new_x = current_x;
            new_y = (current_y + 1) % num_intersections_y_;
        } break;

        default: {
            std::cerr << "Invalid move direction " << direction << std::endl;
            assert(0);
        }
    }
    return lp_name(new_x + new_y * num_intersections_x_);
}

int main(int argc, const char** argv) {

    unsigned int num_intersections_x   = 100;
    unsigned int num_intersections_y   = 100;
    unsigned int num_cars              = 25;
    unsigned int mean_interval         = 400;

    TCLAP::ValueArg<unsigned int> num_intersections_x_arg("x", "num-intersections-x", 
                "Width of intersection grid", false, num_intersections_x, "unsigned int");
    TCLAP::ValueArg<unsigned int> num_intersections_y_arg("y", "num-intersections-y", 
                "Height of intersection grid", false, num_intersections_y, "unsigned int");
    TCLAP::ValueArg<unsigned int> num_cars_arg("n", "number-of-cars", 
                "Number of cars per intersection", false, num_cars, "unsigned int");
    TCLAP::ValueArg<unsigned int> mean_interval_arg("i", "mean-interval", 
                "Mean interval", false, mean_interval, "unsigned int");

    std::vector<TCLAP::Arg*> cmd_line_args = {  &num_intersections_x_arg, 
                                                &num_intersections_y_arg, 
                                                &num_cars_arg, 
                                                &mean_interval_arg  };

    warped::Simulation simulation {"Traffic Simulation", argc, argv, cmd_line_args};

    num_intersections_x = num_intersections_x_arg.getValue();
    num_intersections_y = num_intersections_y_arg.getValue();
    num_cars            = num_cars_arg.getValue();
    mean_interval       = mean_interval_arg.getValue();

    std::vector<Intersection> lps;
    for (unsigned int index = 0; index < num_intersections_x * num_intersections_y; index++) {
        lps.emplace_back(   num_intersections_x, 
                                num_intersections_y, 
                                num_cars, 
                                mean_interval, 
                                index
                            );
    }

    std::vector<warped::LogicalProcess*> lp_pointers;
    for (auto& lp : lps) {
        lp_pointers.push_back(&lp);
    }
    simulation.simulate(lp_pointers);

    unsigned int total_cars_arrived = 0, total_cars_finished = 0;
    for (auto& lp : lps) {
        // std::cerr<<"Total cars arrived  : "<<lp.state_.total_cars_arrived_<<"\n";
        total_cars_arrived  += lp.state_.total_cars_arrived_;
        total_cars_finished += lp.state_.total_cars_finished_;
    }
    std::cout << "Total cars arrived  : " << total_cars_arrived  << std::endl;
    std::cout << "Total cars finished : " << total_cars_finished << std::endl;

    return 0;
}

