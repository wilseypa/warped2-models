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

#if 0
            TrafficState->total_cars_arrived_++;
            
            switch(M->car.current_lane){
                    
                case WEST_LEFT:
                    TrafficState->num_in_east_left_++;
                    M->car.current_lane = EAST_LEFT;
                    break;
                case WEST_STRAIGHT:
                    TrafficState->num_in_east_straight_++;
                    M->car.current_lane = EAST_STRAIGHT;
                    break;
                case WEST_RIGHT:
                    TrafficState->num_in_east_right_++;
                    M->car.current_lane = EAST_RIGHT;
                    break;
                case EAST_LEFT:
                    TrafficState->num_in_west_left_++;
                    M->car.current_lane = WEST_LEFT;
                    break;
                case EAST_STRAIGHT:
                    TrafficState->num_in_west_straight_++;
                    M->car.current_lane = WEST_STRAIGHT;
                    break;
                case EAST_RIGHT:
                    TrafficState->num_in_west_right_++;
                    M->car.current_lane = WEST_RIGHT;
                    break;
                case NORTH_LEFT:
                    TrafficState->num_in_south_left_++;
                    M->car.current_lane = SOUTH_LEFT;
                    break;
                case NORTH_STRAIGHT:
                    TrafficState->num_in_south_straight_++;
                    M->car.current_lane = SOUTH_STRAIGHT;
                    break;
                case NORTH_RIGHT:
                    TrafficState->num_in_south_right_++;
                    M->car.current_lane = SOUTH_RIGHT;
                    break;
                case SOUTH_LEFT:
                    TrafficState->num_in_north_left_++;
                    M->car.current_lane = NORTH_LEFT;
                    break;
                case SOUTH_STRAIGHT:
                    TrafficState->num_in_north_straight_++;
                    M->car.current_lane = NORTH_STRAIGHT;
                    break;
                case SOUTH_RIGHT:
                    TrafficState->num_in_north_right_++;
                    M->car.current_lane = NORTH_RIGHT;
                    break;
            }
            
            events.emplace_back(new TrafficEvent {car.x_to_go, car.y_to_go, car.current_lane, car.arrived_from, car.event_type});
            received_event.type_ = DIRECTION_SELECT;

#endif
        } break;

        case DEPARTURE: {

#if 0
            switch(M->car.current_lane){
                case WEST_LEFT:
                    TrafficState->num_out_west_left_--;
                    new_event_direction = 0;
                    break;
                case WEST_STRAIGHT:
                    TrafficState->num_out_west_straight_--;
                    new_event_direction = 0;
                    break;
                case WEST_RIGHT:
                    TrafficState->num_out_west_right_--;
                    new_event_direction = 0;
                    break;
                case EAST_LEFT:
                    TrafficState->num_out_east_left_--;
                    new_event_direction = 1;
                    break;
                case EAST_STRAIGHT:
                    TrafficState->num_out_east_straight_--;
                    new_event_direction = 1;
                    break;
                case EAST_RIGHT:
                    TrafficState->num_out_east_right_--;
                    new_event_direction = 1;
                    break;
                case NORTH_LEFT:
                    TrafficState->num_out_north_left_--;
                    new_event_direction = 3;
                    break;
                case NORTH_STRAIGHT:
                    TrafficState->num_out_north_straight_--;
                    new_event_direction = 3;
                    break;
                case NORTH_RIGHT:
                    TrafficState->num_out_north_right_--;
                    new_event_direction = 3;
                    break;
                case SOUTH_LEFT:
                    TrafficState->num_out_south_left_--;
                    new_event_direction = 2;
                    break;
                case SOUTH_STRAIGHT:
                    TrafficState->num_out_south_straight_--;
                    new_event_direction = 2;
                    break;
                case SOUTH_RIGHT:
                    TrafficState->num_out_south_right_--;
                    new_event_direction = 2;
                    break;
            }
            
            events.emplace_back(new TrafficEvent {car.x_to_go, car.y_to_go, car.current_lane, car.arrived_from, car.event_type});
            received_event.type_ = ARRIVAL;
#endif
        } break;

            
        case DIRECTION_SELECT: {
#if 0
            switch(M->car.current_lane){
                case EAST_LEFT:
                    TrafficState->num_in_east_left--;
                    if(M->car.y_to_go < 0 && TrafficEvent->num_out_south_straight < MAX_CARS_ON_ROAD){
                        M->car.current_lane = SOUTH_STRAIGHT;
                        TrafficState->num_out_south_straight ++;
                        M->car.y_to_go++;
                    }
                    else if(M->car.x_to_go < 0 && TrafficEvent->num_out_south_right < MAX_CARS_ON_ROAD){
                        M->car.current_lane = SOUTH_RIGHT;
                        TrafficState->num_out_south_right ++;
                        M->car.x_to_go++;
                    }
                    else if(M->car.x_to_go > 0 && TrafficEvent->num_out_south_left < MAX_CARS_ON_ROAD){
                        M->car.current_lane = SOUTH_LEFT;
                        TrafficState->num_out_south_left ++;
                        M->car.x_to_go--;
                    }
                    else{
                        if(M->car.arrived_from == SOUTH_LEFT){
                            M->car.current_lane = EAST_RIGHT;
                            TrafficState->num_out_east_right++;
                        }
                        else if(M->car.arrived_from == EAST_STRAIGHT){
                            M->car.current_lane = EAST_STRAIGHT;
                            TrafficState->num_out_east_straight++;
                        }
                        else if(M->car.arrived_from == NORTH_RIGHT){
                            M->car.current_lane = EAST_LEFT;
                            TrafficState->num_out_east_left++;
                        }
                    }
                    break;
                case EAST_STRAIGHT:
                    TrafficState->num_in_east_straight--;
                    if(M->car.x_to_go < 0 && TrafficEvent->num_out_west_straight < MAX_CARS_ON_ROAD){
                        M->car.current_lane = WEST_STRAIGHT;
                        TrafficState->num_out_west_straight ++;
                        M->car.x_to_go++;
                    }
                    else if(M->car.y_to_go < 0 && TrafficEvent->num_out_west_left < MAX_CARS_ON_ROAD){
                        M->car.current_lane = WEST_LEFT;
                        TrafficState->num_out_west_left ++;
                        M->car.y_to_go++;
                    }
                    else if(M->car.y_to_go > 0 && TrafficEvent->num_out_west_right < MAX_CARS_ON_ROAD){
                        M->car.current_lane = WEST_RIGHT;
                        TrafficState->num_out_west_right ++;
                        M->car.y_to_go--;
                    }
                    else{
                        if(M->car.arrived_from == NORTH_RIGHT){
                            M->car.current_lane = EAST_LEFT;
                            TrafficEvent->num_out_east_left++;
                        }
                        else if(M->car.arrived_from == EAST_STRAIGHT){
                            M->car.current_lane = EAST_STRAIGHT;
                            TrafficEvent->num_out_east_straight++;
                        }
                        else if(M->car.arrived_from == SOUTH_LEFT){
                            M->car.current_lane = EAST_RIGHT;
                            TrafficEvent->num_out_east_right++;
                        }
                    }
                    
                    break;
                case EAST_RIGHT:
                    TrafficEvent->num_in_east_right--;
                    if(M->car.y_to_go > 0 && TrafficEvent->num_out_north_straight < MAX_CARS_ON_ROAD){
                        M->car.current_lane = NORTH_STRAIGHT;
                        TrafficEvent->num_out_north_straight ++;
                        M->car.y_to_go--;
                    }
                    else if(M->car.x_to_go > 0 && TrafficEvent->num_out_north_right < MAX_CARS_ON_ROAD){
                        M->car.current_lane = NORTH_RIGHT;
                        TrafficEvent->num_out_north_right ++;
                        M->car.x_to_go --;
                    }
                    else if(M->car.x_to_go < 0 && TrafficEvent->num_out_north_left < MAX_CARS_ON_ROAD){
                        M->car.current_lane = NORTH_LEFT;
                        TrafficEvent->num_out_north_left ++;
                        M->car.x_to_go++;
                    }
                    else{
                        if(M->car.arrived_from == SOUTH_LEFT){
                            M->car.current_lane = EAST_RIGHT;
                            TrafficEvent->num_out_east_right++;
                        }
                        else if(M->car.arrived_from == EAST_STRAIGHT){
                            M->car.current_lane = EAST_STRAIGHT;
                            TrafficEvent->num_out_east_straight++;
                        }
                        else if(M->car.arrived_from == NORTH_RIGHT){
                            M->car.current_lane = EAST_LEFT;
                            TrafficEvent->num_out_east_left++;
                        }
                    }
                    break;
                case WEST_LEFT:
                    TrafficEvent->num_in_west_left--;
                    if(M->car.y_to_go > 0 && TrafficEvent->num_out_north_straight < MAX_CARS_ON_ROAD){
                        M->car.current_lane = NORTH_STRAIGHT;
                        TrafficEvent->num_out_north_straight ++;
                        M->car.y_to_go--;
                    }
                    else if(M->car.x_to_go > 0 && TrafficEvent->num_out_north_right < MAX_CARS_ON_ROAD){
                        M->car.current_lane = NORTH_RIGHT;
                        TrafficEvent->num_out_north_right ++;
                        M->car.x_to_go--;
                    }
                    else if(M->car.x_to_go < 0 && TrafficEvent->num_out_north_left < MAX_CARS_ON_ROAD){
                        M->car.current_lane = NORTH_LEFT;
                        TrafficEvent->num_out_north_left ++;
                        M->car.x_to_go++;
                    }
                    else{
                        if(M->car.arrived_from == SOUTH_RIGHT){
                            M->car.current_lane = WEST_LEFT;
                            TrafficEvent->num_out_west_left++;
                        }
                        else if(M->car.arrived_from == WEST_STRAIGHT){
                            M->car.current_lane = WEST_STRAIGHT;
                            TrafficEvent->num_out_west_straight++;
                        }
                        else if(M->car.arrived_from == NORTH_LEFT){
                            M->car.current_lane = WEST_RIGHT;
                            TrafficEvent->num_out_west_right++;
                        }
                    }
                    break;
                case WEST_STRAIGHT:
                    TrafficEvent->num_in_west_straight--;
                    if(M->car.x_to_go > 0 && TrafficEvent->num_out_east_straight < MAX_CARS_ON_ROAD){
                        M->car.current_lane = EAST_STRAIGHT;
                        TrafficEvent->num_out_east_straight ++;
                        M->car.x_to_go--;
                    }
                    else if(M->car.y_to_go > 0 && TrafficEvent->num_out_east_left < MAX_CARS_ON_ROAD){
                        M->car.current_lane = EAST_LEFT;
                        TrafficEvent->num_out_east_left ++;
                        M->car.y_to_go --;
                    }
                    else if(M->car.y_to_go < 0 && TrafficEvent->num_out_east_right < MAX_CARS_ON_ROAD){
                        M->car.current_lane = EAST_RIGHT;
                        TrafficEvent->num_out_east_right ++;
                        M->car.y_to_go++;
                    }
                    else{
                        if(M->car.arrived_from == SOUTH_RIGHT){
                            M->car.current_lane = WEST_LEFT;
                            TrafficEvent->num_out_west_left++;
                        }
                        else if(M->car.arrived_from == WEST_STRAIGHT){
                            M->car.current_lane = WEST_STRAIGHT;
                            TrafficEvent->num_out_west_straight++;
                        }
                        else if(M->car.arrived_from == NORTH_LEFT){
                            M->car.current_lane = WEST_RIGHT;
                            TrafficEvent->num_out_west_right++;
                        }
                    }
                    break;
                case WEST_RIGHT:
                    TrafficEvent->num_in_west_right--;
                    if(M->car.y_to_go < 0 && TrafficEvent->num_out_south_straight < MAX_CARS_ON_ROAD){
                        M->car.current_lane = SOUTH_STRAIGHT;
                        TrafficEvent->num_out_south_straight ++;
                        M->car.y_to_go++;
                    }
                    else if(M->car.x_to_go > 0 && TrafficEvent->num_out_south_left < MAX_CARS_ON_ROAD){
                        M->car.current_lane = SOUTH_LEFT;
                        TrafficEvent->num_out_south_left ++;
                        M->car.x_to_go--;
                    }
                    else if(M->car.x_to_go < 0 && TrafficEvent->num_out_south_right < MAX_CARS_ON_ROAD){
                        M->car.current_lane = SOUTH_RIGHT;
                        TrafficEvent->num_out_south_right ++;
                        M->car.x_to_go++;
                    }
                    else{
                        if(M->car.arrived_from == SOUTH_RIGHT){
                            M->car.current_lane = WEST_LEFT;
                            TrafficEvent->num_out_west_left++;
                        }
                        else if(M->car.arrived_from == WEST_STRAIGHT){
                            M->car.current_lane = WEST_STRAIGHT;
                            TrafficEvent->num_out_west_straight++;
                        }
                        else if(M->car.arrived_from == NORTH_LEFT){
                            M->car.current_lane = WEST_RIGHT;
                            TrafficEvent->num_out_west_right++;
                        }
                    }
                    break;
                case NORTH_LEFT:
                    TrafficEvent->num_in_north_left--;
                    if(M->car.x_to_go > 0 && TrafficEvent->num_out_east_straight < MAX_CARS_ON_ROAD){
                        M->car.current_lane = EAST_STRAIGHT;
                        TrafficEvent->num_out_east_straight ++;
                        M->car.x_to_go --;
                    }
                    else if(M->car.y_to_go > 0 && TrafficEvent->num_out_east_left < MAX_CARS_ON_ROAD){
                        M->car.current_lane = EAST_LEFT;
                        TrafficEvent->num_out_east_left ++;
                        M->car.y_to_go--;
                    }
                    else if(M->car.y_to_go < 0 && TrafficEvent->num_out_east_right < MAX_CARS_ON_ROAD){
                        M->car.current_lane = EAST_RIGHT;
                        TrafficEvent->num_out_east_right ++;
                        M->car.y_to_go++;
                    }
                    else{
                        if(M->car.arrived_from == WEST_RIGHT){
                            M->car.current_lane = NORTH_LEFT;
                            TrafficEvent->num_out_north_left++;
                        }
                        else if(M->car.arrived_from == NORTH_STRAIGHT){
                            M->car.current_lane = NORTH_STRAIGHT;
                            TrafficEvent->num_out_north_straight++;
                        }
                        else if(M->car.arrived_from == EAST_LEFT){
                            M->car.current_lane = NORTH_RIGHT;
                            TrafficEvent->num_out_north_right++;
                        }
                    }
                    
                    break;
                case NORTH_STRAIGHT:
                    TrafficEvent->num_in_north_straight--;
                    if(M->car.y_to_go < 0 && TrafficEvent->num_out_south_straight < MAX_CARS_ON_ROAD){
                        M->car.current_lane = SOUTH_STRAIGHT;
                        TrafficEvent->num_out_south_straight ++;
                        M->car.y_to_go++;
                    }
                    else if(M->car.x_to_go > 0 && TrafficEvent->num_out_south_left < MAX_CARS_ON_ROAD){
                        M->car.current_lane = SOUTH_LEFT;
                        TrafficEvent->num_out_south_left ++;
                        M->car.x_to_go--;
                    }
                    else if(M->car.x_to_go < 0 && TrafficEvent->num_out_south_right < MAX_CARS_ON_ROAD){
                        M->car.current_lane = SOUTH_RIGHT;
                        TrafficEvent->num_out_south_right ++;
                        M->car.x_to_go++;
                    }
                    else{
                        if(M->car.arrived_from == WEST_RIGHT){
                            M->car.current_lane = NORTH_LEFT;
                            TrafficEvent->num_out_north_left++;
                        }
                        else if(M->car.arrived_from == NORTH_STRAIGHT){
                            M->car.current_lane = NORTH_STRAIGHT;
                            TrafficEvent->num_out_north_straight++;
                        }
                        else if(M->car.arrived_from == EAST_LEFT){
                            M->car.current_lane = NORTH_RIGHT;
                            TrafficEvent->num_out_north_right++;
                        }
                    }
                    break;
                    
                case NORTH_RIGHT:
                    TrafficEvent->num_in_north_right--;
                    if(M->car.x_to_go < 0 && TrafficEvent->num_out_west_straight < MAX_CARS_ON_ROAD){
                        M->car.current_lane = WEST_STRAIGHT;
                        TrafficEvent->num_out_west_straight ++;
                        M->car.x_to_go++;
                    }
                    else if(M->car.y_to_go < 0 && TrafficEvent->num_out_west_left < MAX_CARS_ON_ROAD){
                        M->car.current_lane = WEST_LEFT;
                        TrafficEvent->num_out_west_left ++;
                        M->car.y_to_go++;
                    }
                    else if(M->car.y_to_go > 0 && TrafficEvent->num_out_west_right < MAX_CARS_ON_ROAD){
                        M->car.current_lane = WEST_RIGHT;
                        TrafficEvent->num_out_west_right ++;
                        M->car.y_to_go--;
                    }
                    else{
                        if(M->car.arrived_from == WEST_RIGHT){
                            M->car.current_lane = NORTH_LEFT;
                            TrafficEvent->num_out_north_left++;
                        }
                        else if(M->car.arrived_from == NORTH_STRAIGHT){
                            M->car.current_lane = NORTH_STRAIGHT;
                            TrafficEvent->num_out_north_straight++;
                        }
                        else if(M->car.arrived_from == EAST_LEFT){
                            M->car.current_lane = NORTH_RIGHT;
                            TrafficEvent->num_out_north_right++;
                        }
                    }
                    break;
                case SOUTH_LEFT:
                    TrafficEvent->num_in_south_left--;
                    if(M->car.x_to_go < 0 && TrafficEvent->num_out_west_straight < MAX_CARS_ON_ROAD){
                        M->car.current_lane = WEST_STRAIGHT;
                        TrafficEvent->num_out_west_straight ++;
                        M->car.x_to_go++;
                    }
                    else if(M->car.y_to_go < 0 && TrafficEvent->num_out_west_left < MAX_CARS_ON_ROAD){
                        M->car.current_lane = WEST_LEFT;
                        TrafficEvent->num_out_west_left ++;
                        M->car.y_to_go++;
                    }
                    else if(M->car.y_to_go > 0 && TrafficEvent->num_out_west_right < MAX_CARS_ON_ROAD){
                        M->car.current_lane = WEST_RIGHT;
                        TrafficEvent->num_out_west_right ++;
                        M->car.y_to_go--;
                    }
                    else{
                        if(M->car.arrived_from == WEST_LEFT){
                            M->car.current_lane = SOUTH_RIGHT;
                            TrafficEvent->num_out_south_right++;
                        }
                        else if(M->car.arrived_from == SOUTH_STRAIGHT){
                            M->car.current_lane = SOUTH_STRAIGHT;
                            TrafficEvent->num_out_south_straight++;
                        }
                        else if(M->car.arrived_from == EAST_RIGHT){
                            M->car.current_lane = SOUTH_LEFT;
                            TrafficEvent->num_out_south_left++;
                        }
                    }
                    
                    break;
                case SOUTH_STRAIGHT:
                    TrafficEvent->num_in_south_straight--;
                    if(M->car.y_to_go > 0 && TrafficEvent->num_out_north_straight < MAX_CARS_ON_ROAD){
                        M->car.current_lane = NORTH_STRAIGHT;
                        TrafficEvent->num_out_north_straight ++;
                        M->car.y_to_go--;
                    }
                    else if(M->car.x_to_go < 0 && TrafficEvent->num_out_north_left < MAX_CARS_ON_ROAD){
                        M->car.current_lane = NORTH_LEFT;
                        TrafficEvent->num_out_north_left ++;
                        M->car.x_to_go++;
                    }
                    else if(M->car.x_to_go > 0 && TrafficEvent->num_out_north_right < MAX_CARS_ON_ROAD){
                        M->car.current_lane = NORTH_RIGHT;
                        TrafficEvent->num_out_north_right ++;
                        M->car.x_to_go --;
                    }
                    else{
                        if(M->car.arrived_from == EAST_RIGHT){
                            M->car.current_lane = SOUTH_LEFT;
                            TrafficEvent->num_out_south_left++;
                        }
                        else if(M->car.arrived_from == SOUTH_STRAIGHT){
                            M->car.current_lane = SOUTH_STRAIGHT;
                            TrafficEvent->num_out_south_straight++;
                        }
                        else if(M->car.arrived_from == WEST_LEFT){
                            M->car.current_lane = SOUTH_RIGHT;
                            TrafficEvent->num_out_south_right++;
                        }
                    }
                    break;
                    
                case SOUTH_RIGHT:
                    TrafficEvent->num_in_south_right--;
                    if(M->car.x_to_go > 0 && TrafficEvent->num_out_east_straight < MAX_CARS_ON_ROAD){
                        M->car.current_lane = EAST_STRAIGHT;
                        TrafficEvent->num_out_east_straight ++;
                        M->car.x_to_go--;
                    }
                    else if(M->car.y_to_go > 0 && TrafficEvent->num_out_east_left < MAX_CARS_ON_ROAD){
                        M->car.current_lane = EAST_LEFT;
                        TrafficEvent->num_out_east_left ++;
                        M->car.y_to_go--;
                    }
                    else if(M->car.y_to_go < 0 && TrafficEvent->num_out_east_right < MAX_CARS_ON_ROAD){
                        M->car.current_lane = EAST_RIGHT;
                        TrafficEvent->num_out_east_right ++;
                        M->car.y_to_go++;
                    }
                    else{
                        if(M->car.arrived_from == EAST_RIGHT){
                            M->car.current_lane = SOUTH_LEFT;
                            TrafficEvent->num_out_south_left++;
                        }
                        else if(M->car.arrived_from == SOUTH_STRAIGHT){
                            M->car.current_lane = SOUTH_STRAIGHT;
                            TrafficEvent->num_out_south_straight++;
                        }
                        else if(M->car.arrived_from == WEST_LEFT){
                            M->car.current_lane = SOUTH_RIGHT;
                            TrafficEvent->num_out_south_right++;
                        }
                    }
                    break;
            }
            events.emplace_back(new TrafficEvent {car.x_to_go, car.y_to_go, car.current_lane, car.arrived_from, car.event_type});
            received_event.type_ = DEPARTURE;
#endif            
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

