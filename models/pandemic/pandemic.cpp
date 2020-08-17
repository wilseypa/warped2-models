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

#define DEFAULT_MODEL_NAME "data/05-26-2020.formatted-JHU-data.json"

PandemicConfig* PandemicConfig::instance_ = nullptr;
ConfigFileHandler* ConfigFileHandler::instance_ = nullptr;

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(DiffusionEvent)
WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(TriggerEvent)


std::vector<std::shared_ptr<warped::Event> > Location::initializeLP() {

    // Register random number generator to allow kernel to roll it back
    this->registerRNG<std::default_random_engine>(this->rng_);

    std::vector<std::shared_ptr<warped::Event> > events;
    events.emplace_back(new TriggerEvent {this->location_name_,
                                    CONFIG->update_trig_interval_});
    events.emplace_back(new TriggerEvent {this->location_name_,
                                    CONFIG->diffusion_trig_interval_, true});
    return events;
}

std::vector<std::shared_ptr<warped::Event> > Location::receiveEvent(const warped::Event& event) {

    std::vector<std::shared_ptr<warped::Event> > events;

    // TODO error: base operand of ‘->’ has non-pointer type ‘const warped::Event’
    if (event->eventType() == event_type_t::TRIGGER) {
        auto trigger_event = static_cast<const TriggerEvent&>(event);
        auto timestamp = trigger_event->timestamp();

        std::string selected_location = diffusion_network_->pickLocation();
        auto travel_time = diffusion_network_->travelTimeToLocation(selected_location);
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


int main(int argc, const char** argv)
{
    std::string model_config_name = DEFAULT_MODEL_NAME;

    TCLAP::ValueArg<std::string> model_config_name_arg( "m", "model-config",
                        "Provide name of the model config", false, model_config_name, "string" );
    std::vector<TCLAP::Arg*> args = {&model_config_name_arg};
    warped::Simulation pandemic_sim {"Covid-19 Pandemic Simulation", argc, argv, args};
    model_config_name   = model_config_name_arg.getValue();

    std::map<std::string, unsigned int> travel_map;
    std::vector<Location> lps;

    CONFIGFILEHANDLER->openFile(model_config_name, &lps);
    CONFIGFILEHANDLER->getValuesFromJsonFile();
    // Simulate the epidemic
    std::vector<warped::LogicalProcess*> lp_pointers;
    for (auto& lp : lps) {
        lp_pointers.push_back(&lp);
    }
    pandemic_sim.simulate(lp_pointers);

    // Plot the heatmaps
    unsigned long cols = std::ceil( sqrt(lps.size()) );
    unsigned long rows = (lps.size()+cols-1)/cols;
    auto population_diffusion_hmap = new ppm(cols, rows);
    auto disease_growth_hmap = new ppm(cols, rows);

    // Record final statistics for each LP
    i = 0;
    for (auto& lp : lps) {
        unsigned long final_population = 0, final_affected_cnt = 0;
        // TODO error: ‘class Location’ has no member named ‘statistics’
        lp.statistics(&final_population, &final_affected_cnt);

        // If initial population >  final population, color is GREEN
        // If initial population == final population, color is BLACK
        // If initial population  < final population, color is RED
        if (initial_population[i] >= final_population) {
            double ratio = ( (double)(initial_population[i] - final_population) ) /
                                                                    initial_population[i];
            population_diffusion_hmap->g[i] = ratio * 255;

        } else {
            double ratio = ( (double)(final_population - initial_population[i]) ) /
                                                                    final_population;
            population_diffusion_hmap->r[i] = ratio * 255;
        }

        // If initial affected cnt >  final affected cnt, color is YELLOWISH GREEN
        // If initial affected cnt == final affected cnt, color is BLACK
        // If initial affected cnt  < final affected cnt, color is RED
        if (initial_affected_cnt[i] >= final_affected_cnt) {
            double ratio = ( (double)(initial_affected_cnt[i] - final_affected_cnt) ) /
                                                                    initial_affected_cnt[i];
            disease_growth_hmap->g[i] = ratio * 255;

        } else {
            double ratio = ( (double)(final_affected_cnt - initial_affected_cnt[i]) ) /
                                                                    final_affected_cnt;
            disease_growth_hmap->r[i] = ratio * 255;
        }
        i++;
    }

    // TODO
    std::string out_fname("");
    ConfigFileHandler->getInstance()->writeSimulationOutputToJsonFile(out_fname);

    population_diffusion_hmap->write("population_diffusion_hmap.ppm");
    disease_growth_hmap->write("disease_growth_hmap.ppm");

    delete population_diffusion_hmap;
    delete disease_growth_hmap;

    return 0;
}