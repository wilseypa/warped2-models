#ifndef INPUT_OBJECT_HPP
#define INPUT_OBJECT_HPP

#include <functional>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "warped.hpp"

#include "Component.hpp"
#include "events.hpp"


WARPED_DEFINE_OBJECT_STATE_STRUCT(InputState) {
    std::default_random_engine generator;

    InputState(std::string object_name) : generator(std::hash<std::string>()(object_name)) {}
};

class InputObject : public Component {
public:
    InputObject(std::string name, unsigned int propagation_delay, unsigned int clock_period);

    virtual bool needsClock() const { return true; }
    warped::ObjectState& getState() { return state_; }

    std::vector<std::shared_ptr<warped::Event>> receiveEvent(const warped::Event& event);

private:
    std::uniform_int_distribution<int> distribution_;
    InputState state_;
};

#endif
