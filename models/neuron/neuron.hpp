#ifndef NEURON_HPP
#define NEURON_HPP

#include <string>
#include <vector>
#include <memory>
#include <random>
#include "warped.hpp"

WARPED_DEFINE_LP_STATE_STRUCT(CellState) {

    double membrane_potential_;
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
    Cell( const std::string& name )
        :   LogicalProcess(name), 
            state_() {

        state_.membrane_potential_ = 0;
    }

    virtual std::vector<std::shared_ptr<warped::Event> > initializeLP() override;
    virtual std::vector<std::shared_ptr<warped::Event> > receiveEvent(const warped::Event&);
    virtual warped::LPState& getState() { return this->state_; }

    void addNeighbor( std::string name, double weight ) {
        neighbors_.push_back(std::make_pair(name, weight));
    }

    CellState state_;

protected:
    std::vector<std::pair<std::string, double>> neighbors_;
};

#endif // NEURON_HPP
