#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cassert>
#include <algorithm>
#include <iterator>

#include "pandemic.hpp"
#include "graph.hpp"
#include "ppm/ppm.hpp"
#include "tclap/ValueArg.h"

#define DEFAULT_MODEL_CONFIG        "data/05-26-2020.formatted-JHU-data.json"
#define DEFAULT_OUTPUT_FILE_NAME    "result.json"

PandemicConfig* PandemicConfig::instance_ = nullptr;
ConfigFileHandler* ConfigFileHandler::instance_ = nullptr;

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(PandemicEvent)


std::vector<std::shared_ptr<warped::Event> > Location::initializeLP() {

    /* Register random number generator to allow kernel to roll it back */
    this->registerRNG<std::default_random_engine>(this->rng_);

    std::vector<std::shared_ptr<warped::Event> > events;

    /* Create timer events for diffusion and update state */
    events.emplace_back(new PandemicEvent {this->location_name_,
            CONFIG->update_trig_interval_in_hrs_, event_type_t::UPDATE_TIMER});
    events.emplace_back(new PandemicEvent {this->location_name_,
            CONFIG->diffusion_trig_interval_in_hrs_, event_type_t::DIFFUSION_TIMER});
    return events;
}

std::vector<std::shared_ptr<warped::Event> > Location::receiveEvent(const warped::Event& event) {

    std::vector<std::shared_ptr<warped::Event> > events;
    auto pandemic_event = reinterpret_cast<const PandemicEvent&>(event);
    auto timestamp = pandemic_event.timestamp();

    /* If Diffusion Timer event */
    if (pandemic_event.type_ == event_type_t::DIFFUSION_TIMER) {
        /* TODO: Do we only allow susceptible and exposed to travel ? It is part
                 of advanced features we need to add for quarantine.
           Note: a. Temporarily let's assume only susceptible people are travelling
                 b. We won't consider Recovered in the diffusion mechanism since their
                    diffusion has no impact if we assume they have acquired immunity.
                    If not, the diffusion mechanism needs to be updated.
         */
        auto diffusion_cnt = diffusionCount();
        while (diffusion_cnt--) {
            if (state_->population_[infection_state_t::SUSCEPTIBLE] == 0) break;
            std::string target = diffusionTarget();
            auto travel_time = travelTimeToTarget(target);
            --state_->population_[infection_state_t::SUSCEPTIBLE];
            events.emplace_back(new PandemicEvent {target,
                timestamp + travel_time, event_type_t::DIFFUSION, infection_state_t::SUSCEPTIBLE});
        }
        events.emplace_back(new PandemicEvent {location_name_,
            timestamp + CONFIG->diffusion_trig_interval_in_hrs_, event_type_t::DIFFUSION_TIMER});

    } else if (pandemic_event.type_ == event_type_t::UPDATE_TIMER) { /* Update Timer event */
        reaction();
        events.emplace_back(new PandemicEvent {location_name_,
            timestamp + CONFIG->update_trig_interval_in_hrs_, event_type_t::UPDATE_TIMER});

    } else if (pandemic_event.type_ == event_type_t::DIFFUSION) { /* Diffusion event */
        ++state_->population_[pandemic_event.state_];

    } else { /* Invalid choice */
        assert(0);
    }
    return events;
}

std::string toString(unsigned int num) {
    std::ostringstream convert;
    convert << num;
    return convert.str();
}


int main(int argc, const char** argv) {
    std::string model_config_name   = DEFAULT_MODEL_CONFIG;
    std::string out_file_name       = DEFAULT_OUTPUT_FILE_NAME;

    TCLAP::ValueArg<std::string> model_config_name_arg( "m", "model-config",
                        "Provide name of the model config", false, model_config_name, "string" );
    TCLAP::ValueArg<std::string> out_file_name_arg( "o", "out-file",
                        "Provide name of the output file", false, out_file_name, "string" );
    std::vector<TCLAP::Arg*> args = {&model_config_name_arg, &out_file_name_arg};
    warped::Simulation pandemic_sim {"Covid-19 Pandemic Simulation", argc, argv, args};
    model_config_name = model_config_name_arg.getValue();
    out_file_name = out_file_name_arg.getValue();

    std::vector<Location> lps;

    CONFIGFILEHANDLER->readConfig(model_config_name, lps);

    /* Simulate the pandemic */
    std::vector<warped::LogicalProcess*> lp_pointers;
    for (auto& lp : lps) {
        lp_pointers.push_back(&lp);
    }
    pandemic_sim.simulate(lp_pointers);

    CONFIGFILEHANDLER->writeConfig(out_file_name, lps);

    return 0;
}
