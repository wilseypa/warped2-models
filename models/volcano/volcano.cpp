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

    warped::Simulation volcano_sim {"Volcano Simulation", argc, argv, args};

    std::vector<Volcano> lps;

    std::vector<warped::LogicalProcess*> lp_pointers;
    for (auto& lp : lps) {
        lp_pointers.push_back(&lp);
    }

    volcano_sim.simulate(lp_pointers);

    return 0;
}

