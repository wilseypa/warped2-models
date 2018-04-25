/* Simulation of a neural network - read the header and README for further documentation */

#include "neuron.hpp"
#include <fstream>
#include "ppm/ppm.hpp"
#include "tclap/ValueArg.h"
#include <cassert>

#define INIT_SPIKE_FACTOR 10

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(CellEvent)

std::vector<std::shared_ptr<warped::Event> > Cell::initializeLP() {

    std::vector<std::shared_ptr<warped::Event> > events;

    /* Don't send any event if membrane_potential < 1 */
    if (this->state_.membrane_potential_ < 1) { return events; }

    /* Send spike events to all connected neighbors with receive time = refractory period */
    for (auto neighbor : this->state_.neighbors_) {
        // TODO : Update the weight for the connected neighbors and send that
        /* Current weight update is +1% of 1 - weight for positives and 
           +2% of 1 + weight for negatives */
        if (neighbor.second >= 0) neighbor.second += (1 - neighbor.second) * .01;
        if (neighbor.second < 0) neighbor.second += (1 + neighbor.second) * .02;
        events.emplace_back(new CellEvent {neighbor.first, this->refractory_period_, neighbor.second});
    }

    /* Send the refractory self-event with receive time = refractory period */
    events.emplace_back(new CellEvent {this->name_, this->refractory_period_, 0});

    return events;
}

std::vector<std::shared_ptr<warped::Event> > Cell::receiveEvent(const warped::Event& event) {

    std::vector<std::shared_ptr<warped::Event> > response_events;
    auto received_event = static_cast<const CellEvent&>(event);

    /* Check whether it is a refractory self-event */
    if (received_event.sender_name_ == received_event.receiverName()) {
        this->state_.membrane_potential_ = 0;
        this->state_.latest_update_ts_ = received_event.timestamp() + this->refractory_period_;
        return response_events;
    }

    /* Check if there is any connection for a spike to occur*/
    auto it = this->state_.neighbors_.find(received_event.sender_name_);
    assert(it != this->state_.neighbors_.end());
    it->second = received_event.weight_;
    if (received_event.weight_ < this->connection_threshold_) {
        return response_events;
    }

    /* Check if the neuron is responsive */
    if (this->state_.membrane_potential_ < 1.0) {

        /* IntFire1 : Basic Integrate and Fire Model */
        double delta_t = received_event.timestamp() - this->state_.latest_update_ts_;
        this->state_.membrane_potential_ *= exp(-delta_t/membrane_time_const_);
        if (received_event.weight_ >= 0) {
            this->state_.membrane_potential_ += received_event.weight_;
        }
        this->state_.latest_update_ts_ = received_event.timestamp();
    }

    /* Check if neuron is about to fire */
    if (this->state_.membrane_potential_ >= 1.0) {

        this->state_.num_spikes_++;
        unsigned int ts = received_event.timestamp() + this->refractory_period_;

        /* Send spike events to all connected neighbors with receive time = current time + ts */
        for (auto neighbor : this->state_.neighbors_) {
            // TODO : Update the weight for the connected neighbors and send that
            /* Current weight update is +1% of 1 - weight for positives and 
               +2% of 1 + weight for negatives */
            if (neighbor.second >= 0) neighbor.second += (1 - neighbor.second) * .01;
            if (neighbor.second < 0) neighbor.second += (1 + neighbor.second) * .02;
            response_events.emplace_back(new CellEvent {neighbor.first, ts, neighbor.second});
        }

        /* Send the refractory self-event with receive time = current time + ts */
        response_events.emplace_back(new CellEvent {this->name_, ts, 0});
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

    std::default_random_engine gen;
    std::uniform_int_distribution<int> mem_potential(0, INIT_SPIKE_FACTOR);

    std::vector<Cell> lps;
    unsigned int row_index = 0;
    while (getline(file_stream, buffer)) {
        auto name = buffer + "_" + std::to_string(row_index++);
        double membrane_potential = (static_cast<double> (mem_potential(gen))) / INIT_SPIKE_FACTOR;
        lps.emplace_back(name, connection_threshold, membrane_time_const, refractory_period, membrane_potential);
    }
    file_stream.close();

    /* Read the connection matrix and create the network for the neurons */
    buffer = study_name + "/connection_matrix.txt";
    file_stream.open(buffer);
    if (!file_stream.is_open()) {
        std::cerr << "Invalid connection matrix file - " << buffer << std::endl;
        return 0;
    }
    row_index = 0;
    while (getline(file_stream, buffer)) {
        size_t pos = 0;
        unsigned int col_index = 0;

        while ((pos = buffer.find_first_of(" ", 0)) != std::string::npos) {

            std::string::size_type sz;
            auto weight = std::stod(buffer.substr(0, pos), &sz);

            /* Note: Negative weight indicates inhibitory neurons
                     Positive weight indicates excitatory neurons
             */
            if (row_index != col_index) {
                lps[row_index].addNeighbor(lps[col_index].name_, weight);
            }
            buffer = buffer.substr(pos+1);
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

    /** Post-simulation statistics **/

    /* PPM spike heatmap setup*/
    auto cols = std::ceil( sqrt(lps.size()) );
    auto rows = (lps.size()+cols-1)/cols;
    auto spikes_hmap = new ppm(cols, rows);

    unsigned int num_active_neurons = 0, num_spikes = 0, max_spikes = 0;
    for (auto& lp : lps) {
        num_active_neurons += lp.spikeCount() ? 1 : 0;
        num_spikes += lp.spikeCount();
        if (lp.spikeCount() > max_spikes) {max_spikes = lp.spikeCount();}
    }
    std::cout   << num_active_neurons << " out of " << lps.size()
                << " neurons fired a total of " << num_spikes << " spikes."
                << std::endl
                << "neuron with the most spikes fired " << max_spikes << " times."
                << std::endl;

    /* Color heatmap red based on ratio of lp spikes versus the lp with the most spikes*/
    for (unsigned int i = 0; i < lps.size(); i++) {
        double ratio = ((double)lps[i].spikeCount()) / max_spikes;

        /*Color red if the neuron spiked more than half as much as the neuron with the most spikes*/
        if (ratio >= 0.5) {
            ratio = (ratio - 0.5) / 0.5;
            spikes_hmap->r[i] = ratio * 255;
        }

        /*Color green if the neuron spiked from 25% to 75% as much as the max spiked neuron*/
        /*Green + Red (from above) = Yellow*/
        if (ratio >= 0.25 && ratio <= 0.75) {
            ratio = (ratio - 0.25) / 0.5;
            spikes_hmap->g[i] = ratio * 255;
        }

        /*Color blue if the neuron spiked from 50% to 0% as much as the max spiked neuron*/
        /*Green (from above) + Blue should be cyan but in this map it is very dark*/
        if (ratio < 0.5) {
            ratio = (ratio * 2);
            spikes_hmap->b[i] = (1.0 - ratio) * 255;
        }
    }
    spikes_hmap->write("neuron_spikes_hmap.ppm");
    delete spikes_hmap;

    /* Temporary debugging statistics */
    std::ofstream LPDiag;
    LPDiag.open("Diagnostics/LPData.txt");
    for (auto lp : lps) {
        LPDiag << lp.state_.membrane_potential_ 
            << " " << lp.state_.latest_update_ts_ 
            << " " << lp.spikeCount() << std::endl;
}
    LPDiag.close();
    LPDiag.open("Diagnostics/NeighborData.txt");
    for (auto lp : lps) {
        for (auto neighbor : lp.state_.neighbors_) {
            LPDiag << neighbor.first << " " << neighbor.second << std::endl;
        }
        LPDiag << "Next LP:" << std::endl;
    }
    return 0;
}

