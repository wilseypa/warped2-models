#include <fstream>
#include <sstream>
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

std::string toString(unsigned int num) {
    std::ostringstream convert;
    convert << num;
    return convert.str();
}

int main(int argc, const char** argv) {

    unsigned int num_regions        = 256;
    unsigned int num_locations      = 64;
    unsigned int population         = 500;
    std::string graph_type          = "ws";
    std::string model_config_name   = "*";

    TCLAP::ValueArg<unsigned int> num_regions_arg("r", "num-regions", "Number of regions",
                                                            false, num_regions, "unsigned int");
    TCLAP::ValueArg<unsigned int> num_locations_arg("l", "num-locations", "Number of locations",
                                                            false, num_locations, "unsigned int");
    TCLAP::ValueArg<unsigned int> population_arg("p", "population", "Population",
                                                            false, population, "unsigned int");
    TCLAP::ValueArg<std::string> graph_type_arg( "n", "graph-type", "Graph type (ws/ba)",
                                                            false, graph_type, "string" );
    TCLAP::ValueArg<std::string> model_config_name_arg( "m", "model-config",
            "Skip config creation by providing name of the model config. No other flags needed",
                                                            false, model_config_name, "string" );
    std::vector<TCLAP::Arg*> args = {&num_regions_arg, &num_locations_arg, &population_arg,
                                               &graph_type_arg, &model_config_name_arg};

    warped::Simulation epidemic_sim {"Epidemic Simulation", argc, argv, args};

    num_regions         = num_regions_arg.getValue();
    num_locations       = num_locations_arg.getValue();
    population          = population_arg.getValue();
    graph_type          = graph_type_arg.getValue();
    model_config_name   = model_config_name_arg.getValue();

    if (model_config_name == "*") {
        model_config_name =     std::string("model")
                                + "-lp" + toString(num_regions * num_locations)
                                + "-p"  + toString(num_regions * num_locations * population)
                                + "-"    + graph_type
                                + ".config";

        unsigned int  graph_val = (graph_type == "ws") ? 1 : 0;
        std::string command =   "./config/create_config "
                            +   model_config_name       + " "
                            +   toString(num_regions)   + " "
                            +   toString(num_locations) + " "
                            +   toString(population)    + " "
                            +   toString(graph_val);

        std::cout << "Build the config file using the command:\n\t" << command << std::endl;
        if (std::system(command.c_str())) {
            exit(EXIT_FAILURE);
        }
        std::cout << "Created epidemic config file: " << model_config_name << std::endl;
    }

    std::ifstream config_stream;
    config_stream.open(model_config_name);
    if (!config_stream.is_open()) {
        std::cerr << "Invalid configuration file - " << model_config_name << std::endl;
        return 0;
    }

    std::string buffer;
    std::string delimiter = ",";
    size_t pos = 0;
    std::string token;

    // Diffusion model
    getline(config_stream, buffer);
    pos = buffer.find(delimiter);
    graph_type = buffer.substr(0, pos);
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
    float prob_urv = std::stof(buffer);

    getline(config_stream, buffer);
    unsigned int location_state_refresh_interval = (unsigned int) stoul(buffer);

    //Population
    getline(config_stream, buffer);
    num_regions = (unsigned int) std::stoul(buffer);

    std::map<std::string, unsigned int> travel_map;
    std::vector<Location> lps;

    for (unsigned int region_id = 0; region_id < num_regions; region_id++) {

        getline(config_stream, buffer);
        pos = buffer.find(delimiter);
        std::string region_name = buffer.substr(0, pos);
        buffer.erase(0, pos + delimiter.length());
        num_locations = (unsigned int) std::stoul(buffer);

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
    epidemic_sim.simulate(lp_pointers);

    // Plot the heatmaps
    unsigned long cols = std::ceil( sqrt(lps.size()) );
    unsigned long rows = (lps.size()+cols-1)/cols;
    auto population_diffusion_hmap = new ppm(cols, rows);
    auto disease_growth_hmap = new ppm(cols, rows);

    // Record final statistics for each LP
    i = 0;
    for (auto& lp : lps) {
        unsigned long final_population = 0, final_affected_cnt = 0;
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
