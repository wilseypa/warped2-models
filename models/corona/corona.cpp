#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include "corona.hpp"
#include "graph.hpp"
#include "ppm/ppm.hpp"
#include "tclap/ValueArg.h"

#include <string>
#include <algorithm>
#include <iterator>


#define DEFAULT_MODEL_NAME "*"

CoronaConfig* CoronaConfig::instance_ = nullptr;
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
        // TODO error: base operand of ‘->’ has non-pointer type ‘TriggerEvent’
        auto timestamp = trigger_event->timestamp();

        /* Check whether a diffusion needs to be triggered */
        // TODO error: base operand of ‘->’ has non-pointer type ‘TriggerEvent’
        if (trigger_event->is_diffusion_) {
            // TODO error: ‘diffusion_network_’ was not declared in this scope
            std::string selected_location = diffusion_network_->pickLocation();
            if (selected_location != "") {
                auto travel_time = diffusion_network_->travelTimeToLocation(selected_location);
                // TODO error: ‘using element_type = struct LocationState’ {aka ‘struct LocationState’} has no member named ‘current_population_’
                unsigned int person_count = state_->current_population_->size();
                if (person_count) {
                    unsigned int person_id = diffusion_network_->pickPerson(person_count);
                    // TODO error: ‘using element_type = struct LocationState’ {aka ‘struct LocationState’} has no member named ‘current_population_’
                    auto map_iter = state_->current_population_->begin();
                    unsigned int temp_cnt = 0;
                    while (temp_cnt < person_id) {
                        map_iter++;
                        temp_cnt++;
                    }
                    // TODO error: ‘Person’ was not declared in this scope
                    // TODO error: ‘Person’ was not declared in this scope
                    std::shared_ptr<Person> person = map_iter->second;
                    // TODO error: ‘EpidemicEvent’ does not name a type
                    events.emplace_back(new EpidemicEvent {selected_location,
                                            timestamp + travel_time, person, DIFFUSION});
                    // TODO error: ‘using element_type = struct LocationState’ {aka ‘struct LocationState’} has no member named ‘current_population_’
                    state_->current_population_->erase(map_iter);
                }
            }
            // TODO error: no matching function for call to ‘TriggerEvent::TriggerEvent(<brace-enclosed initializer list>)’
            events.emplace_back(new TriggerEvent {location_name_,
                                timestamp + CONFIG->diffusion_trig_interval_, true});

        } else { /* Update the LP state */
            std::uniform_real_distribution<double> distribution(0.0, 1.0);
            auto rand_factor = distribution(*rng_);
            reaction();
            // TODO error: no matching function for call to ‘TriggerEvent::TriggerEvent(<brace-enclosed initializer list>)’
            events.emplace_back(new TriggerEvent {location_name_,
                                timestamp + CONFIG->update_trig_interval_});
        }
    } else {
        // TODO error: ‘Person’ was not declared in this scope
        // TODO error: template argument 1 is invalid
        std::shared_ptr<Person> person = std::make_shared<Person>(
            // TODO error: ‘epidemic_event’ was not declared in this scope
                        epidemic_event.pid_, epidemic_event.susceptibility_,
                        epidemic_event.vaccination_status_, epidemic_event.infection_state_,
                        // TODO error: ‘timestamp’ was not declared in this scope
                        timestamp, epidemic_event.prev_state_change_timestamp_);
        // TODO error: ‘using element_type = struct LocationState’ {aka ‘struct LocationState’} has no member named ‘current_population_’
        state_->current_population_->insert(state_->current_population_->begin(),
                                            // TODO error: template argument 2 is invalid
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
    std::string model_config_name   = DEFAULT_MODEL_NAME;

    TCLAP::ValueArg<std::string> model_config_name_arg( "m", "model-config",
                        "Provide name of the model config", false, model_config_name, "string" );
    std::vector<TCLAP::Arg*> args = {&model_config_name_arg};
    warped::Simulation corona_sim {"Covid-19 Pandemic Simulation", argc, argv, args};
    model_config_name   = model_config_name_arg.getValue();

    if (model_config_name == DEFAULT_MODEL_NAME) {
        std::cout << "Need a valid model config filename.\n";
        exit(EXIT_FAILURE);
    }

    // commented existing file open code
    // std::ifstream config_stream;
    // config_stream.open(model_config_name);
    // if (!config_stream.is_open()) {
    //     exit(EXIT_FAILURE);
    // }

    std::map<std::string, unsigned int> travel_map;
    std::vector<Location> lps;

    CONFIGFILEHANDLER->getInstance()->openFile(model_config_name, &lps);
    CONFIGFILEHANDLER->getInstance()->getValuesFromJsonFile();


    // Create the Network Graph
    std::vector<std::string> nodes;
    for (auto& lp : lps) {
        nodes.push_back(lp.getLocationName());
    }

    Graph *graph = nullptr;
    // TODO error: ‘graph_type’ was not declared in this scope
    if (graph_type == "Watts-Strogatz") { // If the choice is Watts-Strogatz
        // TODO error: ‘pos’ was not declared in this scope; did you mean ‘pow’?
        // TODO error: ‘diffusion_params’ was not declared in this scope
        // TODO error: ‘delimiter’ was not declared in this scope
        pos = diffusion_params.find(delimiter);
        // TODO error: ‘token’ was not declared in this scope
        token = diffusion_params.substr(0, pos);
        unsigned int k = (unsigned int) std::stoul(token);
        diffusion_params.erase(0, pos + delimiter.length());
        double beta = std::stod(diffusion_params);
        graph = new WattsStrogatz(nodes, k, beta);

    } else if (graph_type == "Barabasi-Albert") { // If the choice is Barabasi-Albert
        // TODO error: ‘pos’ was not declared in this scope; did you mean ‘pow’?
        // TODO error: ‘diffusion_params’ was not declared in this scope
        // TODO error: ‘delimiter’ was not declared in this scope
        pos = diffusion_params.find(delimiter);
        // TODO error: ‘token’ was not declared in this scope
        token = diffusion_params.substr(0, pos);
        unsigned int m = (unsigned int) std::stoul(token);
        diffusion_params.erase(0, pos + delimiter.length());
        double a = std::stod(diffusion_params);
        graph = new BarabasiAlbert(nodes, m, a);

    } else { // Invalid choice
        std::cerr << "Invalid choice of diffusion network." << std::endl;
        return 0;
    }

    // Create the travel map and record initial statistics for each LP
    std::vector<unsigned long> initial_population( lps.size() );
    std::vector<unsigned long> initial_affected_cnt( lps.size() );
    unsigned long i = 0;
    for (auto& lp : lps) {
        // Create the travel map
        std::vector<std::string> connections = graph->adjacencyList(lp.getLocationName());
        std::map<std::string, unsigned int> temp_travel_map;
        for (auto& link : connections) {
            auto travel_map_iter = travel_map.find(link);
            temp_travel_map.insert(std::pair<std::string, unsigned int>
                                (travel_map_iter->first, travel_map_iter->second));
        }
        lp.populateTravelDistances(temp_travel_map);

        // Record the initial statistics
        unsigned long init_popu = 0, init_affected = 0;
        // TODO error: ‘class Location’ has no member named ‘statistics’
        lp.statistics(&init_popu, &init_affected);
        initial_population[i]   = init_popu;
        initial_affected_cnt[i] = init_affected;
        i++;
    }
    delete graph;

    // Simulate the epidemic
    std::vector<warped::LogicalProcess*> lp_pointers;
    for (auto& lp : lps) {
        lp_pointers.push_back(&lp);
    }
    corona_sim.simulate(lp_pointers);

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
    population_diffusion_hmap->write("population_diffusion_hmap.ppm");
    disease_growth_hmap->write("disease_growth_hmap.ppm");

    delete population_diffusion_hmap;
    delete disease_growth_hmap;

    return 0;
}
