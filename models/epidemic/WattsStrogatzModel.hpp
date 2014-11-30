#ifndef WATTS_STROGATZ_MODEL_HPP
#define WATTS_STROGATZ_MODEL_HPP

#include "RandomNumGenerator.hpp"

#define BETA_PRECISION 10000

class WattsStrogatzModel {
public:

    WattsStrogatzModel(unsigned int k, float beta, unsigned int seed) 
            : k_(k), beta_(beta) {

        rand_num_generator_ = make_unique<RandomNumGenerator>(seed);
    }

    void populateNodes(std::vector<std::string> nodes) {

        nodes_ = nodes;
        unsigned int num_nodes = nodes.size();
        for(unsigned int index = 0; index < num_nodes; index++) {
                connections_[index].assign(num_nodes, false);
            }
        }
    }

    void mapNodes() {

        unsigned int num_nodes = nodes_.size();
        unsigned int left  = k_ / 2;
        unsigned int right = k_ - left;
        unsigned precision_beta = (unsigned int) (beta_ * BETA_PRECISION);

        /* Setup the ring lattice with N nodes, each of degree K */
        for(unsigned int index = 0; index < num_nodes; index++) {
            for(unsigned int node_index = 1; node_index <= left; node_index++) {
                unsigned int left_index = (index + num_nodes - node_index) % num_nodes;
                connections_[index][left_index] = true;
                connections_[left_index][index] = true;
            }
            for(unsigned int node_index = 1; node_index <= right; node_index++) {
                unsigned int right_index = (index + node_index) % num_nodes;
                connections_[index][right_index] = true;
                connections_[right_index][index] = true;
            }
        }

        /* Rewire each edge with probability beta */
        for(unsigned int index = 0; index < num_nodes; index++) {
            for(unsigned int node_index = 0; node_index < num_nodes; node_index++) {
                if(!connections_[index][node_index]) continue;
                unsigned int rand_num = rand_num_generator_->generateRandNum(BETA_PRECISION);
                if(rand_num >= precision_beta) continue;

                unsigned int new_index = 0;
                while(1) {
                    new_index = rand_num_generator_->generateRandNum(num_nodes);
                    if((new_index != index) && (new_index != node_index)) break;
                }
                connections_[index][node_index] = false;
                connections_[node_index][index] = false;
                connections_[index][new_index] = false;
                connections_[new_index][index] = false;
            }
        }
    }

    /* Send the node links for a particular node */
    std::vector<std::string> fetchNodeLinks(std::string location_name) {

        unsigned int count = 0;
        for(auto node : nodes_) {
            if(!node.compare(location_name)) {
                break;
            } else {
                count++;
            }
        }
        if(count == nodes_.size()) {
            std::cerr << "Watts-Strogatz model: Invalid fetch request." << endl;
            abort();
        }

        std::vector<std::string> node_links; 
        for(unsigned int index = 0; index < nodes_.size(); index++) {
            if(connections_[count][index]) {
                node_links.push_back(nodes_[index]);
            }
        }
        return node_links;
    }

private:
    std::vector<std::string> nodes_;
    unsigned int k_;
    float beta_;
    std::unique_ptr<RandomNumGenerator> rand_num_generator_;
    std::vector<std::vector<bool>> connections_;
};

#endif
