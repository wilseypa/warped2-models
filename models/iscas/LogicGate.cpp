#include "LogicGate.hpp"

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "warped.hpp"
#include "utility/memory.hpp"

#include "Component.hpp"
#include "events.hpp"

LogicGate::LogicGate(std::string name, unsigned int propagation_delay,
                     unsigned int clock_period, Type type)
    : Component(name, propagation_delay, clock_period), type_(type), state_() {}

std::vector<std::unique_ptr<warped::Event>> LogicGate::receiveEvent(const warped::Event& event) {
    std::vector<std::unique_ptr<warped::Event>> v;

    auto signal_event = dynamic_cast<const SignalEvent*>(&event);
    if (!signal_event) {
        throw std::runtime_error("Received Invalid Evnet");
    }
    // The output only changes if the input changes
    if (state_.inputs[signal_event->input_index_] != signal_event->input_value_) {
        state_.inputs[signal_event->input_index_] = signal_event->input_value_;
        // Send the new output signal to all connected component
        auto receive_time = event.timestamp() + propagation_delay_;
        for (const auto& it : outputs_) {
            v.emplace_back(make_unique<SignalEvent>(it.first, receive_time, it.second, computeOutput()));
        }
    }

    return v;
}

unsigned int LogicGate::addInput() {
    if (type_ == Type::NOT && state_.inputs.size() >= 1) {
        throw std::runtime_error(std::string("NOT Gates only support one input"));
    }
    state_.inputs.push_back(false);
    return state_.inputs.size() - 1;
}

bool LogicGate::computeOutput() {
    switch (type_) {
    case Type::NOT:
        return !(state_.inputs[0]);
    case Type::AND:
        for (bool input : state_.inputs) {
            if (!input) {
                return false;
            }
        }
        return true;
    case Type::OR:
        for (bool input : state_.inputs) {
            if (input) {
                return true;
            }
        }
        return false;
    case Type::XOR: {
        // There are multiple types of N-input XOR gates.
        // This one uses parity style, which is much more common.
        bool result = false;
        for (bool input : state_.inputs) {
            result ^= input;
        }
        return result;
    }
    case Type::NAND:
        for (bool input : state_.inputs) {
            if (!input) {
                return true;
            }
        }
        return false;
    case Type::NOR:
        for (bool input : state_.inputs) {
            if (input) {
                return false;
            }
        }
        return true;
    default: return false;
    }
}
