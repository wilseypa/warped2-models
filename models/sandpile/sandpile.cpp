#include <cassert>
#include <algorithm>
#include <cstdlib>

#include "warped.hpp"
#include "sandpile.hpp"
#include "ppm/ppm.hpp"
#include "tclap/ValueArg.h"

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(SandEvent)


std::string node_name (unsigned int index) {

    return std::string("Vertex_") + std::to_string(index);
}

std::vector<std::shared_ptr<warped::Event> > Vertex::initializeLP() {

    std::vector<std::shared_ptr<warped::Event>> events;

    /* Find the center of the grid */
    unsigned int center_index = (this->grid_dimension_*this->grid_dimension_)/2;
    center_index -= (this->grid_dimension_%2) ? 0 : (this->grid_dimension_/2);

    /* Return if not the center */
    if (this->index_ != center_index) return events;

    /* Send a self event for center to trigger the first topple */
    events.emplace_back( new SandEvent {node_name(this->index_), 4} );
    state_.sandpile_height_ = 3;

    return events;
}

std::vector<std::shared_ptr<warped::Event> > Vertex::receiveEvent(const warped::Event& event) {

    auto sand_event = static_cast<const SandEvent&>(event);
    unsigned int event_ts = sand_event.event_ts_ + 1;

    std::vector<std::shared_ptr<warped::Event>> events;

    /* Add a grain of sand */
    state_.sandpile_height_++;

    /*  NOTE:   Only the vertex (here center of grid) has non-empty sandpile initially.
                It can keep getting more sand.
     */
    if (sand_event.sender_name_ == node_name(this->index_)) {
        events.emplace_back( new SandEvent {node_name(this->index_), event_ts} );
    }

    /* If the sandpile is at topple threshold */
    if (state_.sandpile_height_ == 4) {

        state_.sandpile_height_ = 0;
        state_.collapse_cnt_++;

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
    unsigned int current_y = index / grid_dimension_;
    unsigned int current_x = index % grid_dimension_;

    switch(direction) {
        case LEFT: {
            new_x = (current_x + grid_dimension_ - 1) % grid_dimension_;
            new_y = current_y;
        } break;

        case RIGHT: {
            new_x = (current_x + 1) % grid_dimension_;
            new_y = current_y;
        } break;

        case DOWN: {
            new_x = current_x;
            new_y = (current_y + 1) % grid_dimension_;
        } break;

        case UP: {
            new_x = current_x;
            new_y = (current_y + grid_dimension_ - 1) % grid_dimension_;
        } break;

        default: {
            std::cerr << "Invalid move direction " << direction << std::endl;
            assert(0);
        }
    }

    return (new_x + new_y * grid_dimension_);
}

int main(int argc, const char **argv) {

    /* Set the default values for the argument */
    unsigned int grid_dimension = 1000;

    /* Read the argument */
    TCLAP::ValueArg<unsigned int> grid_dimension_arg("d", "grid-dimension",
                    "Dimension of the square sandpile grid", false, grid_dimension, "unsigned int");
    std::vector<TCLAP::Arg*> cmd_line_args = { &grid_dimension_arg };
    warped::Simulation simulation {"Abelian Sandpile Simulation", argc, argv, cmd_line_args};
    grid_dimension = grid_dimension_arg.getValue();

    /* Create the LPs */
    std::vector<Vertex> lps;
    std::vector<warped::LogicalProcess*> lp_pointers;

    for (unsigned int i = 0; i < grid_dimension*grid_dimension; i++) {
        /* Note: Initially, there is no sand */
        lps.emplace_back(node_name(i), grid_dimension, i, 0);
    }

    for (auto& lp : lps) {
        lp_pointers.push_back(&lp);
    }

    simulation.simulate(lp_pointers);

    /* Post-processing : Print fractal image of the grid */
    auto grid = new ppm(grid_dimension, grid_dimension);
    unsigned int i = 0;
    for (auto& lp : lps) {
        if (lp.state_.sandpile_height_ == 3) { /* Vertex will topple immediately, mark WHITE */
            grid->r[i] = 255;
            grid->g[i] = 255;
            grid->b[i] = 255;

        } else if (lp.state_.sandpile_height_ == 2) { /* Vertex will topple after WHITE, mark RED */
            grid->r[i] = 255;
            grid->g[i] = 0;
            grid->b[i] = 0;

        } else if (lp.state_.sandpile_height_ == 1) { /* Vertex will topple after RED, mark YELLOW */
            grid->r[i] = 255;
            grid->g[i] = 255;
            grid->b[i] = 0;

        } else {
            /* If the vertex has collapsed before and empty now, mark GREEN */
            if (lp.state_.collapse_cnt_) {
                grid->r[i] = 0;
                grid->g[i] = 255;
                grid->b[i] = 0;

            } else { /* If vertex has never collapsed, mark BLACK */
                grid->r[i] = 0;
                grid->g[i] = 0;
                grid->b[i] = 0;
            }
        }
        i++;
    }
    grid->write("output_grid.ppm");
    delete grid;

    return 0;
}

