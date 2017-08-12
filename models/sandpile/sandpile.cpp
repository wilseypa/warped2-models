#include <cassert>
#include <algorithm>
#include <cstdlib>

#include "warped.hpp"
#include "sandpile.hpp"
#include "ppm.hpp"
#include "tclap/ValueArg.h"

#define TS_INTERVAL 1 /* Timestamp interval */

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(SandEvent)


std::string node_name (unsigned int index) {

    return std::string("Node_") + std::to_string(index);
}

std::vector<std::shared_ptr<warped::Event> > Vertex::initializeLP() {

    std::vector<std::shared_ptr<warped::Event>> events;

    /* If z-value exceeds threshold, send an event to itself and 4 neighbors */
    if (state_.z_ >= stability_threshold_) {
        events.emplace_back( new SandEvent {node_name(this->index_)                 , TS_INTERVAL} );
        events.emplace_back( new SandEvent {node_name(neighbor(this->index_, UP))   , TS_INTERVAL} );
        events.emplace_back( new SandEvent {node_name(neighbor(this->index_, RIGHT)), TS_INTERVAL} );
        events.emplace_back( new SandEvent {node_name(neighbor(this->index_, DOWN)) , TS_INTERVAL} );
        events.emplace_back( new SandEvent {node_name(neighbor(this->index_, LEFT)) , TS_INTERVAL} );
    }
    return events;
}

std::vector<std::shared_ptr<warped::Event> > Vertex::receiveEvent(const warped::Event& event) {

    auto node_event = static_cast<const SandEvent&>(event);
    unsigned int event_ts = node_event.event_ts_ + TS_INTERVAL;

    std::vector<std::shared_ptr<warped::Event>> events;

    /* If self-event, decrease the z-value by 4 */
    if (node_event.receiver_name_ == node_name(this->index_)) {
        state_.z_ -= 4;

    } else { /* Event received from a neighbor, increment the z-value by 1 */
        state_.z_ += 1;
    }

    /* If z-value exceeds threshold, send an event to itself and 4 neighbors */
    if (state_.z_ >= stability_threshold_) {
        events.emplace_back( new SandEvent {node_name(this->index_)                 , event_ts} );
        events.emplace_back( new SandEvent {node_name(neighbor(this->index_, UP))   , event_ts} );
        events.emplace_back( new SandEvent {node_name(neighbor(this->index_, RIGHT)), event_ts} );
        events.emplace_back( new SandEvent {node_name(neighbor(this->index_, DOWN)) , event_ts} );
        events.emplace_back( new SandEvent {node_name(neighbor(this->index_, LEFT)) , event_ts} );
    }
    return events;
}

/* Find the nearest neighbor in a particular direction */
unsigned int Vertex::neighbor( unsigned int index, direction_t direction ) {

    unsigned int new_x = 0, new_y = 0;
    unsigned int current_y = index / num_cells_x_;
    unsigned int current_x = index % num_cells_x_;

    switch(direction) {
        case LEFT: {
            new_x = (current_x + num_cells_x_ - 1) % num_cells_x_;
            new_y = current_y;
        } break;

        case RIGHT: {
            new_x = (current_x + 1) % num_cells_x_;
            new_y = current_y;
        } break;

        case DOWN: {
            new_x = current_x;
            new_y = (current_y + num_cells_y_ - 1) % num_cells_y_;
        } break;

        case UP: {
            new_x = current_x;
            new_y = (current_y + 1) % num_cells_y_;
        } break;

        default: {
            std::cerr << "Invalid move direction " << direction << std::endl;
            assert(0);
        }
    }

    return (new_x + new_y * num_cells_x_);
}

int main(int argc, const char **argv) {

    /* Set the default values for the arguments */
    unsigned int num_cells_x            = 1000;
    unsigned int num_cells_y            = 1000;
    unsigned int stability_threshold    = 4;
    unsigned int z_at_center            = 300;

    /* Read the arguments */
    TCLAP::ValueArg<unsigned int> num_cells_x_arg("x", "num-cells-x",
                    "Width of sandpile grid", false, num_cells_x, "unsigned int");
    TCLAP::ValueArg<unsigned int> num_cells_y_arg("y", "num-cells-y",
                    "Height of sandpile grid", false, num_cells_y, "unsigned int");
    TCLAP::ValueArg<unsigned int> stability_threshold_arg("s", "stability-threshold",
                    "Threshold of Z-value", false, stability_threshold, "unsigned int");
    TCLAP::ValueArg<unsigned int> z_at_center_arg("z", "z-at-center",
                    "Z-value at center of the grid", false, z_at_center, "unsigned int");

    std::vector<TCLAP::Arg*> cmd_line_args =    {   &num_cells_x_arg,
                                                    &num_cells_y_arg,
                                                    &stability_threshold_arg,
                                                    &z_at_center_arg
                                                };
    warped::Simulation simulation {"Abelian Sandpile Simulation", argc, argv, cmd_line_args};
    num_cells_x         = num_cells_x_arg.getValue();
    num_cells_y         = num_cells_y_arg.getValue();
    stability_threshold = stability_threshold_arg.getValue();
    z_at_center         = z_at_center_arg.getValue();

    /* Create the LPs */
    std::vector<Vertex> lps;
    std::vector<warped::LogicalProcess*> lp_pointers;

    unsigned int total_cells  = num_cells_x * num_cells_y;
    unsigned int center_index = (num_cells_x / 2) * num_cells_y + (num_cells_y / 2);

    for (unsigned int i = 0; i < total_cells; i++) {

        /* Note: Initially, only the center has a slope */
        unsigned int z = (i == center_index) ? z_at_center : 0;
        lps.emplace_back(node_name(i), num_cells_x, num_cells_y, i, z, stability_threshold);
    }

    for (auto& lp : lps) {
        lp_pointers.push_back(&lp);
    }

    simulation.simulate(lp_pointers);

    /* Post-processing : Print fractal image of the output */
    auto lattice = new ppm(num_cells_x, num_cells_y);

    for (auto& lp : lps) {
        auto i = lp.index_;
        if (lp.state_.z_ > 255) {
            lattice->r[i] = 255;
            lattice->g[i] = 255;
            lattice->b[i] = 255;

        } else if (lp.state_.z_ >= 128) {
            lattice->r[i] = lp.state_.z_;
            lattice->g[i] = 0;
            lattice->b[i] = 0;

        } else if (lp.state_.z_ >= stability_threshold) {
            lattice->r[i] = 0;
            lattice->g[i] = 128 + lp.state_.z_;
            lattice->b[i] = 0;

        } else {
            lattice->r[i] = 0;
            lattice->g[i] = 0;
            lattice->b[i] = 0;
        }
    }
    lattice->write("output_lattice.ppm");
    delete lattice;

    return 0;
}

