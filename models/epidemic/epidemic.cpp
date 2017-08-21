#include <fstream>
#include "epidemic.hpp"
#include "Graph.hpp"
#include "ppm/ppm.hpp"
#include "tclap/ValueArg.h"

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(EpidemicEvent)

std::vector<std::shared_ptr<warped::Event> > Location::initializeLP() {

    // Register random number generator to allow kernel to roll it back
    this->registerRNG<std::default_random_engine>(this->rng_);

    std::vector<std::shared_ptr<warped::Event> > events;
    events.emplace_back(new EpidemicEvent {this->location_name_, 
                    this->location_state_refresh_interval_, nullptr, DISEASE_UPDATE_TRIGGER});
    events.emplace_back(new EpidemicEvent {this->location_name_, 
                    this->location_diffusion_trigger_interval_, nullptr, DIFFUSION_TRIGGER});
    return events;
}

std::vector<std::shared_ptr<warped::Event> > Location::receiveEvent(const warped::Event& event) {

    std::vector<std::shared_ptr<warped::Event> > events;
    auto epidemic_event = static_cast<const EpidemicEvent&>(event);
    auto timestamp = epidemic_event.loc_arrival_timestamp_;

    switch (epidemic_event.event_type_) {

        case event_type_t::DISEASE_UPDATE_TRIGGER: {
            std::uniform_real_distribution<double> distribution(0.0, 1.0);
            auto rand_factor = distribution(*rng_);
            disease_model_->reaction(state_->current_population_, timestamp, rand_factor);
            events.emplace_back(new EpidemicEvent {location_name_, 
                                timestamp + location_state_refresh_interval_, 
                                nullptr, DISEASE_UPDATE_TRIGGER});
        } break;

        case event_type_t::DIFFUSION_TRIGGER: {
            std::string selected_location = diffusion_network_->pickLocation();
            if (selected_location != "") {
                auto travel_time = diffusion_network_->travelTimeToLocation(selected_location);
                unsigned int person_count = state_->current_population_->size();
                if (person_count) {
                    unsigned int person_id = diffusion_network_->pickPerson(person_count);
                    auto map_iter = state_->current_population_->begin();
                    unsigned int temp_cnt = 0;
                    while (temp_cnt < person_id) {
                        map_iter++;
                        temp_cnt++;
                    }
                    std::shared_ptr<Person> person = map_iter->second;
                    events.emplace_back(new EpidemicEvent {selected_location, 
                                            timestamp + travel_time, person, DIFFUSION});
                    state_->current_population_->erase(map_iter);
                }
            }
            events.emplace_back(new EpidemicEvent {location_name_, 
                                timestamp + location_diffusion_trigger_interval_, 
                                nullptr, DIFFUSION_TRIGGER});
        } break;

        case event_type_t::DIFFUSION: {
            std::shared_ptr<Person> person = std::make_shared<Person>(
                        epidemic_event.pid_, epidemic_event.susceptibility_, 
                        epidemic_event.vaccination_status_, epidemic_event.infection_state_,
                        timestamp, epidemic_event.prev_state_change_timestamp_);
            state_->current_population_->insert(state_->current_population_->begin(), 
                std::pair <unsigned long, std::shared_ptr<Person>> (epidemic_event.pid_, person));
        } break;

        default: {}
    }
    return events;
}

int main(int argc, const char** argv) {

    std::string config_filename = "model_10k.dat";
    TCLAP::ValueArg<std::string> config_arg("m", "model", 
            "Epidemic model config", false, config_filename, "string");
    std::vector<TCLAP::Arg*> args = {&config_arg};

    warped::Simulation epidemic_sim {"Epidemic Simulation", argc, argv, args};

    config_filename = config_arg.getValue();

    std::ifstream config_stream;
    config_stream.open(config_filename);
    if (!config_stream.is_open()) {
        std::cerr << "Invalid configuration file - " << config_filename << std::endl;
        return 0;
    }

    std::string buffer;
    std::string delimiter = ",";
    size_t pos = 0;
    std::string token;

    // Diffusion model
    getline(config_stream, buffer);
    pos = buffer.find(delimiter);
    std::string graph_type = buffer.substr(0, pos);
    buffer.erase(0, pos + delimiter.length());
    std::string diffusion_params(buffer);

    // Disease model
    getline(config_stream, buffer);
    float transmissibility = std::stof(buffer);

    getline(config_stream, buffer);
    pos = buffer.find(delimiter);
    token = buffer.substr(0, pos);
    unsigned int latent_dwell_time = (unsigned int) std::stoul(token);
    buffer.erase(0, pos + delimiter.length());
    float latent_infectivity = std::stof(buffer);

    getline(config_stream, buffer);
    pos = buffer.find(delimiter);
    token = buffer.substr(0, pos);
    unsigned int incubating_dwell_time = (unsigned int) std::stoul(token);
    buffer.erase(0, pos + delimiter.length());
    float incubating_infectivity = std::stof(buffer);

    getline(config_stream, buffer);
    pos = buffer.find(delimiter);
    token = buffer.substr(0, pos);
    unsigned int infectious_dwell_time = (unsigned int) std::stoul(token);
    buffer.erase(0, pos + delimiter.length());
    float infectious_infectivity = std::stof(buffer);

    getline(config_stream, buffer);
    pos = buffer.find(delimiter);
    token = buffer.substr(0, pos);
    unsigned int asympt_dwell_time = (unsigned int) std::stoul(token);
    buffer.erase(0, pos + delimiter.length());
    float asympt_infectivity = std::stof(buffer);

    getline(config_stream, buffer);
    pos = buffer.find(delimiter);
    token = buffer.substr(0, pos);
    float prob_ulu = stof(token);
    buffer.erase(0, pos + delimiter.length());
    pos = buffer.find(delimiter);
    token = buffer.substr(0, pos);
    float prob_ulv = std::stof(token);
    buffer.erase(0, pos + delimiter.length());
    pos = buffer.find(delimiter);
    token = buffer.substr(0, pos);
    float prob_urv = std::stof(token);
    buffer.erase(0, pos + delimiter.length());
    pos = buffer.find(delimiter);
    token = buffer.substr(0, pos);
    float prob_uiv = std::stof(token);
    buffer.erase(0, pos + delimiter.length());
    float prob_uiu = std::stof(buffer);

    getline(config_stream, buffer);
    unsigned int location_state_refresh_interval = (unsigned int) stoul(buffer);

    //Population
    getline(config_stream, buffer);
    unsigned int num_regions = (unsigned int) std::stoul(buffer);

    std::map<std::string, unsigned int> travel_map;
    std::vector<Location> lps;

    for (unsigned int region_id = 0; region_id < num_regions; region_id++) {

        getline(config_stream, buffer);
        pos = buffer.find(delimiter);
        std::string region_name = buffer.substr(0, pos);
        buffer.erase(0, pos + delimiter.length());
        unsigned int num_locations = (unsigned int) std::stoul(buffer);

        for (unsigned int location_id = 0; location_id < num_locations; location_id++) {

            getline(config_stream, buffer);
            pos = buffer.find(delimiter);
            std::string location_name = buffer.substr(0, pos);
            std::string location = region_name + location_name;
            buffer.erase(0, pos + delimiter.length());
            pos = buffer.find(delimiter);
            token = buffer.substr(0, pos);
            unsigned int travel_time_to_hub = (unsigned int) std::stoul(token);
            buffer.erase(0, pos + delimiter.length());
            pos = buffer.find(delimiter);
            token = buffer.substr(0, pos);
            unsigned int diffusion_interval = (unsigned int) std::stoul(token);
            buffer.erase(0, pos + delimiter.length());
            unsigned int num_persons = (unsigned int) std::stoul(buffer);

            std::vector<std::shared_ptr<Person>> population;
            travel_map.insert(std::pair<std::string, unsigned int>(location, travel_time_to_hub));

            for (unsigned int person_id = 0; person_id < num_persons; person_id++) {

                getline(config_stream, buffer);
                pos = buffer.find(delimiter);
                token = buffer.substr(0, pos);
                unsigned long pid = std::stoul(token);
                buffer.erase(0, pos + delimiter.length());
                pos = buffer.find(delimiter);
                token = buffer.substr(0, pos);
                double susceptibility = std::stod(buffer);
                buffer.erase(0, pos + delimiter.length());
                pos = buffer.find(delimiter);
                token = buffer.substr(0, pos);
                bool vaccination_status = (bool) std::stoi(token);
                buffer.erase(0, pos + delimiter.length());
                infection_state_t state = (infection_state_t) std::stoi(buffer);

                auto person = std::make_shared<Person> (    pid,
                                                            susceptibility,
                                                            vaccination_status,
                                                            state,
                                                            0,
                                                            0
                                                       );
                population.push_back(person);
            }
            lps.emplace_back(   location,
                                    transmissibility,
                                    latent_dwell_time,
                                    incubating_dwell_time,
                                    infectious_dwell_time,
                                    asympt_dwell_time,
                                    latent_infectivity,
                                    incubating_infectivity,
                                    infectious_infectivity,
                                    asympt_infectivity,
                                    prob_ulu,
                                    prob_ulv,
                                    prob_urv,
                                    prob_uiv,
                                    prob_uiu,
                                    location_state_refresh_interval,
                                    diffusion_interval,
                                    population,
                                    travel_time_to_hub, 
                                    location_id
                                );
        }
    }
    config_stream.close();

    // Create the Network Graph
    std::vector<std::string> nodes;
    for (auto& lp : lps) {
        nodes.push_back(lp.getLocationName());
    }

    Graph *graph = nullptr;
    if (graph_type == "Watts-Strogatz") { // If the choice is Watts-Strogatz
        pos = diffusion_params.find(delimiter);
        token = diffusion_params.substr(0, pos);
        unsigned int k = (unsigned int) std::stoul(token);
        diffusion_params.erase(0, pos + delimiter.length());
        double beta = std::stod(diffusion_params);
        graph = new WattsStrogatz(nodes, k, beta);

    } else if (graph_type == "Barabasi-Albert") { // If the choice is Barabasi-Albert
        pos = diffusion_params.find(delimiter);
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
    unsigned long i = 0;
    auto heatmap = new ppm(100, (lps.size()+99)/100);
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
        unsigned long population_size = 0, affected_cnt = 0;
        lp.statistics(&population_size, &affected_cnt);
        double frac_affected = ((double) affected_cnt) / population_size;
        heatmap->g[i] = frac_affected * 255;
        heatmap->b[i] = frac_affected * 255;
        i++;
    }
    delete graph;

    std::vector<warped::LogicalProcess*> lp_pointers;
    for (auto& lp : lps) {
        lp_pointers.push_back(&lp);
    }
    epidemic_sim.simulate(lp_pointers);

    // Record final statistics for each LP
    i = 0;
    for (auto& lp : lps) {
        unsigned long population_size = 0, affected_cnt = 0;
        lp.statistics(&population_size, &affected_cnt);
        double frac_affected = ((double) affected_cnt) / population_size;
        heatmap->r[i] = frac_affected * 255;
        i++;
    }
    heatmap->write("heatmap.ppm");
    delete heatmap;

    return 0;
}
