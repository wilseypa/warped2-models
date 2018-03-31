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

enum cell_event_t {

    SPIKE,
    UPDATE
};

class CellEvent : public warped::Event {
public:
    CellEvent() = default;
    CellEvent(  const std::string& receiver_cell,
                const cell_event_t type,
                const unsigned int timestamp    )
        :   receiver_cell_(receiver_cell),
            type_(type),
            ts_(timestamp) {}

    const std::string& receiverName() const { return receiver_cell_; }

    cell_event_t type() const { return type_; }

    unsigned int timestamp() const { return ts_; }

    unsigned int size() const { return receiver_cell_.length() +
                                sizeof(ts_) +
                                sizeof(type_);
    }

private:
    std::string receiver_cell_;
    cell_event_t type_;
    unsigned int ts_;
};

class Cell : public warped::LogicalProcess {
public:

    /* NOTE : It has been assumed that the neuron config files don't have repetitive names */
    Cell(   const std::string& name,
            double membrane_time_const,
            unsigned int refractory_period  )

        :   LogicalProcess(name),
            membrane_time_const_(membrane_time_const),
            refractory_period_(refractory_period),
            state_() {

        state_.membrane_potential_  = 0;
        state_.latest_update_ts_    = 0;
        state_.num_spikes_          = 0;
    }

    virtual std::vector<std::shared_ptr<warped::Event> > initializeLP() override;

    virtual std::vector<std::shared_ptr<warped::Event> > receiveEvent(const warped::Event&);

    virtual warped::LPState& getState() { return this->state_; }

    unsigned int spikeCount() { return this->state_.num_spikes_; }

    void addNeighbor( std::string name, double weight ) {
        state_.neighbors_.emplace(name, weight);
    }

protected:
    double membrane_time_const_     = 0;
    unsigned int refractory_period_ = 0;
    CellState state_;
};

#endif // NEURON_HPP
