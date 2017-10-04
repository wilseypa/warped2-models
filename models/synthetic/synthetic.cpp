#include <cassert>
#include <random>
#include "synthetic.hpp"
#include "network.hpp"
#include "tclap/ValueArg.h"

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(InternalEvent)
WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(ExternalEvent)

inline std::string Node::lpName(const unsigned int lp_index) {

    return std::string("Node_") + std::to_string(lp_index);
}

std::vector<std::shared_ptr<warped::Event> > Node::initializeLP() {

    std::vector<std::shared_ptr<warped::Event> > events;

    /* Register random number generator */
    registerRNG<std::default_random_engine>(rng_);

    for (unsigned int i = 0; i < num_nodes_; i++) {
        unsigned int time = send_distribution_->nextRandNum(*rng_);
        events.emplace_back(new InternalEvent {time});
    }
    return events;
}

std::vector<std::shared_ptr<warped::Event> > Node::receiveEvent(const warped::Event& event) {

    std::vector<std::shared_ptr<warped::Event> > response_events;

    /* Check if event received is a self/internal timer event */
    if (event.sender_name_ == event.receiverName()) {

        /* Restart the processing_timer/internal event */
        unsigned int ts = event.timestamp() + send_distribution_->nextRandNum(*rng_);
        response_events.emplace_back(new InternalEvent {ts});

        /* Send an external event to one of the nodes in its adjacency list.
         * Select node using chosen distribution */
        auto id = (unsigned int) node_sel_distribution_->nextRandNum(*rng_) - 1;
        // Note: 1 subtracted since min value of distribution is 1.
        response_events.emplace_back(
                new ExternalEvent { adjacency_list_[id],
                                    floating_point_ops_cnt_,
                                    event.timestamp()+1 });

    } else { /* External Event received from other LPs/nodes */
        /* Process the event */
        float val = 0.5;
        for (unsigned int i = 0; i < floating_point_ops_cnt_; i++) {
            /* A floating point operation */
            val += i;
        }

        /* TODO: Update the state, part of future design plans */
    }
    return response_events;
}

int main(int argc, const char** argv) {

    unsigned int num_nodes                  = 100000;
    std::string network                     = "Watts-Strogatz,30,0.1";
    std::string node_selection              = "exponential,3.5";
    std::string floating_point_ops_cnt      = "1000,1000";
    std::string state_size                  = "100,100";
    std::string event_send                  = "geometric,0.5,10";

    TCLAP::ValueArg<unsigned int> num_nodes_arg("", "num-nodes",
            "Number of nodes", false, num_nodes, "unsigned int");
    TCLAP::ValueArg<std::string> network_arg("", "network-params",
            "Network details - <type,param1,param2,...>", false, network, "string");
    TCLAP::ValueArg<std::string> node_selection_arg("", "node-selection-params",
            "Node selection details - <distribution-type,<distribution-params>>",
            false, node_selection, "string");
    TCLAP::ValueArg<std::string> floating_point_ops_cnt_arg("", "event-processing-time-range",
            "Event processing time (as floating-point calculation count) range - <min,max>",
            false, floating_point_ops_cnt, "string");
    TCLAP::ValueArg<std::string> state_size_arg("", "state-size-range",
            "Size range (in bytes) for the LP state - <min,max>",
            false, state_size, "string");
    TCLAP::ValueArg<std::string> event_send_arg("", "event-send-time-delta-params",
            "Event send time delta details - <dist-type,<dist-params>,ceiling-value>",
            false, event_send, "string");

    std::vector<TCLAP::Arg*> model_args = { &num_nodes_arg,
                                            &network_arg,
                                            &node_selection_arg,
                                            &floating_point_ops_cnt_arg,
                                            &state_size_arg,
                                            &event_send_arg
                                          };

    warped::Simulation synthetic_sim {"Synthetic Simulation", argc, argv, model_args};

    num_nodes                   = num_nodes_arg.getValue();
    network                     = network_arg.getValue();
    node_selection              = node_selection_arg.getValue();
    floating_point_ops_cnt      = floating_point_ops_cnt_arg.getValue();
    state_size                  = state_size_arg.getValue();
    event_send                  = event_send_arg.getValue();

    std::vector<Node> lps;
    std::vector<std::string> lp_names;

    /* Create uniform distribution for floating point operations count */
    std::string delimiter = ",";
    size_t pos = floating_point_ops_cnt.find(delimiter);
    unsigned int min_floating_point_ops_cnt =
            (unsigned int) std::stoul(floating_point_ops_cnt.substr(0, pos));
    unsigned int max_floating_point_ops_cnt =
            (unsigned int) std::stoul(floating_point_ops_cnt.substr(pos+1));
    std::uniform_int_distribution<unsigned int>
            floating_point_ops_cnt_dist(    min_floating_point_ops_cnt,
                                            max_floating_point_ops_cnt  );

    /* Create uniform distribution for the state size */
    pos = state_size.find(delimiter);
    unsigned int min_state_size = (unsigned int) std::stoul(state_size.substr(0, pos));
    unsigned int max_state_size = (unsigned int) std::stoul(state_size.substr(pos+1));
    std::uniform_int_distribution<unsigned int>
                state_size_dist( min_state_size, max_state_size );

    /* Create the LPs */
    std::default_random_engine generator (num_nodes);
    for (unsigned int i = 0; i < num_nodes; i++) {
        auto name = Node::lpName(i);
        lp_names.push_back(name);
        lps.emplace_back(   name,
                            num_nodes,
                            floating_point_ops_cnt_dist(generator),
                            state_size_dist(generator),
                            i
                        );
    }

    std::vector<warped::LogicalProcess*> lp_pointers;
    for (auto& lp : lps) {
        lp_pointers.push_back(&lp);
    }

    /* Create the Network */
    Network *ntwrk = nullptr;
    pos = network.find(delimiter);
    std::string type = network.substr(0, pos);
    network.erase(0, pos + delimiter.length());
    if (type == "Watts-Strogatz") { // If the choice is Watts-Strogatz
        pos = network.find(delimiter);
        std::string token = network.substr(0, pos);
        unsigned int k = (unsigned int) std::stoul(token);
        network.erase(0, pos + delimiter.length());
        double beta = std::stod(network);
        ntwrk = new WattsStrogatz(lp_names, k, beta);

    } else if (type == "Barabasi-Albert") { // If the choice is Barabasi-Albert
        pos = network.find(delimiter);
        std::string token = network.substr(0, pos);
        unsigned int m = (unsigned int) std::stoul(token);
        network.erase(0, pos + delimiter.length());
        double a = std::stod(network);
        ntwrk = new BarabasiAlbert(lp_names, m, a);

    } else { // Invalid choice
        std::cerr << "Invalid choice of network." << std::endl;
        abort();
    }

    /* Parse the event send distribution type and parameters */
    pos = event_send.find(delimiter);
    std::string distribution_type = event_send.substr(0, pos);
    event_send.erase(0, pos + delimiter.length());

    for (auto& lp : lps) {

        /* Create the send time distributions */
        if (distribution_type == "geometric") {
            lp.send_distribution_ = new Geometric(event_send);

        } else if (distribution_type == "exponential") {
            lp.send_distribution_ = new Exponential(event_send);

        } else if (distribution_type == "binomial") {
            lp.send_distribution_ = new Binomial(event_send);

        } else {
            std::cerr << "Invalid choice of event send distribution." << std::endl;
            abort();
        }

        /* Fetch the adjacency list for each node */
        lp.adjacency_list_ = ntwrk->adjacencyList(lp.name_);
        assert(lp.adjacency_list_.size());
    }
    delete ntwrk;

    /* Parse the node selection distribution type and parameters */
    pos = node_selection.find(delimiter);
    distribution_type = node_selection.substr(0, pos);
    node_selection.erase(0, pos + delimiter.length());

    for (auto& lp : lps) {
        node_selection += "," + lp.adjacency_list_.size();

        /* Create the node selection distributions */
        if (distribution_type == "geometric") {
            lp.node_sel_distribution_ = new Geometric(node_selection);

        } else if (distribution_type == "exponential") {
            lp.node_sel_distribution_ = new Exponential(node_selection);

        } else if (distribution_type == "binomial") {
            lp.node_sel_distribution_ = new Binomial(node_selection);

        } else {
            std::cerr << "Invalid choice of node select distribution." << std::endl;
            abort();
        }
    }

    /* Simulate the synthetic model */
    synthetic_sim.simulate(lp_pointers);

    return 0;
}

