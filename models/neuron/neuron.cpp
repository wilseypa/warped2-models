/* Implementation of a Neural Network Simulation */

#include "neuron.hpp"
#include <fstream>
#include "tclap/ValueArg.h"
#include <cassert>

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(Spike)

std::vector<std::shared_ptr<warped::Event> > Cell::initializeLP() {

    std::vector<std::shared_ptr<warped::Event> > events;
    return events;
}

std::vector<std::shared_ptr<warped::Event> > Cell::receiveEvent(const warped::Event& event) {

    std::vector<std::shared_ptr<warped::Event> > response_events;
    auto received_event = static_cast<const Spike&>(event);

    // Add details of the paper
    double delta_t = received_event.timestamp() - this->state_.latest_update_ts_;
    auto it = this->neighbors_.find(event.sender_name_);
    assert(it != this->neighbors_.end());

    this->state_.membrane_potential_ *= exp(-delta_t/membrane_time_const_);
    this->state_.membrane_potential_ += it->second;
    this->state_.latest_update_ts_ = received_event.timestamp();

    if (this->state_.membrane_potential_ >= 1.0) {
        // Send event to neighboring neurons
        this->state_.membrane_potential_ = 0; 
    }

    // Send refractory event

    return response_events;
}

int main(int argc, const char** argv) {

    /* Read the command line arguments for study name and connection threshold */
    std::string study_name          = "ADHD200_CC200";
    double connection_threshold     = 0.0;
    double membrane_time_const      = 10.0; /* Unit is milli seconds */

    TCLAP::ValueArg<std::string> study_name_arg( "", "study-name",
                                    "Name of the study", false, study_name, "string");

    TCLAP::ValueArg<double> connection_threshold_arg( "", "connection-threshold",
            "Threshold of connection strength", false, connection_threshold, "double");

    TCLAP::ValueArg<double> membrane_time_const_arg( "", "membrane-time-const",
            "Membrane Time Constant (in milli seconds)", false, membrane_time_const, "double");

    std::vector<TCLAP::Arg*> args = {   &study_name_arg,
                                        &connection_threshold_arg,
                                        &membrane_time_const_arg
                                    };

    warped::Simulation neuron_sim {"Neural Network Simulation", argc, argv, args};

    study_name              = study_name_arg.getValue();
    connection_threshold    = connection_threshold_arg.getValue();
    membrane_time_const     = membrane_time_const_arg.getValue();

    /* Read the abbreviated file for neuron names and create the LPs */
    std::ifstream file_stream;
    std::string buffer = study_name + "/names.txt";
    file_stream.open(buffer);
    if (!file_stream.is_open()) {
        std::cerr << "Invalid names file - " << buffer << std::endl;
        return 0;
    }
    unsigned int neuron_count = 0;
    std::vector<Cell> lps;
    while (getline(file_stream, buffer)) {
        lps.emplace_back(buffer, membrane_time_const);
        neuron_count++;
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
            if ((row_index != col_index) && (weight > connection_threshold)) {
                lps[row_index].addNeighbor(lps[col_index].name_, weight);
            }
            buffer.substr(pos+1);
            col_index++;
        }
        row_index++;
    }
    file_stream.close();

    return 0;
}

