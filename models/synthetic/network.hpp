#ifndef NETWORK_HPP
#define NETWORK_HPP

#include "memory.hpp"
#include <random>

#define BETA_PRECISION 10000

class Network {
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
            std::cerr << "Network: Invalid fetch request." << std::endl;
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

/* Grid Network */
class Grid : public Network{
public:
    Grid( std::vector<std::string> nodes, unsigned int rows, unsigned int cols ) {

        /* Initialize the connection matrix */
        nodes_ = nodes;
        unsigned int num_nodes = nodes.size();
        assert(rows*cols == num_nodes);

        connections_ = new bool*[num_nodes];
        for (unsigned int i = 0; i < num_nodes; i++) {
            connections_[i] = new bool[num_nodes];
            for (unsigned int j = 0; j < num_nodes; j++) {
                connections_[i][j] = false;
            }
        }

        for (unsigned int i = 0; i < num_nodes; i++) {
            unsigned int row_id = i / cols;
            unsigned int col_id = i % cols;

            if (!row_id && !col_id) { // top left corner
                connections_[i][num_nodes-cols]   = true;
                connections_[i][1 % cols]         = true;
                connections_[i][cols % num_nodes] = true;
                connections_[i][cols-1]           = true;

            } else if (row_id == rows-1 && !col_id) { // bottom left corner

            } else if (!row_id && col_id == cols-1) { // top right corner

            } else if (row_id == rows-1 && col_id == cols-1) { // bottom right corner

            } else if (!row_id) { // top edge

            } else if (row_id == rows-1) { // bottom edge

            } else if (!col_id) { // left edge
                connections_[i][i-cols]   = true;
                connections_[i][i+1]      = true;
                connections_[i][i+cols]    = true;
                connections_[i][i+cols-1] = true;

            } else if (col_id == cols-1) { // right edge
                connections_[i][i-cols]   = true;
                connections_[i][i-cols+1] = true;
                connections_[i][i+cols]    = true;
                connections_[i][i-1]      = true;

            } else { // internal node
                connections_[i][i-cols] = true;
                connections_[i][i+1]    = true;
                connections_[i][i+cols] = true;
                connections_[i][i-1]    = true;
            }
            connections_[i][i] = false; // extra check for row/col count = 1
        }
    }
};

/* Watts-Strogatz Network */
class WattsStrogatz : public Network {
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

/* Barabasi-Albert Network */
class BarabasiAlbert : public Network {
public:
    BarabasiAlbert( std::vector<std::string> nodes, unsigned int m, double a ) {

        /* Create the initial fully connected network using m+1 nodes */
        nodes_ = nodes;
        unsigned int num_nodes = nodes.size();
        if (m >= num_nodes) {
            std::cerr << "Barabasi-Albert Network: Input 'm' >= No. of Nodes." << std::endl;
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

Network *buildNetwork (std::string params, std::vector<std::string> lp_names) {

    std::string delimiter = ",";
    size_t pos = params.find(delimiter);
    std::string type = params.substr(0, pos);
    params.erase(0, pos + delimiter.length());

    if (type == "Grid") { // If the choice is Grid
        pos = params.find(delimiter);
        std::string token = params.substr(0, pos);
        unsigned int rows = (unsigned int) std::stoul(token);
        params.erase(0, pos + delimiter.length());
        unsigned int cols = (unsigned int) std::stoul(params);
        return new Grid(lp_names, rows, cols);

    } else if (type == "Watts-Strogatz") { // If the choice is Watts-Strogatz
        pos = params.find(delimiter);
        std::string token = params.substr(0, pos);
        unsigned int k = (unsigned int) std::stoul(token);
        params.erase(0, pos + delimiter.length());
        double beta = std::stod(params);
        return new WattsStrogatz(lp_names, k, beta);

    } else if (type == "Barabasi-Albert") { // If the choice is Barabasi-Albert
        pos = params.find(delimiter);
        std::string token = params.substr(0, pos);
        unsigned int m = (unsigned int) std::stoul(token);
        params.erase(0, pos + delimiter.length());
        double a = std::stod(params);
        return new BarabasiAlbert(lp_names, m, a);

    } else { // Invalid choice
        std::cerr << "Invalid choice of network." << std::endl;
        abort();
    }
}

#endif
