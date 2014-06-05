#ifndef ISCAS_COMPONENT_HPP
#define ISCAS_COMPONENT_HPP

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "warped.hpp"
#include "utility/memory.hpp"

#include "events.hpp"

class Component : public warped::SimulationObject {
public:
    typedef unsigned int input_index_t;
    Component(const std::string& name, unsigned int propagation_delay, unsigned int clock_period)
        : SimulationObject(name), propagation_delay_(propagation_delay),
          clock_period_(clock_period), outputs_() {}

    // Indicates whether of not objects of a class should receive clock tick events.
    virtual bool needsClock() const = 0;

    void addOutput(std::string receiver, input_index_t receiver_input) {
        outputs_.emplace_back(receiver, receiver_input);
    }

    std::vector<std::unique_ptr<warped::Event>> createInitialEvents()  {
        std::vector<std::unique_ptr<warped::Event>> v;

        // If necessary, start the clock ticks
        if (needsClock()) {
            v.emplace_back(make_unique<ClockEvent>(name_, clock_period_));
        }
        return v;
    }

protected:
    const unsigned int propagation_delay_;
    const unsigned int clock_period_;
    std::vector<std::pair<std::string, input_index_t>> outputs_;
};

#endif
