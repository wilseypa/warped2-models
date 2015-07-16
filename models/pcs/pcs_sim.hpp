#ifndef PCS_HPP
#define PCS_HPP

#include <string>
#include <vector>
#include <memory>
#include <random>

#include "warped.hpp"

WARPED_DEFINE_OBJECT_STATE_STRUCT(PcsState) {

    unsigned int normal_channels_;
    unsigned int reserve_channels_;
    unsigned int call_attempts_;
    unsigned int channel_blocks_;
    unsigned int busy_lines_;
    unsigned int handoff_blocks_;
};

enum event_type_t {

    CALL_ARRIVED,
    CALL_COMPLETED,
    MOVE_CALL_IN,
    MOVE_CALL_OUT
};

enum channel_type_t {

    NONE,
    NORMAL,
    RESERVE
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
                event_type_t        event_type, 
                channel_type_t      channel_type    )

        :   receiver_name_(receiver_name), 
            event_timestamp_(timestamp), 
            call_arrival_ts_(call_arrival_ts), 
            event_type_(event_type), 
            channel_type_(channel_type) {}

    const std::string& receiverName() const { return receiver_name_; }
    unsigned int timestamp() const { return event_timestamp_; }

    std::string     receiver_name_;
    unsigned int    event_timestamp_;
    unsigned int    call_arrival_ts_;
    event_type_t    event_type_;
    channel_type_t  channel_type_;

    WARPED_REGISTER_SERIALIZABLE_MEMBERS(cereal::base_class<warped::Event>(this), 
                                            receiver_name_, event_timestamp_, event_type_, 
                                            channel_type_, call_arrival_ts_)
};

class PcsCell : public warped::SimulationObject {
public:

    PcsCell(    const std::string&  name, 
                unsigned int        num_cells_x, 
                unsigned int        num_cells_y, 
                unsigned int        max_normal_ch_cnt, 
                unsigned int        max_reserve_ch_cnt, 
                unsigned int        call_interval_mean, 
                unsigned int        call_duration_mean, 
                unsigned int        move_interval_mean, 
                unsigned int        portable_cnt, 
                unsigned int        index      )

        :   SimulationObject(name), 
            state_(), 
            num_cells_x_(num_cells_x), 
            num_cells_y_(num_cells_y), 
            max_normal_ch_cnt_(max_normal_ch_cnt), 
            max_reserve_ch_cnt_(max_reserve_ch_cnt), 
            call_interval_mean_(call_interval_mean), 
            call_duration_mean_(call_duration_mean), 
            move_interval_mean_(move_interval_mean),
            portable_init_cnt_(portable_cnt),
            index_(index),
            rng_(new std::default_random_engine(index)) {

        // Update the state variables
        state_.normal_channels_  = max_normal_ch_cnt_;
        state_.reserve_channels_ = max_reserve_ch_cnt_;
        state_.call_attempts_    = 0;
        state_.channel_blocks_   = 0;
        state_.busy_lines_       = 0;
        state_.handoff_blocks_   = 0;
    }

    virtual warped::ObjectState& getState() { return state_; }

    virtual std::vector<std::shared_ptr<warped::Event> > initializeObject() override;

    virtual std::vector<std::shared_ptr<warped::Event> > receiveEvent(const warped::Event&);

    PcsState state_;

protected:

    unsigned int num_cells_x_;
    unsigned int num_cells_y_;
    unsigned int max_normal_ch_cnt_;
    unsigned int max_reserve_ch_cnt_;
    unsigned int call_interval_mean_;
    unsigned int call_duration_mean_;
    unsigned int move_interval_mean_;
    unsigned int portable_init_cnt_;
    unsigned int index_;

    std::shared_ptr<std::default_random_engine> rng_;

    std::string compute_move(direction_t direction);
    std::string random_move();
};

#endif
