#ifndef PCS_HPP
#define PCS_HPP

#include <string>
#include <vector>
#include <memory>

#include "warped.hpp"

#include "MLCG.h"
#include "NegExp.h"


WARPED_DEFINE_OBJECT_STATE_STRUCT(PcsState) {

    unsigned int idle_channel_cnt_;
};

enum event_type_t {

    CALL_ARRIVAL,
    CALL_COMPLETION,
    PORTABLE_MOVE_OUT,
    PORTABLE_MOVE_IN
};

enum direction_t {

    LEFT,
    RIGHT,
    DOWN,
    UP
};

class PcsEvent : public warped::Event {
public:

    PcsEvent() = default;

    PcsEvent(   const std::string   receiver_name, 
                unsigned int        timestamp, 
                unsigned int        call_arrival_ts, 
                event_type_t        event_type         )
        :   receiver_name_(receiver_name), 
            event_timestamp_(timestamp), 
            call_arrival_ts_(call_arrival_ts), 
            event_type_(event_type) {}

    const std::string& receiverName() const { return receiver_name_; }
    unsigned int timestamp() const { return event_timestamp_; }

    std::string     receiver_name_;
    unsigned int    event_timestamp_;
    unsigned int    call_arrival_ts_;
    event_type_t    event_type_;

    WARPED_REGISTER_SERIALIZABLE_MEMBERS(cereal::base_class<warped::Event>(this), 
                                            receiver_name_, event_timestamp_, event_type_, 
                                            call_arrival_ts_)
};

class PcsCell : public warped::SimulationObject {
public:

    PcsCell(    const std::string&  name, 
                unsigned int        num_cells_x, 
                unsigned int        num_cells_y, 
                unsigned int        channel_cnt, 
                unsigned int        call_interval_mean, 
                unsigned int        call_duration_mean, 
                unsigned int        move_interval_mean, 
                unsigned int        portable_cnt, 
                unsigned int        index      )

        :   SimulationObject(name), 
            state_(), 
            num_cells_x_(num_cells_x), 
            num_cells_y_(num_cells_y), 
            channel_cnt_(channel_cnt), 
            call_interval_mean_(call_interval_mean), 
            call_duration_mean_(call_duration_mean), 
            move_interval_mean_(move_interval_mean),
            portable_init_cnt_(portable_cnt),
            index_(index), 
            rng_(new MLCG) {

        // Update the state variables
        state_.idle_channel_cnt_ = channel_cnt_;
    }

    virtual warped::ObjectState& getState() { return state_; }

    virtual std::vector<std::shared_ptr<warped::Event> > createInitialEvents();

    virtual std::vector<std::shared_ptr<warped::Event> > receiveEvent(const warped::Event&);

protected:

    PcsState                state_;
    unsigned int            num_cells_x_;
    unsigned int            num_cells_y_;
    unsigned int            channel_cnt_;
    unsigned int            call_interval_mean_;
    unsigned int            call_duration_mean_;
    unsigned int            move_interval_mean_;
    unsigned int            portable_init_cnt_;
    unsigned int            index_;
    std::shared_ptr<MLCG>   rng_;

    std::string compute_move(direction_t direction);
    std::string random_move();
};

#endif
