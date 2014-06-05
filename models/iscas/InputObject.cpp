#include "InputObject.hpp"

#include <functional>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "warped.hpp"
#include "utility/memory.hpp"

#include "Component.hpp"
#include "events.hpp"


InputObject::InputObject(std::string name, unsigned int propagation_delay,
                         unsigned int clock_period)
    :  Component(name, propagation_delay, clock_period), distribution_(0, 1), state_(name) {}

std::vector<std::unique_ptr<warped::Event>> InputObject::receiveEvent(const warped::Event& event) {
    std::vector<std::unique_ptr<warped::Event>> v;

    if (dynamic_cast<const ClockEvent*>(&event)) {
        auto current_time = event.timestamp();

        // Send a random signal to all connected components
        auto receive_time = current_time + propagation_delay_;
        bool value = distribution_(state_.generator) == 1;
        for (auto& it : outputs_) {
            v.emplace_back(make_unique<SignalEvent>(it.first, receive_time, it.second, value));
        }
        // Schedule another clock tick
        v.emplace_back(make_unique<ClockEvent>(name_, current_time + clock_period_));
    }

    return v;
}

