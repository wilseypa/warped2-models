/* Simulation of a neural network - read the header and README for further documentation */

#include "neuron.hpp"
#include <fstream>
#include "tclap/ValueArg.h"
#include <cassert>

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(Spike)

std::vector<std::shared_ptr<warped::Event> > Cell::initializeLP() {

    std::vector<std::shared_ptr<warped::Event> > events;

    /* Send a spike event with receive time = 1 to any one neighbor (if available) */
    for (auto neighbor : this->neighbors_) {
        events.emplace_back(new Spike {neighbor.first, 1});
        break;
    }
    return events;
}

std::vector<std::shared_ptr<warped::Event> > Cell::receiveEvent(const warped::Event& event) {

    std::vector<std::shared_ptr<warped::Event> > response_events;
    auto received_event = static_cast<const Spike&>(event);

    /* Check whether it is a refractory self-event */
    if (received_event.sender_name_ == received_event.receiverName()) {

        this->state_.membrane_potential_    = 0;
        this->state_.latest_update_ts_      = received_event.timestamp();

    } else if (this->state_.membrane_potential_ < 1.0) { /* Check if the neuron is responsive */

        /* IntFire1 : Basic Integrate and Fire Model */
        auto it = this->neighbors_.find(event.sender_name_);
        assert(it != this->neighbors_.end());

        double delta_t = received_event.timestamp() - this->state_.latest_update_ts_;
        this->state_.membrane_potential_ *= exp(-delta_t/membrane_time_const_);
        this->state_.membrane_potential_ += it->second;
        this->state_.latest_update_ts_ = received_event.timestamp();

        if (this->state_.membrane_potential_ >= 1.0) {

            this->state_.membrane_potential_ = 2.0;
            this->state_.num_spikes_++;

            unsigned int ts = received_event.timestamp() + this->refractory_period_;

            /* Send the refractory self-event with receive time = ts */
            response_events.emplace_back(new Spike {this->name_, ts});

            /* Send spike events to all its connected neighbors with receive time = ts */
            for (auto neighbor : this->neighbors_) {
                response_events.emplace_back(new Spike {neighbor.first, ts});
            }
        }
    }
    return response_events;
}

int main(int argc, const char** argv) {

    /* Read the command line arguments for study name and connection threshold */
    std::string study_name          = "ADHD200_CC200";
    double connection_threshold     = 0.0;
    double membrane_time_const      = 10.0; /* Unit is msecs */
    unsigned int refractory_period  = 5;    /* Unit is msecs */

    TCLAP::ValueArg<std::string> study_name_arg( "", "study-name",
                                    "Name of the study", false, study_name, "string");

    TCLAP::ValueArg<double> connection_threshold_arg( "", "connection-threshold",
            "Threshold of connection strength", false, connection_threshold, "double");

    TCLAP::ValueArg<double> membrane_time_const_arg( "", "membrane-time-const",
            "Membrane Time Constant (in milli seconds)", false, membrane_time_const, "double");

    TCLAP::ValueArg<unsigned int> refractory_period_arg( "", "refractory-period",
            "Refractory Period (in milli seconds)", false, refractory_period, "unsigned int");

    std::vector<TCLAP::Arg*> args = {   &study_name_arg,
                                        &connection_threshold_arg,
                                        &membrane_time_const_arg,
                                        &refractory_period_arg
                                    };

    warped::Simulation neuron_sim {"Neural Network Simulation", argc, argv, args};

    study_name              = study_name_arg.getValue();
    connection_threshold    = connection_threshold_arg.getValue();
    membrane_time_const     = membrane_time_const_arg.getValue();
    refractory_period       = refractory_period_arg.getValue();

    /* Read the abbreviated file for neuron names and create the LPs */
    std::ifstream file_stream;
    std::string buffer = study_name + "/names.txt";
    file_stream.open(buffer);
    if (!file_stream.is_open()) {
        std::cerr << "Invalid names file - " << buffer << std::endl;
        return 0;
    }
    std::vector<Cell> lps;
    while (getline(file_stream, buffer)) {
        lps.emplace_back(buffer, membrane_time_const, refractory_period);
    }
    file_stream.close();

    /* Read the connection matrix and create the network for the neurons */
    buffer = study_name + "/connection_matrix.txt";
    file_stream.open(buffer);
    if (!file_stream.is_open()) {
        std::cerr << "Invalid connection matrix file - " << buffer << std::endl;
        return 0;
    }
    unsigned int row_index = 0;
    while (getline(file_stream, buffer)) {
        size_t pos = 0;
        unsigned int col_index = 0;

        while ((pos = buffer.find_first_of(" ", 0)) != std::string::npos) {

            std::string::size_type sz;
            auto weight = std::stod(buffer.substr(0, pos), &sz);

            /* Note: Negative weight indicates inhibitory neurons
                     Positive weight indicates excitatory neurons
             */
            if ((row_index != col_index) && (weight > connection_threshold)) {
                lps[row_index].addNeighbor(lps[col_index].name_, weight);
            }
            buffer.substr(pos+1);
            col_index++;
        }
        row_index++;
    }
    file_stream.close();

    /* Simulate the neuron model */
    std::vector<warped::LogicalProcess*> lp_pointers;
    for (auto& lp : lps) {
        lp_pointers.push_back(&lp);
    }
    neuron_sim.simulate(lp_pointers);

    /* Post-simulation statistics */
    unsigned int num_active_neurons = 0, num_spikes = 0;
    for (auto& lp : lps) {
        num_active_neurons += lp.spikeCount() ? 1 : 0;
        num_spikes += lp.spikeCount();
    }
    std::cout   << num_active_neurons << " out of " << lps.size()
                << " neurons fired a total of " << num_spikes << " spikes."
                << std::endl;

    return 0;
}

