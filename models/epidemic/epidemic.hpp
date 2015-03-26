#ifndef EPIDEMIC_HPP
#define EPIDEMIC_HPP

#include <string>
#include <vector>
#include <map>
#include "memory.hpp"
#include "warped.hpp"
#include "Person.hpp"
#include "RandomNumGenerator.hpp"
#include "DiseaseModel.hpp"
#include "DiffusionNetwork.hpp"

WARPED_DEFINE_OBJECT_STATE_STRUCT(LocationState) {

    LocationState () = default;
    LocationState (const LocationState& other) {
        current_population_.clear();
        for (auto it = other.current_population_.begin(); 
                            it != other.current_population_.end(); it++) {
            auto person = it->second;
            auto new_person = 
                std::make_shared<Person>(   person->pid_, 
                                            person->susceptibility_, 
                                            person->vaccination_status_, 
                                            person->infection_state_, 
                                            person->loc_arrival_timestamp_, 
                                            person->prev_state_change_timestamp_    );
            current_population_.insert(current_population_.begin(), 
                std::pair <unsigned int, std::shared_ptr<Person>> (person->pid_, new_person));
        }
    };

    std::map <unsigned int, std::shared_ptr<Person>> current_population_;
};

enum event_type_t {

    DISEASE_UPDATE_TRIGGER,
    DIFFUSION_TRIGGER,
    DIFFUSION
};

class EpidemicEvent : public warped::Event {
public:

    EpidemicEvent() = default;

    EpidemicEvent(const std::string receiver_name, unsigned int timestamp, 
                            std::shared_ptr<Person> person, event_type_t event_type)
            : receiver_name_(receiver_name), loc_arrival_timestamp_(timestamp), 
                event_type_(event_type) {

        if (person != nullptr) {
            pid_ = person->pid_;
            susceptibility_ = person->susceptibility_;
            vaccination_status_ = person->vaccination_status_;
            infection_state_ = person->infection_state_;
            prev_state_change_timestamp_ = person->prev_state_change_timestamp_;
        }
    }

    const std::string& receiverName() const { return receiver_name_; }
    unsigned int timestamp() const { return loc_arrival_timestamp_; }

    std::string receiver_name_;
    unsigned int pid_;
    double susceptibility_;
    bool vaccination_status_;
    infection_state_t infection_state_;
    unsigned int loc_arrival_timestamp_;
    unsigned int prev_state_change_timestamp_;
    event_type_t event_type_;

    WARPED_REGISTER_SERIALIZABLE_MEMBERS(cereal::base_class<warped::Event>(this), 
                receiver_name_, pid_, susceptibility_, vaccination_status_, infection_state_, 
                loc_arrival_timestamp_, prev_state_change_timestamp_, event_type_)
};

class Location : public warped::SimulationObject {
public:

    Location(const std::string& name, float transmissibility, unsigned int latent_dwell_interval, 
                unsigned int incubating_dwell_interval, unsigned int infectious_dwell_interval, 
                unsigned int asympt_dwell_interval, float latent_infectivity, 
                float incubating_infectivity, float infectious_infectivity, 
                float asympt_infectivity, float prob_ulu, float prob_ulv, float prob_urv, 
                float prob_uiv, float prob_uiu, unsigned int loc_state_refresh_interval, 
                unsigned int loc_diffusion_trig_interval, 
                std::vector<std::shared_ptr<Person>> population, unsigned int travel_time_to_hub, 
                unsigned int disease_seed, unsigned int diffusion_seed)
            : SimulationObject(name), state_(), location_name_(name), 
                location_state_refresh_interval_(loc_state_refresh_interval), 
                location_diffusion_trigger_interval_(loc_diffusion_trig_interval) {

        disease_model_ = 
            std::make_shared<DiseaseModel>(
                    transmissibility, latent_dwell_interval, incubating_dwell_interval, 
                    infectious_dwell_interval, asympt_dwell_interval, latent_infectivity, 
                    incubating_infectivity, infectious_infectivity, asympt_infectivity, 
                    prob_ulu, prob_ulv, prob_urv, prob_uiv, prob_uiu, disease_seed);

        diffusion_network_ = 
            std::make_shared<DiffusionNetwork>(diffusion_seed, travel_time_to_hub);

        for (auto& person : population) {
            state_.current_population_.insert(state_.current_population_.begin(), 
                std::pair <unsigned int, std::shared_ptr<Person>> (person->pid_, person));
        }
    }

    virtual warped::ObjectState& getState() { return state_; }

    virtual std::vector<std::shared_ptr<warped::Event>> createInitialEvents();

    virtual std::vector<std::shared_ptr<warped::Event>> receiveEvent(const warped::Event& event);

    void populateTravelDistances(std::map<std::string, unsigned int> travel_chart) {

        diffusion_network_->populateTravelChart(travel_chart);
    }

    std::string getLocationName() {

        return location_name_;
    }

protected:

    LocationState state_;
    std::string location_name_;
    std::shared_ptr<DiseaseModel> disease_model_;
    std::shared_ptr<DiffusionNetwork> diffusion_network_;
    unsigned int location_state_refresh_interval_;
    unsigned int location_diffusion_trigger_interval_;
};

#endif
