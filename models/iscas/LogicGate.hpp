#ifndef LOGIC_GATE_HPP
#define LOGIC_GATE_HPP

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "warped.hpp"

#include "Component.hpp"

WARPED_DEFINE_OBJECT_STATE_STRUCT(LogicGateState) {
    std::vector<bool> inputs;

    LogicGateState() : inputs() {}
};

class LogicGate : public Component {
public:
    enum class Type {NOT, AND, OR, XOR, NAND, NOR};

    LogicGate(std::string name, unsigned int propagation_delay,
              unsigned int clock_period, Type type);

    bool needsClock() const { return false; }
    warped::ObjectState& getState() { return state_; }

    std::vector<std::shared_ptr<warped::Event>> receiveEvent(const warped::Event& event);

    // Add a new input and return its index
    input_index_t addInput();

private:
    const Type type_;
    LogicGateState state_;

    bool computeOutput();
};

#endif
