// Based on ROSS NeMo simulator

#include <cassert>
#include <random>
#include "neuromorphic.hpp"
#include "tclap/ValueArg.h"

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(NeuromorphicEvent)

std::vector<std::shared_ptr<warped::Event> > Neuromorphic::initializeLP() {

    // Register random number generator
    this->registerRNG<std::default_random_engine>(this->rng_);

    std::vector<std::shared_ptr<warped::Event> > events;

    return events;
}

inline std::string Neuromorphic::lp_name(const unsigned int lp_index) {

    return std::string("Neuron_") + std::to_string(lp_index);
}

std::vector<std::shared_ptr<warped::Event> > Neuromorphic::receiveEvent(const warped::Event& event) {

    std::vector<std::shared_ptr<warped::Event> > response_events;
    auto received_event = static_cast<const NeuromorphicEvent&>(event);

    switch (received_event.type_) {
    }
    return response_events;
}

int main(int argc, const char** argv) {

    warped::Simulation neuromorphic_sim {"Neuromorphic Simulation", argc, argv, args};

    std::vector<Neuromorphic> lps;

    std::vector<warped::LogicalProcess*> lp_pointers;
    for (auto& lp : lps) {
        lp_pointers.push_back(&lp);
    }

    neuromorphic_sim.simulate(lp_pointers);

    return 0;
}

