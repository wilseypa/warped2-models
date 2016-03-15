// A customized implementation of volcano model

#include <cassert>
#include <random>
#include "volcano.hpp"
#include "tclap/ValueArg.h"

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(VolcanoEvent)

std::vector<std::shared_ptr<warped::Event> > Volcano::initializeLP() {

    // Register random number generator
    this->registerRNG<std::default_random_engine>(this->rng_);

    std::vector<std::shared_ptr<warped::Event> > events;

    return events;
}

inline std::string Volcano::lp_name(const unsigned int lp_index) {

    return std::string("Particle_") + std::to_string(lp_index);
}

std::vector<std::shared_ptr<warped::Event> > Volcano::receiveEvent(const warped::Event& event) {

    std::vector<std::shared_ptr<warped::Event> > response_events;
    auto received_event = static_cast<const VolcanoEvent&>(event);

    switch (received_event.type_) {
    }
    return response_events;
}

int main(int argc, const char** argv) {

    unsigned int grid_dimension_x   = 1000;
    unsigned int grid_dimension_y   = 1000;
    unsigned int grid_dimension_z   = 1000;
    unsigned int num_particles      = 1000000;

    TCLAP::ValueArg<unsigned int> grid_dimension_x_arg("x", "grid-dimension-x", 
                "grid dimension on x-axis", false, grid_dimension_x, "unsigned int");
    TCLAP::ValueArg<unsigned int> grid_dimension_y_arg("y", "grid-dimension-y", 
                "grid dimension on y-axis", false, grid_dimension_y, "unsigned int");
    TCLAP::ValueArg<unsigned int> grid_dimension_z_arg("z", "grid-dimension-z", 
                "grid dimension on z-axis", false, grid_dimension_z, "unsigned int");
    TCLAP::ValueArg<unsigned int> num_particles_arg("n", "num-particles", 
                "total number of particles", false, num_particles, "unsigned int");

    std::vector<TCLAP::Arg*> cmd_line_args = {  &grid_dimension_x_arg, 
                                                &grid_dimension_y_arg, 
                                                &grid_dimension_z_arg, 
                                                &num_particles_arg  };

    warped::Simulation simulation {"Volcano Simulation", argc, argv, cmd_line_args};

    grid_dimension_x    = grid_dimension_x_arg.getValue();
    grid_dimension_y    = grid_dimension_y_arg.getValue();
    grid_dimension_z    = grid_dimension_z_arg.getValue();
    num_particles       = num_particles_arg.getValue();

    std::vector<Volcano> lps;
    unsigned int index = 0;
    for (unsigned int x = 0; x < grid_dimension_x; x++) {
        for (unsigned int y = 0; y < grid_dimension_y; y++) {
            for (unsigned int z = 0; z < grid_dimension_z; z++) {
                lps.emplace_back(x, y, z, index);
                index++;
            }
        }
    }

    std::vector<warped::LogicalProcess*> lp_pointers;
    for (auto& lp : lps) {
        lp_pointers.push_back(&lp);
    }
    simulation.simulate(lp_pointers);

    return 0;
}

