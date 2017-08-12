#ifndef SAND_HPP
#define SAND_HPP

#include <string>
#include <memory>

#include "warped.hpp"

WARPED_DEFINE_LP_STATE_STRUCT(SandState) {

    unsigned int z_;
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

    std::string     receiver_name_;
    unsigned int    event_ts_;

    WARPED_REGISTER_SERIALIZABLE_MEMBERS(cereal::base_class<warped::Event>(this),
                                            receiver_name_, event_ts_)
};


class Site : public warped::LogicalProcess {
public:
    Site(   const std::string&  name,
            unsigned int        num_cells_x,
            unsigned int        num_cells_y,
            unsigned int        index,
            unsigned int        z,
            unsigned int        z_threshold     )

        :   LogicalProcess(name),
            state_(),
            index_(index),
            num_cells_x_(num_cells_x),
            num_cells_y_(num_cells_y),
            z_threshold_(z_threshold) {

        /* Initialize the state variable */
        state_.z_ = z;
    }

    virtual warped::LPState& getState() { return state_; }

    virtual std::vector<std::shared_ptr<warped::Event> > initializeLP() override;

    virtual std::vector<std::shared_ptr<warped::Event> > receiveEvent(const warped::Event&);

    SandState state_;
    unsigned int index_;

protected:
    unsigned int num_cells_x_;
    unsigned int num_cells_y_;
    unsigned int z_threshold_;

    unsigned int neighbor( unsigned int index, direction_t direction );

};

#endif
