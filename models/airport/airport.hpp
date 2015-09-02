// An implementation of Fujimoto's airport model
// Ported from the ROSS airport model (https://github.com/carothersc/ROSS/blob/master/ross/models/airport)

#ifndef AIRPORT_HPP_DEFINED
#define AIRPORT_HPP_DEFINED

#include <string>
#include <vector>
#include <memory>
#include <random>

#include "warped.hpp"

WARPED_DEFINE_LP_STATE_STRUCT(AirportState) {
    unsigned int arrivals_;
    unsigned int departures_;
    unsigned int planes_grounded_;
};

enum airport_event_t {
    ARRIVAL,
    DEPARTURE
};

enum direction_t {

    LEFT,
    RIGHT,
    DOWN,
    UP
};

class AirportEvent : public warped::Event {
public:
    AirportEvent() = default;
    AirportEvent(const std::string& receiver_name, const airport_event_t type, 
                                                    const unsigned int timestamp)
        : receiver_name_(receiver_name), type_(type), ts_(timestamp) {}

    const std::string& receiverName() const { return receiver_name_; }
    unsigned int timestamp() const { return ts_; }

    std::string receiver_name_;
    airport_event_t type_;
    unsigned int ts_;

    WARPED_REGISTER_SERIALIZABLE_MEMBERS(cereal::base_class<warped::Event>(this), receiver_name_, type_, ts_)
};

class Airport : public warped::LogicalProcess {
public:
    Airport(    const std::string& name, 
                const unsigned int num_airports_x, 
                const unsigned int num_airports_y, 
                const unsigned int num_planes, 
                const unsigned int arrive_mean, 
                const unsigned int depart_mean, 
                const unsigned int index)
        :   LogicalProcess(name), 
            state_(), 
            rng_(new std::default_random_engine(index)),
            num_airports_x_(num_airports_x), 
            num_airports_y_(num_airports_y), 
            num_planes_(num_planes), 
            arrive_mean_(arrive_mean), 
            depart_mean_(depart_mean), 
            index_(index) {

        state_.departures_      = 0;
        state_.arrivals_        = 0;
        state_.planes_grounded_ = num_planes_;
    }

    virtual std::vector<std::shared_ptr<warped::Event> > initializeLP() override;
    virtual std::vector<std::shared_ptr<warped::Event> > receiveEvent(const warped::Event&);
    virtual warped::LPState& getState() { return this->state_; }

    AirportState state_;

    static inline std::string lp_name(const unsigned int);

protected:
    std::shared_ptr<std::default_random_engine> rng_;
    const unsigned int num_airports_x_;
    const unsigned int num_airports_y_;
    const unsigned int num_planes_;
    const unsigned int arrive_mean_;
    const unsigned int depart_mean_;
    const unsigned int index_;

    std::string compute_move(direction_t direction);
    std::string random_move();
};

#endif
