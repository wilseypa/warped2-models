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

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(DiffusionEvent)
WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(TriggerEvent)


std::vector<std::shared_ptr<warped::Event> > Location::initializeLP() {

    // Register random number generator to allow kernel to roll it back
    this->registerRNG<std::default_random_engine>(this->rng_);

    std::vector<std::shared_ptr<warped::Event> > events;
    events.emplace_back(new TriggerEvent {this->location_name_,
                                    CONFIG->update_trig_interval_in_hrs});
    events.emplace_back(new TriggerEvent {this->location_name_,
                                    CONFIG->diffusion_trig_interval_in_hrs, true});
    return events;
}

std::vector<std::shared_ptr<warped::Event> > Location::receiveEvent(const warped::Event& event) {

    std::vector<std::shared_ptr<warped::Event> > events;

    // TODO error: base operand of ‘->’ has non-pointer type ‘const warped::Event’
    if (event->eventType() == event_type_t::TRIGGER) {
        auto trigger_event = static_cast<const TriggerEvent&>(event);
        auto timestamp = trigger_event->timestamp();

        std::string selected_location = pickLocation(location_name_);

        auto travel_time = travelTimeToLocation(selected_location);

        state_->current_population_->erase(map_iter);
            // TODO error: no matching function for call to ‘TriggerEvent::TriggerEvent(<brace-enclosed initializer list>)’
        events.emplace_back(new TriggerEvent {location_name_,
                                timestamp + CONFIG->diffusion_trig_interval_, true});

        std::uniform_real_distribution<double> distribution(0.0, 1.0);
        auto rand_factor = distribution(*rng_);
        reaction();
        // TODO error: no matching function for call to ‘TriggerEvent::TriggerEvent(<brace-enclosed initializer list>)’
        events.emplace_back(new TriggerEvent {location_name_,
                                timestamp + CONFIG->update_trig_interval_});
    } else {
        state_->current_population_->insert(state_->current_population_->begin(),
                std::pair <unsigned long, std::shared_ptr<Person>> (epidemic_event.pid_, person));
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
    CONFIGFILEHANDLER->openFile(model_config_name, &lps);
    CONFIGFILEHANDLER->getValuesFromJsonFile();

    /* Simulate the pandemic */
    std::vector<warped::LogicalProcess*> lp_pointers;
    for (auto& lp : lps) {
        lp_pointers.push_back(&lp);
    }
    pandemic_sim.simulate(lp_pointers);

    CONFIGFILEHANDLER->writeSimulationOutputToJsonFile(out_file_name);

    return 0;
}
