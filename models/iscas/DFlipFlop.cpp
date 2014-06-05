#include "DFlipFlop.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "warped.hpp"
#include "utility/memory.hpp"

#include "Component.hpp"
#include "events.hpp"

DFlipFlop::DFlipFlop(std::string name, unsigned int propagation_delay, unsigned int clock_period)
    : Component(name, propagation_delay, clock_period), state_() {}

std::vector<std::unique_ptr<warped::Event>> DFlipFlop::receiveEvent(const warped::Event& event) {
    std::vector<std::unique_ptr<warped::Event>> v;

    if (auto signal_event = dynamic_cast<const SignalEvent*>(&event)) {
        state_.input = signal_event->input_value_;
    } else if (dynamic_cast<const ClockEvent*>(&event)) {
        auto current_time = event.timestamp();
        if (state_.input != state_.previous_output) {
            state_.previous_output = state_.input;
            // Send the new output signal to all connected components
            auto receive_time = event.timestamp() + propagation_delay_;
            for (const auto& it : outputs_) {
                v.emplace_back(make_unique<SignalEvent>(it.first, receive_time, it.second, state_.input));
            }
        }

        // Schedule another clock tick
        v.emplace_back(make_unique<ClockEvent>(name_, current_time + clock_period_));
    }

    return v;
}