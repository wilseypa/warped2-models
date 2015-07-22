#ifndef PCS_HPP
#define PCS_HPP

#include <string>
#include <vector>
#include <memory>
#include <random>

#include "warped.hpp"

WARPED_DEFINE_OBJECT_STATE_STRUCT(PcsState) {

    unsigned int idle_channel_cnt_;
    unsigned int call_attempts_;
    unsigned int channel_blocks_;
    unsigned int handoff_blocks_;
};

enum method_t {

    NEXT_CALL_METHOD, 
    COMPLETE_CALL_METHOD, 
    MOVE_CALL_IN_METHOD, 
    MOVE_CALL_OUT_METHOD
};

enum action_t {

    NEXTCALL, 
    COMPLETECALL, 
    MOVECALL
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
                unsigned int        event_ts, 
                unsigned int        complete_call_ts, 
                unsigned int        next_call_ts, 
                unsigned int        move_call_ts,
                method_t            method      )

        :   receiver_name_(receiver_name), 
            event_ts_(event_ts), 
            complete_call_ts_(complete_call_ts), 
            next_call_ts_(next_call_ts), 
            move_call_ts_(move_call_ts), 
            method_(method) {}

    const std::string& receiverName() const { return receiver_name_; }

    unsigned int timestamp() const { return event_ts_; }

    std::string     receiver_name_;
    unsigned int    event_ts_;
    unsigned int    complete_call_ts_;
    unsigned int    next_call_ts_;
    unsigned int    move_call_ts_;
    method_t        method_;

    WARPED_REGISTER_SERIALIZABLE_MEMBERS(cereal::base_class<warped::Event>(this), 
                                            receiver_name_, event_ts_, complete_call_ts_, 
                                            next_call_ts_, move_call_ts_, method_)
};

class PcsCell : public warped::SimulationObject {
public:

    PcsCell(    const std::string&  name, 
                unsigned int        num_cells_x, 
                unsigned int        num_cells_y, 
                unsigned int        max_channel_cnt, 
                unsigned int        call_interval_mean, 
                unsigned int        call_duration_mean, 
                unsigned int        move_interval_mean, 
                unsigned int        portable_cnt, 
                unsigned int        index      )

        :   SimulationObject(name), 
            state_(), 
            num_cells_x_(num_cells_x), 
            num_cells_y_(num_cells_y), 
            max_channel_cnt_(max_channel_cnt), 
            call_interval_mean_(call_interval_mean), 
            call_duration_mean_(call_duration_mean), 
            move_interval_mean_(move_interval_mean),
            portable_init_cnt_(portable_cnt),
            index_(index),
            rng_(new std::default_random_engine(index)) {

        // Update the state variables
        state_.idle_channel_cnt_ = max_channel_cnt_;
        state_.call_attempts_    = 0;
        state_.channel_blocks_   = 0;
        state_.handoff_blocks_   = 0;
    }

    virtual warped::ObjectState& getState() { return state_; }

    virtual std::vector<std::shared_ptr<warped::Event> > initializeObject() override;

    virtual std::vector<std::shared_ptr<warped::Event> > receiveEvent(const warped::Event&);

    PcsState state_;

protected:

    unsigned int num_cells_x_;
    unsigned int num_cells_y_;
    unsigned int max_channel_cnt_;
    unsigned int call_interval_mean_;
    unsigned int call_duration_mean_;
    unsigned int move_interval_mean_;
    unsigned int portable_init_cnt_;
    unsigned int index_;

    std::shared_ptr<std::default_random_engine> rng_;

    std::string compute_move(direction_t direction);
    std::string random_move();

    action_t min_ts(unsigned int complete_call_ts, 
                    unsigned int next_call_ts, 
                    unsigned int move_call_ts);
};

#endif
