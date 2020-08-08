#ifndef GRAPH_HPP
#define GRAPH_HPP

#include "memory.hpp"
#include <random>

#define BETA_PRECISION 10000

class Graph {
protected:
    std::vector<std::string> nodes_;
    bool **connections_;

public:
    /* Fetch the adjacency list for a particular node */
    std::vector<std::string> adjacencyList( std::string node_name ) {

        unsigned int count = 0;
        for (auto node : nodes_) {
            if (node == node_name) {
                break;
            } else {
                count++;
            }
        }
        if (count == nodes_.size()) {
            std::cerr << "Graph: Invalid fetch request." << std::endl;
            abort();
        }

        std::vector<std::string> adjacency_list; 
        for (unsigned int index = 0; index < nodes_.size(); index++) {
            if (connections_[count][index]) {
                adjacency_list.push_back(nodes_[index]);
            }
        }
        return adjacency_list;
    }
};

/* Watts-Strogatz Graph */
class WattsStrogatz : public Graph {
public:
    WattsStrogatz( std::vector<std::string> nodes, unsigned int k, double beta ) {

        /* Initialize the connection matrix */
        nodes_ = nodes;
        unsigned int num_nodes = nodes.size();
        connections_ = new bool*[num_nodes];
        for (unsigned int index_x = 0; index_x < num_nodes; index_x++) {
            connections_[index_x] = new bool[num_nodes];
            for (unsigned int index_y = 0; index_y < num_nodes; index_y++) {
                connections_[index_x][index_y] = false;
            }
        }
        unsigned int left  = k / 2;
        unsigned int right = k - left;
        unsigned precision_beta = (unsigned int) (beta * BETA_PRECISION);

        /* Setup the ring lattice with N nodes, each of degree K */
        for (unsigned int index = 0; index < num_nodes; index++) {
            for (unsigned int node_index = 1; node_index <= left; node_index++) {
                unsigned int left_index = (index + num_nodes - node_index) % num_nodes;
                connections_[index][left_index] = true;
                connections_[left_index][index] = true;
            }
            for (unsigned int node_index = 1; node_index <= right; node_index++) {
                unsigned int right_index = (index + node_index) % num_nodes;
                connections_[index][right_index] = true;
                connections_[right_index][index] = true;
            }
        }

        /* Rewire each edge with probability beta */
        std::default_random_engine generator;
        std::uniform_int_distribution<int> precision_dist(0, BETA_PRECISION-1);
        std::uniform_int_distribution<int> node_dist(0, num_nodes-1);

        for (unsigned int index = 0; index < num_nodes; index++) {
            for (unsigned int node_index = 0; node_index < num_nodes; node_index++) {
                if (!connections_[index][node_index]) continue;
                auto rand_num = (unsigned int) precision_dist(generator);
                if (rand_num >= precision_beta) continue;

                unsigned int new_index = 0;
                while(1) {
                    new_index = (unsigned int) node_dist(generator);
                    if ((new_index != index) && (new_index != node_index)) break;
                }
                connections_[index][node_index] = false;
                connections_[node_index][index] = false;
                connections_[index][new_index] = false;
                connections_[new_index][index] = false;
            }
        }
    }
};

/* Barabasi-Albert Graph */
class BarabasiAlbert : public Graph {
public:
    BarabasiAlbert( std::vector<std::string> nodes, unsigned int m, double a ) {

        /* Create the initial fully connected graph using m+1 nodes */
        nodes_ = nodes;
        unsigned int num_nodes = nodes.size();
        if (m >= num_nodes) {
            std::cerr << "Barabasi-Albert Graph: Input 'm' >= No. of Nodes." << std::endl;
            abort();
        }
        connections_ = new bool*[num_nodes];
        unsigned int *degree = new unsigned int[num_nodes];
        for (unsigned int index_x = 0; index_x < num_nodes; index_x++) {
            connections_[index_x] = new bool[num_nodes];
            degree[index_x] = 0;
            for (unsigned int index_y = 0; index_y <= m; index_y++) {
                if (index_x <= m && index_x != index_y) {
                    connections_[index_x][index_y] = true;
                    degree[index_x]++;
                } else {
                    connections_[index_x][index_y] = false;
                }
            }
            for (unsigned int index_y = m+1; index_y < num_nodes; index_y++) {
                connections_[index_x][index_y] = false;
            }
        }
        unsigned int total_degrees = m*(m+1);

        std::default_random_engine generator;
        std::uniform_real_distribution<double> r_dist(0.0, 1.0);

        /* For each new nodes, create atmost m new edges */
        for (unsigned int new_node_index = m+1; new_node_index < num_nodes; new_node_index++) {

            std::uniform_int_distribution<int> node_dist(0, new_node_index-1);
            for (unsigned int edge_cnt = 0; edge_cnt < m; edge_cnt++) {

                /* Preferential Attachment Growth */
                unsigned int i = (unsigned int) node_dist(generator);
                double p = ((double) degree[i]) / total_degrees;
                p = pow(p, a);
                double r = r_dist(generator);
                if (p >= r) continue; // no new edge is created

                /* New edge is created if it already doesn't exist */
                if (!connections_[new_node_index][i]) {
                    connections_[new_node_index][i] = true;
                    connections_[i][new_node_index] = true;
                    degree[i]++;
                    degree[new_node_index]++;
                    total_degrees += 2;
                }
            }
        }
    }
};

#endif
