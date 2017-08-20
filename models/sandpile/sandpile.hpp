#ifndef SAND_HPP
#define SAND_HPP

#include <string>
#include <memory>

#include "warped.hpp"

WARPED_DEFINE_LP_STATE_STRUCT(VertexState) {

    unsigned int sandpile_height_ = 0;
    unsigned int collapse_cnt_ = 0;
};

enum direction_t {
    LEFT,
    RIGHT,
    DOWN,
    UP
};

class SandEvent : public warped::Event {
public:
    SandEvent() = default;

    SandEvent(  const std::string   receiver_name,
                unsigned int        event_ts  )

        :   receiver_name_(receiver_name),
            event_ts_(event_ts) {}

    const std::string& receiverName() const { return receiver_name_; }

    unsigned int timestamp() const { return event_ts_; }

    unsigned int size() const {
        return receiver_name_.length() + sizeof(event_ts_);
    }

    std::string     receiver_name_;
    unsigned int    event_ts_;

    WARPED_REGISTER_SERIALIZABLE_MEMBERS(cereal::base_class<warped::Event>(this),
                                            receiver_name_, event_ts_)
};


class Vertex : public warped::LogicalProcess {
public:
    Vertex( const std::string&  name,
            unsigned int        grid_dimension,
            unsigned int        index,
            unsigned int        sandpile_height )

        :   LogicalProcess(name),
            state_(),
            index_(index),
            grid_dimension_(grid_dimension) {

        /* Initialize the pile height (state variable) */
        state_.sandpile_height_ = sandpile_height;
    }

    virtual warped::LPState& getState() { return state_; }

    virtual std::vector<std::shared_ptr<warped::Event> > initializeLP() override;

    virtual std::vector<std::shared_ptr<warped::Event> > receiveEvent(const warped::Event&);

    VertexState state_;
    unsigned int index_;

protected:
    unsigned int grid_dimension_;

    unsigned int neighbor( unsigned int index, direction_t direction );

};

#endif
