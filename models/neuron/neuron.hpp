/*  This model simulated how signals flow across a network of neurons. It has been modelled
    using details from the following paper:
      - https://www.neuron.yale.edu/neuron/static/papers/discrete/discrete_event_poster.pdf
 */
#ifndef NEURON_HPP
#define NEURON_HPP

#include <string>
#include <vector>
#include <memory>
#include <random>
#include "warped.hpp"

WARPED_DEFINE_LP_STATE_STRUCT(CellState) {

    double membrane_potential_;
    unsigned int latest_update_ts_;
    unsigned int num_spikes_;
    std::unordered_map<std::string, double> neighbors_;
};

class CellEvent : public warped::Event {
public:
    CellEvent() = default;
    CellEvent(  const std::string& receiver_cell,
                const unsigned int timestamp,
                const double weight     )
        :   receiver_cell_(receiver_cell),
            ts_(timestamp),
            weight_(weight) {}

    const std::string& receiverName() const { return receiver_cell_; }

    unsigned int timestamp() const { return ts_; }

    unsigned int size() const { return receiver_cell_.length() + sizeof(ts_) + sizeof(weight_); }

    std::string receiver_cell_;
    unsigned int ts_;
    double weight_;

    WARPED_REGISTER_SERIALIZABLE_MEMBERS(
            cereal::base_class<warped::Event>(this), receiver_cell_, ts_, weight_)
};

class Cell : public warped::LogicalProcess {
public:

    /* NOTE : It has been assumed that the neuron config files don't have repetitive names */
    Cell(   const std::string& name,
            double connection_threshold,
            double membrane_time_const,
            unsigned int refractory_period,
            double membrane_potential   )

        :   LogicalProcess(name),
            state_(),
            connection_threshold_(connection_threshold),
            membrane_time_const_(membrane_time_const),
            refractory_period_(refractory_period)   {

        state_.membrane_potential_  = membrane_potential;
        state_.latest_update_ts_    = 0;
        state_.num_spikes_          = 0;
    }

    virtual std::vector<std::shared_ptr<warped::Event> > initializeLP() override;

    virtual std::vector<std::shared_ptr<warped::Event> > receiveEvent(const warped::Event&) override;

    virtual warped::LPState& getState() override { return this->state_; }

    unsigned int spikeCount() { return this->state_.num_spikes_; }

    void addNeighbor( std::string name, double weight ) {
        state_.neighbors_.emplace(name, weight);
    }

    CellState state_;
    double connection_threshold_ = 0;

protected:
    double membrane_time_const_     = 0;
    unsigned int refractory_period_ = 0;
};

#endif // NEURON_HPP
