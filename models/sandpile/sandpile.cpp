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

    return std::string("Vertex_") + std::to_string(index);
}

std::vector<std::shared_ptr<warped::Event> > Vertex::initializeLP() {

    std::vector<std::shared_ptr<warped::Event>> events;

    /* If there is no grain of sand */
    if (!state_.sandpile_height_) return events;

    /* Sandpile can't initially be at topple threshold */
    assert (state_.sandpile_height_ < 4);

    /* Add a grain of sand and send an update event to itself */
    state_.sandpile_height_++;
    events.emplace_back( new SandEvent {node_name(this->index_), TS_INTERVAL} );

    /* If the sandpile is at topple threshold */
    if (state_.sandpile_height_ == 4) {

        state_.sandpile_height_ = 0;
        state_.collapse_cnt_++;

        events.emplace_back( new SandEvent {node_name(neighbor(this->index_, UP))   , TS_INTERVAL} );
        events.emplace_back( new SandEvent {node_name(neighbor(this->index_, RIGHT)), TS_INTERVAL} );
        events.emplace_back( new SandEvent {node_name(neighbor(this->index_, DOWN)) , TS_INTERVAL} );
        events.emplace_back( new SandEvent {node_name(neighbor(this->index_, LEFT)) , TS_INTERVAL} );
    }

    return events;
}

std::vector<std::shared_ptr<warped::Event> > Vertex::receiveEvent(const warped::Event& event) {

    auto sand_event = static_cast<const SandEvent&>(event);
    unsigned int event_ts = sand_event.event_ts_ + TS_INTERVAL;

    std::vector<std::shared_ptr<warped::Event>> events;

    /* Add a grain of sand */
    state_.sandpile_height_++;

    /*  NOTE:   Only the vertex (here center of grid) has non-empty sandpile initially.
                It can keep getting more sand.
     */
    if (sand_event.receiver_name_ == node_name(this->index_)) {
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

    unsigned int vertex_cnt   = grid_dimension * grid_dimension;
    unsigned int center_index = (vertex_cnt / 2) - ((grid_dimension % 2) ? 0 : (grid_dimension / 2));

    for (unsigned int i = 0; i < vertex_cnt; i++) {

        /* Note: Initially, only the center has a grain of sand */
        unsigned int sandpile_height = (i == center_index) ? 1 : 0;
        lps.emplace_back(node_name(i), grid_dimension, i, sandpile_height);
    }

    for (auto& lp : lps) {
        lp_pointers.push_back(&lp);
    }

    simulation.simulate(lp_pointers);

    /* Post-processing : Print fractal image of the grid */
    auto grid = new ppm(grid_dimension, grid_dimension);

    for (auto& lp : lps) {
        auto i = lp.index_;
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
    }
    grid->write("output_grid.ppm");
    delete grid;

    return 0;
}

