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
            /* Dummy floating point operation */
            val += i;
        }

        /* Modify the state size */
        std::uniform_real_distribution<double>
                state_size_change_dist( min_state_size_change_, max_state_size_change_  );
        state_size_ += (unsigned int)(state_size_change_dist(*rng_) * state_size_);
        state_.stream_.resize(state_size_, '0');
    }
    return response_events;
}

int main(int argc, const char** argv) {

    unsigned int num_nodes                  = 100000;
    std::string network                     = "Watts-Strogatz,30,0.1";
    std::string node_selection              = "exponential,0.5";
    std::string floating_point_ops_cnt      = "1000,1000";
    std::string state_size                  = "100,100";
    std::string state_size_change           = "0,0";
    std::string event_send                  = "geometric,0.1,10";

    TCLAP::ValueArg<unsigned int> num_nodes_arg("", "num-nodes",
            "Number of nodes", false, num_nodes, "unsigned int");
    TCLAP::ValueArg<std::string> network_arg("", "network-params",
            "Network details - <network-type,<network-params>>", false, network, "string");
    TCLAP::ValueArg<std::string> node_selection_arg("", "node-selection-params",
            "Node selection details - <distribution-type,<distribution-params>>",
            false, node_selection, "string");
    TCLAP::ValueArg<std::string> floating_point_ops_cnt_arg("", "event-processing-time-range",
            "Event processing time (as floating-point calculation count) range - <min,max>",
            false, floating_point_ops_cnt, "string");
    TCLAP::ValueArg<std::string> state_size_arg("", "state-size-range",
            "Size range (in bytes) for the LP state - <min,max>",
            false, state_size, "string");
    TCLAP::ValueArg<std::string> state_size_change_arg("", "state-size-change-range",
            "Change in LP state when an event is processed ; range=(-1,1) - <min,max>",
            false, state_size_change, "string");
    TCLAP::ValueArg<std::string> event_send_arg("", "event-send-time-delta-params",
            "Event send time delta details - <dist-type,<dist-params>,ceiling-value>",
            false, event_send, "string");

    std::vector<TCLAP::Arg*> model_args = { &num_nodes_arg,
                                            &network_arg,
                                            &node_selection_arg,
                                            &floating_point_ops_cnt_arg,
                                            &state_size_arg,
                                            &state_size_change_arg,
                                            &event_send_arg
                                          };

    warped::Simulation synthetic_sim {"Synthetic Simulation", argc, argv, model_args};

    num_nodes                   = num_nodes_arg.getValue();
    network                     = network_arg.getValue();
    node_selection              = node_selection_arg.getValue();
    floating_point_ops_cnt      = floating_point_ops_cnt_arg.getValue();
    state_size                  = state_size_arg.getValue();
    state_size_change           = state_size_change_arg.getValue();
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

    /* Parse the range of state size change */
    pos = state_size_change.find(delimiter);
    double min_state_size_change = std::stod(state_size_change.substr(0, pos));
    double max_state_size_change = std::stod(state_size_change.substr(pos+1));

    /* Create the LPs */
    std::default_random_engine generator (num_nodes);
    for (unsigned int i = 0; i < num_nodes; i++) {
        auto name = Node::lpName(i);
        lp_names.push_back(name);
        lps.emplace_back(   name,
                            num_nodes,
                            floating_point_ops_cnt_dist(generator),
                            state_size_dist(generator),
                            min_state_size_change,
                            max_state_size_change,
                            i
                        );
    }

    std::vector<warped::LogicalProcess*> lp_pointers;
    for (auto& lp : lps) {
        lp_pointers.push_back(&lp);
    }

    /* Create the Network */
    Network *graph = buildNetwork(network, lp_names);

    for (auto& lp : lps) {
        /* Create the send time distribution */
        lp.send_distribution_ = buildDist(event_send);

        /* Fetch the adjacency list for each node */
        lp.adjacency_list_ = graph->adjacencyList(lp.name_);
        assert(lp.adjacency_list_.size());

        /* Create the node selection distribution */
        node_selection.append(",");
        node_selection.append(std::to_string(lp.adjacency_list_.size()));
        lp.node_sel_distribution_ = buildDist(node_selection);
    }
    delete graph;

    /* Simulate the synthetic model */
    synthetic_sim.simulate(lp_pointers);

    return 0;
}

