// Ported from the ROSS traffic model 
// https://github.com/carothersc/ROSS-Models/blob/master/traffic/

#ifndef TRAFFIC_HPP_DEFINED
#define TRAFFIC_HPP_DEFINED

#include <string>
#include <vector>
#include <memory>

#include "warped.hpp"
#include "MLCG.h"

WARPED_DEFINE_OBJECT_STATE_STRUCT(TrafficState) {

    unsigned int total_cars_arrived_;
    unsigned int total_cars_finished_;
    unsigned int num_in_north_left_;
    unsigned int num_in_north_straight_;
    unsigned int num_in_north_right_;
    unsigned int num_in_south_left_;
    unsigned int num_in_south_straight_;
    unsigned int num_in_south_right_;
    unsigned int num_in_east_left_;
    unsigned int num_in_east_straight_;
    unsigned int num_in_east_right_;
    unsigned int num_in_west_left_;
    unsigned int num_in_west_straight_;
    unsigned int num_in_west_right_;
    unsigned int num_out_north_left_;
    unsigned int num_out_north_straight_;
    unsigned int num_out_north_right_;
    unsigned int num_out_south_left_;
    unsigned int num_out_south_straight_;
    unsigned int num_out_south_right_;
    unsigned int num_out_east_left_;
    unsigned int num_out_east_straight_;
    unsigned int num_out_east_right_;
    unsigned int num_out_west_left_;
    unsigned int num_out_west_straight_;
    unsigned int num_out_west_right_;
};

enum traffic_event_t {

    ARRIVAL,
    DEPARTURE,
    DIRECTION_SELECT
};

enum car_direction_t {

    NORTH_LEFT, 
    NORTH_STRAIGHT, 
    NORTH_RIGHT, 
    SOUTH_LEFT, 
    SOUTH_STRAIGHT, 
    SOUTH_RIGHT, 
    EAST_LEFT, 
    EAST_STRAIGHT, 
    EAST_RIGHT, 
    WEST_LEFT, 
    WEST_STRAIGHT, 
    WEST_RIGHT
};

enum direction_t {

    NORTH, 
    SOUTH, 
    EAST, 
    WEST
};

class TrafficEvent : public warped::Event {
public:
    TrafficEvent() = default;
    TrafficEvent(   const std::string& receiver_name, 
                    const traffic_event_t type, 
                    const int x_to_go, 
                    const int y_to_go, 
                    const car_direction_t arrived_from, 
                    const car_direction_t current_lane, 
                    const unsigned int timestamp    )
            :   receiver_name_(receiver_name), 
                type_(type), 
                x_to_go_(x_to_go),
                y_to_go_(y_to_go),
                arrived_from_(arrived_from),
                current_lane_(current_lane),
                ts_(timestamp)  {}

    const std::string& receiverName() const { return receiver_name_; }
    unsigned int timestamp() const { return ts_; }

    std::string receiver_name_;
    traffic_event_t type_;
	int x_to_go_;
	int y_to_go_;
	car_direction_t arrived_from_;
	car_direction_t current_lane_;
    unsigned int ts_;

    WARPED_REGISTER_SERIALIZABLE_MEMBERS(
                    cereal::base_class<warped::Event>(this), receiver_name_, 
                    type_, x_to_go_, y_to_go_, arrived_from_, current_lane_, ts_)
};

class Intersection : public warped::SimulationObject {
public:
    Intersection(   const unsigned int num_intersections_x,
                    const unsigned int num_intersections_y,
                    const unsigned int num_cars,
                    const unsigned int mean_interval,
                    const unsigned int index    )
            :   SimulationObject(object_name(index)),
                state_(),
                rng_(new MLCG),
                num_intersections_x_(num_intersections_x),
                num_intersections_y_(num_intersections_y),
                num_cars_(num_cars),
                mean_interval_(mean_interval),
                index_(index)       {

         state_.total_cars_arrived_ = 0;
         state_.total_cars_finished_ = 0;
         state_.num_in_north_left_ = 0;
         state_.num_in_north_straight_ = 0;
         state_.num_in_north_right_ = 0;
         state_.num_in_south_left_ = 0;
         state_.num_in_south_straight_ = 0;
         state_.num_in_south_right_ = 0;
         state_.num_in_east_left_ = 0;
         state_.num_in_east_straight_ = 0;
         state_.num_in_east_right_ = 0;
         state_.num_in_west_left_ = 0;
         state_.num_in_west_straight_ = 0;
         state_.num_in_west_right_ = 0;
         state_.num_out_north_left_ = 0;
         state_.num_out_north_straight_ = 0;
         state_.num_out_north_right_ = 0;
         state_.num_out_south_left_ = 0;
         state_.num_out_south_straight_ = 0;
         state_.num_out_south_right_ = 0;
         state_.num_out_east_left_ = 0;
         state_.num_out_east_straight_ = 0;
         state_.num_out_east_right_ = 0;
         state_.num_out_west_left_ = 0;
         state_.num_out_west_straight_ = 0;
         state_.num_out_west_right_ = 0;
    }

    virtual std::vector<std::shared_ptr<warped::Event>> createInitialEvents();
    virtual std::vector<std::shared_ptr<warped::Event>> receiveEvent(const warped::Event&);
    virtual warped::ObjectState& getState() { return this->state_; }

    TrafficState state_;

    static inline std::string object_name(const unsigned int);

protected:
    std::shared_ptr<MLCG> rng_;
    const unsigned int num_intersections_x_;
    const unsigned int num_intersections_y_;
    const unsigned int num_cars_;
    const unsigned int mean_interval_;
    const unsigned int index_;

    std::string compute_move(direction_t direction);
};

#endif
