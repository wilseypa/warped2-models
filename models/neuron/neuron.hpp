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
};

class Spike : public warped::Event {
public:
    Spike() = default;

    Spike(const std::string& receiver_cell, const unsigned int timestamp)
        : receiver_cell_(receiver_cell), ts_(timestamp) {}

    const std::string& receiverName() const { return receiver_cell_; }

    unsigned int timestamp() const { return ts_; }

    unsigned int size() const { return receiver_cell_.length() + sizeof(ts_); }

private:
    std::string receiver_cell_;
    unsigned int ts_;
};

class Cell : public warped::LogicalProcess {
public:

    /* NOTE : It has been assumed that the neuron config files don't have repetitive names */
    Cell( const std::string& name, double membrane_time_const )
        :   LogicalProcess(name),
            membrane_time_const_(membrane_time_const),
            state_() {

        state_.membrane_potential_  = 0;
        state_.latest_update_ts_    = 0;
    }

    virtual std::vector<std::shared_ptr<warped::Event> > initializeLP() override;
    virtual std::vector<std::shared_ptr<warped::Event> > receiveEvent(const warped::Event&);
    virtual warped::LPState& getState() { return this->state_; }

    void addNeighbor( std::string name, double weight ) {
        neighbors_.emplace(name, weight);
    }

protected:
    double membrane_time_const_ = 0;
    std::unordered_map<std::string, double> neighbors_;
    CellState state_;
};

#endif // NEURON_HPP
