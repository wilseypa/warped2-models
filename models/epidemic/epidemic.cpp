#include "epidemic.hpp"
#include "WattsStrogatzModel.hpp"
#include "tclap/ValueArg.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

using namespace boost::property_tree;

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(EpidemicEvent)

std::vector<std::shared_ptr<warped::Event> > Location::createInitialEvents() {

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
            disease_model_->reaction(state_.current_population_, timestamp);
            events.emplace_back(new EpidemicEvent {location_name_, 
                                timestamp + location_state_refresh_interval_, 
                                nullptr, DISEASE_UPDATE_TRIGGER});
        } break;

        case event_type_t::DIFFUSION_TRIGGER: {
            std::string selected_location = diffusion_network_->pickLocation();
            if (selected_location != "") {
                auto travel_time = diffusion_network_->travelTimeToLocation(selected_location);
                unsigned int person_count = state_.current_population_.size();
                unsigned int person_id = diffusion_network_->pickPerson(person_count);
                if (person_count) {
                    std::shared_ptr<Person> person = state_.current_population_[person_id];
                    events.emplace_back(new EpidemicEvent {selected_location, 
                                            timestamp + travel_time, person, DIFFUSION});
                    state_.current_population_.erase(person_id);
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
            state_.current_population_.insert( 
                std::pair <unsigned int, std::shared_ptr<Person>> (epidemic_event.pid_, person));
        } break;

        default: {}
    }
    return events;
}

int main(int argc, const char** argv) {

    std::string configuration_filename = "";
    TCLAP::ValueArg<std::string> configuration_arg("m", "model", 
            "Epidemic model configuration", false, configuration_filename, "string");
    std::vector<TCLAP::Arg*> cmd_line_args = {&configuration_arg};
    warped::Simulation simulation {"Epidemic Simulation", argc, argv, cmd_line_args};
    configuration_filename = configuration_arg.getValue();

    unsigned int num_regions = 0, num_locations = 0, pid = 0, travel_time_to_hub = 0, 
                latent_dwell_time = 0, incubating_dwell_time = 0, infectious_dwell_time = 0, 
                asympt_dwell_time = 0, loc_state_refresh_interval = 0, 
                loc_diffusion_trig_interval = 0, disease_seed = 0, diffusion_seed = 0, k = 0;
    double susceptibility = 0.0;
    float transmissibility = 0.0, prob_ulu = 0.0, prob_ulv = 0.0, prob_urv = 0.0, 
                prob_uiv = 0.0, prob_uiu = 0.0, latent_infectivity = 0.0, 
                incubating_infectivity = 0.0, infectious_infectivity = 0.0, 
                asympt_infectivity = 0.0, beta = 0.0;
    std::string infection_state = "", vaccination_status = "", model = "";
    std::map<std::string, unsigned int> travel_map;
    std::vector<Location> objects;

    ptree tree;
    xml_parser::read_xml(configuration_filename, tree);

    BOOST_FOREACH (ptree::value_type const& v, tree.get_child("epidemic_configuration")) {
        if (v.first == "diffusion") {
            model = v.second.get<std::string>("model");
            diffusion_seed = v.second.get<unsigned int>("seed");
            if (model == "WattsStrogatz") {
                k = v.second.get<unsigned int>("watts_strogatz.k");
                beta = v.second.get<float>("watts_strogatz.beta");
            }
        } else if (v.first == "disease") {
            transmissibility = v.second.get<float>("transmissibility");
            latent_dwell_time = v.second.get<unsigned int>("latent_dwell_time");
            latent_infectivity = v.second.get<float>("latent_infectivity");
            incubating_dwell_time = v.second.get<unsigned int>("incubating_dwell_time");
            incubating_infectivity = v.second.get<float>("incubating_infectivity");
            infectious_dwell_time = v.second.get<unsigned int>("infectious_dwell_time");
            infectious_infectivity = v.second.get<float>("infectious_infectivity");
            asympt_dwell_time = v.second.get<unsigned int>("asympt_dwell_time");
            asympt_infectivity = v.second.get<float>("asympt_infectivity");
            prob_ulu = v.second.get<float>("prob_ul_u");
            prob_ulv = v.second.get<float>("prob_ul_v");
            prob_urv = v.second.get<float>("prob_ur_v");
            prob_uiv = v.second.get<float>("prob_ui_v");
            prob_uiu = v.second.get<float>("prob_ui_u");
            loc_state_refresh_interval = 
                v.second.get<unsigned int>("location_state_refresh_interval");
            disease_seed = v.second.get<unsigned int>("seed");

        } else if (v.first == "population") {
            BOOST_FOREACH (ptree::value_type const& w, tree.get_child("population", v.second)) {
                std::string region_name = std::string("region_") + std::to_string(num_regions);
                num_locations = 0;
                BOOST_FOREACH (ptree::value_type const& x, tree.get_child(region_name, w.second)) {
                    std::string location_name = std::string("location_") + std::to_string(num_locations);
                    std::string location = region_name + std::string("-") + location_name;
                    std::vector<std::shared_ptr<Person>> population;
                    BOOST_FOREACH (ptree::value_type const& y, tree.get_child(location_name, x.second)) {
                        if (y.first == "travel_time_to_central_hub") {
                            travel_time_to_hub = 
                                (unsigned int) std::stoi(std::string(y.second.data()));
                            travel_map.insert(std::pair<std::string, unsigned int>(location, travel_time_to_hub));
                        } else if (y.first == "diffusion_trigger_interval") {
                            loc_diffusion_trig_interval = 
                                (unsigned int) std::stoi(std::string(y.second.data()));
                        } else {
                            pid++;
                            std::string person_pid = std::string("person_") +std::to_string(pid);
                            BOOST_FOREACH (ptree::value_type const& z, tree.get_child(person_pid, y.second)) {
                                if (z.first == "susceptibility") {
                                    susceptibility = std::stod(std::string(z.second.data()));
                                } else if (z.first == "is_vaccinated") {
                                    vaccination_status = std::string(z.second.data());
                                } else {
                                    infection_state = std::string(z.second.data());
                                }
                            }

                            infection_state_t state;
                            if (infection_state == "uninfected") {
                                state = UNINFECTED;
                            } else if (infection_state == "latent") {
                                state = LATENT;
                            } else if (infection_state == "incubating") {
                                state = INCUBATING;
                            } else if (infection_state == "infectious") {
                                state = INFECTIOUS;
                            } else if (infection_state == "asympt") {
                                state = ASYMPT;
                            } else {
                                state = RECOVERED;
                            }

                            auto person = 
                                std::make_shared<Person>(pid, susceptibility, 
                                        (vaccination_status == "yes") ? true : false, state, 0, 0);
                            population.push_back(person);
                        }
                    }
                    objects.emplace_back(location, transmissibility, latent_dwell_time, 
                            incubating_dwell_time, infectious_dwell_time, asympt_dwell_time, 
                            latent_infectivity, incubating_infectivity, infectious_infectivity, 
                            asympt_infectivity, prob_ulu, prob_ulv, prob_urv, prob_uiv, prob_uiu, 
                            loc_state_refresh_interval, loc_diffusion_trig_interval, 
                            population, travel_time_to_hub, disease_seed, diffusion_seed);

                    num_locations++;
                }
                num_regions++;
            }
        }
    }

    if (model == "FullyConnected") {
        for (auto &o : objects) {
            auto temp_travel_map = travel_map;
            temp_travel_map.erase(o.getLocationName());
            o.populateTravelDistances(temp_travel_map);
        }
    } else if (model == "WattsStrogatz") {
        auto ws = std::make_shared<WattsStrogatzModel>(k, beta, diffusion_seed);
        std::vector<std::string> nodes;
        for (auto& o : objects) {
            nodes.push_back(o.getLocationName());
        }
        ws->populateNodes(nodes);
        ws->mapNodes();

        for (auto& o : objects) {
            std::vector<std::string> connections = ws->fetchNodeLinks(o.getLocationName());
            std::map<std::string, unsigned int> temp_travel_map;
            for (auto& link : connections) {
                auto travel_map_iter = travel_map.find(link);
                temp_travel_map.insert(std::pair<std::string, unsigned int>
                                (travel_map_iter->first, travel_map_iter->second));
            }
            o.populateTravelDistances(temp_travel_map);
        }
    }

    std::vector<warped::SimulationObject*> object_pointers;
    for (auto& o : objects) {
        object_pointers.push_back(&o);
    }
    simulation.simulate(object_pointers);

    return 0;
}
