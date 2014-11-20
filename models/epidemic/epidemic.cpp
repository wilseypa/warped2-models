#include "epidemic.hpp"
#include "tclap/ValueArg.h"

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
                std::shared_ptr<Person> person = 
                    diffusion_network_->pickPerson(state_.current_population_);

                if (person) {
                    events.emplace_back(new EpidemicEvent {selected_location, 
                                            timestamp + travel_time, person, DIFFUSION});
                    state_.current_population_.erase(person->pid_);
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
    TCLAP::ValueArg<std::string> configuration_arg("c", "config", 
            "Epidemic model configuration", false, configuration_filename, "string");
    std::vector<TCLAP::Arg*> cmd_line_args = {&configuration_arg};
    warped::Simulation simulation {"Epidemic Simulation", argc, argv, cmd_line_args};
    configuration_filename = configuration_arg.getValue();

#if 0
    unsigned int num_regions = 0, num_locations = 0, num_persons = 0, pid = 0, 
                travel_time_to_hub = 0, latent_dwell_time = 0, incubating_dwell_time = 0, 
                infectious_dwell_time = 0, asympt_dwell_time = 0, loc_state_refresh_interval = 0, 
                loc_diffusion_trig_interval = 0, disease_seed = 0, diffusion_seed = 0;
    double susceptibility = 0.0;
    float transmissibility = 0.0, prob_ulu = 0.0, prob_ulv = 0.0, prob_urv = 0.0, 
                prob_uiv = 0.0, prob_uiu = 0.0, latent_infectivity = 0.0, 
                incubating_infectivity = 0.0, infectious_infectivity = 0.0, 
                asympt_infectivity = 0.0;
    std::string location_name = "", infection_state = "", vaccination_status = "", 
                region_name = "", model = "";

    std::vector<std::shared_ptr<Person>> population;
    std::map <std::string, unsigned int> travel_map;

    XMLDocument epidemic_config;
    int error_id = epidemic_config.LoadFile(configuration_filename.c_str());
    if (error_id) {
        abort();
    }

    std::unique_ptr<XMLElement> diffusion = nullptr, disease = nullptr, 
        num_regions = nullptr, region = nullptr, location = nullptr, people = nullptr;

    diffusion = EpidemicConfig.FirstChildElement()->FirstChildElement("diffusion");
    model.assign(diffusion->FirstChildElement("model")->GetText());
    diffusion->FirstChildElement("seed")->QueryUnsignedText(&diffusion_seed);

    disease = EpidemicConfig.FirstChildElement()->FirstChildElement("disease");
    disease->FirstChildElement("transmissibility")->QueryFloatText(&transmissibility);
    disease->FirstChildElement("latent_dwell_time")->QueryUnsignedText(&latent_dwell_time);
    disease->FirstChildElement("latent_infectivity")->QueryFloatText(&latent_infectivity);
    disease->FirstChildElement("incubating_dwell_time")->QueryUnsignedText(&incubating_dwell_time);
    disease->FirstChildElement("incubating_infectivity")->QueryFloatText(&incubating_infectivity);
    disease->FirstChildElement("infectious_dwell_time")->QueryUnsignedText(&infectious_dwell_time);
    disease->FirstChildElement("infectious_infectivity")->QueryFloatText(&infectious_infectivity);
    disease->FirstChildElement("asympt_dwell_time")->QueryUnsignedText(&asympt_dwell_time);
    disease->FirstChildElement("asympt_infectivity")->QueryFloatText(&asympt_infectivity);
    disease->FirstChildElement("prob_ul_u")->QueryFloatText(&prob_ulu);
    disease->FirstChildElement("prob_ul_v")->QueryFloatText(&prob_ulv);
    disease->FirstChildElement("prob_ur_v")->QueryFloatText(&prob_urv);
    disease->FirstChildElement("prob_ui_v")->QueryFloatText(&prob_uiv);
    disease->FirstChildElement("prob_ui_u")->QueryFloatText(&prob_uiu);
    disease->FirstChildElement("location_state_refresh_interval")->QueryUnsignedText(
                                                                    &loc_state_refresh_interval);
    disease->FirstChildElement("seed")->QueryUnsignedText(&disease_seed);

    num_of_regions = EpidemicConfig.FirstChildElement()->FirstChildElement("number_of_regions");
    num_of_regions->QueryIntText(&num_regions);
    region = num_of_regions;

    /* For each region in the simulation, initialize the locations */
    for( int regIndex = 0; regIndex < numRegions; regIndex++ ) {
        region =region->NextSiblingElement();
        regionName.assign( region->FirstChildElement("region_name")->GetText() );

        numLocations = 0;
        region->FirstChildElement("number_of_locations")->QueryIntText(&numLocations);
        location = region->FirstChildElement("number_of_locations");
        locObjs = new vector<SimulationObject*>;

        for( int locIndex = 0; locIndex < numLocations; locIndex++ ) {
            personVec = new vector <Person *>;

            location = location->NextSiblingElement();
            locationName.assign( location->FirstChildElement("location_name")->GetText() );          
            location->FirstChildElement("number_of_persons")->QueryIntText(&numPersons);            
            location->FirstChildElement("travel_time_to_central_hub")->QueryUnsignedText(&travelTimeToHub);         
            location->FirstChildElement("diffusion_trigger_interval")->QueryUnsignedText(&locDiffusionTrigInterval);            
            people = location->FirstChildElement("diffusion_trigger_interval");

            locationName += ",";
            locationName += regionName;

            travelMap.insert( pair <string, unsigned int>(locationName, travelTimeToHub) );

            /* Read each person's details */
            for(int perIndex = 0; perIndex < numPersons; perIndex++) {
                people= people->NextSiblingElement();
                people->FirstChildElement("pid")->QueryUnsignedText(&pid);
                people->FirstChildElement("susceptibility")->QueryDoubleText(&susceptibility);
                vaccinationStatus.assign( people->FirstChildElement("is_vaccinated")->GetText() );
                infectionState.assign( people->FirstChildElement("infection_state")->GetText() );

                person = new Person( pid, susceptibility, vaccinationStatus, infectionState, INIT_VTIME, INIT_VTIME );
                personVec->push_back( person );
            }

            LocationObject *locObject = new LocationObject( locationName, transmissibility,
                                                            latentDwellTime, incubatingDwellTime,
                                                            infectiousDwellTime, asymptDwellTime,
                                                            latentInfectivity, incubatingInfectivity,
                                                            infectiousInfectivity, asymptInfectivity,
                                                            probULU, probULV, probURV, probUIV, probUIU,
                                                            (dataCaptureStatus == "yes") ? true : false, 
                                                            locStateRefreshInterval, locDiffusionTrigInterval, 
                                                            personVec, travelTimeToHub, diseaseSeed, diffusionSeed );
            locObjs->push_back(locObject);
            simulationObjMap.insert( pair <string, SimulationObject *>(locationName, locObject) );
            simulationObjVec->push_back(locObject);
        }

        /* Add the group of objects to the partition information */
        myPartitioner->addObjectGroup(locObjs);
    }

    if (model == "FullyConnected") {
        for( map <string, SimulationObject*>::iterator mapIter = simulationObjMap.begin();
                                            mapIter != simulationObjMap.end(); mapIter++ ) {

            LocationObject *locObj = static_cast <LocationObject *> (mapIter->second);
            map <string, unsigned int> tempTravelMap = travelMap;
            tempTravelMap.erase(mapIter->first);
            locObj->populateTravelMap(tempTravelMap);
        }

    } else if( model == "WattsStrogatz" ) {
        /* Refer to README for more details */
        unsigned int k = 0;
        float beta = 0.0;
        XMLElement *wattsStrogatz = NULL;

        wattsStrogatz = diffusion->FirstChildElement("watts_strogatz");
        wattsStrogatz->FirstChildElement("k")->QueryUnsignedText(&k);
        wattsStrogatz->FirstChildElement("beta")->QueryFloatText(&beta);

        WattsStrogatzModel *wsModel = new WattsStrogatzModel(k, beta, diffusionSeed);

        vector <string> nodeVec;
        for( map <string, unsigned int>::iterator mapIter = travelMap.begin();
                                            mapIter != travelMap.end(); mapIter++ ) {
            string location = static_cast <string> (mapIter->first);
            nodeVec.push_back(location);
        }
        wsModel->populateNodes(nodeVec);
        wsModel->mapNodes();

        for( map <string, SimulationObject*>::iterator mapIter = simulationObjMap.begin();
                                            mapIter != simulationObjMap.end(); mapIter++ ) {

            LocationObject *locObj = static_cast <LocationObject *> (mapIter->second);
            vector <string> connVec = wsModel->fetchNodeLinks(mapIter->first);
            map <string, unsigned int> tempTravelMap;
            for( vector <string>::iterator connVecIter = connVec.begin(); 
                                    connVecIter != connVec.end(); connVecIter++ ) {

                map <string, unsigned int>::iterator travelMapIter = travelMap.find(*connVecIter);
                tempTravelMap.insert( 
                    pair <string, unsigned int>(travelMapIter->first, travelMapIter->second) );
            }
            locObj->populateTravelMap(tempTravelMap);
        }
        delete wsModel;
    }

    std::vector<Location> objects;
    for (unsigned int i = 0; i < num_objects; ++i) {
        objects.emplace_back();
    }

    std::vector<warped::SimulationObject*> object_pointers;
    for (auto& o : objects) {
        object_pointers.push_back(&o);
    }
    simulation.simulate(object_pointers);
#endif

    return 0;
}
