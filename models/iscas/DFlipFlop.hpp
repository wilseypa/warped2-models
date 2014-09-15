#ifndef D_FLIP_FLOP_HPP
#define D_FLIP_FLOP_HPP

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "warped.hpp"

#include "Component.hpp"

WARPED_DEFINE_OBJECT_STATE_STRUCT(DFlipFlopState) {
    bool input;
    bool previous_output;
};

class DFlipFlop : public Component {
public:
    DFlipFlop(std::string name, unsigned int propagation_delay, unsigned int clock_period);

    bool needsClock() const { return true; }
    warped::ObjectState& getState() { return state_; }

    std::vector<std::shared_ptr<warped::Event>> receiveEvent(const warped::Event& event);

private:
    DFlipFlopState state_;

    bool computeOutput();
};

#endif
