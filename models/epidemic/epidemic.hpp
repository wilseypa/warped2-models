#ifndef EPIDEMIC_HPP
#define EPIDEMIC_HPP

#include "warped.hpp"
#include "Person.hpp"
#include "RandomNumGenerator.hpp"
#include "DiseaseModel.hpp"
#include "DiffusionNetwork.hpp"

WARPED_DEFINE_OBJECT_STATE_STRUCT(LocationState) {

    std::map <unsigned int, std::shared_ptr<Person>> current_population_;
};

enum event_type_t {

    DISEASE_UPDATE_TRIGGER,
    DIFFUSION_TRIGGER,
    DIFFUSION
};

class EpidemicEvent : public warped::Event {

    EpidemicEvent() = default;

    EpidemicEvent(const std::string receiver_name, const unsigned int timestamp, 
                            std::shared_ptr<Person> person, event_type_t event_type)
            : receiver_name_(receiver_name), timestamp_(timestamp), event_type_(event_type) {

        if (person != nullptr) {
            pid_ = person->pid_;
            susceptibility_ = person->susceptibility_;
            vaccination_status_ = person->vaccination_status_;
            infection_state_ = person->infection_state_;
            loc_arrival_timestamp_ = person->loc_arrival_timestamp_;
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
                                    receiver_name_, pid_, susceptibility_, 
                                    vaccination_status_, infection_state_,
                                    loc_arrival_timestamp_, prev_state_change_timestamp_)
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
        : SimulationObject(name), state_(), location_name_(name) {
        
        // TODO
    }

    virtual warped::ObjectState& getState() { return state_; }

    virtual std::vector<std::shared_ptr<warped::Event>> createInitialEvents();

    virtual std::vector<std::shared_ptr<warped::Event>> receiveEvent(const warped::Event& event);

    void populateTravelDistances(std::map<std::string, unsigned int> travel_chart) {

        diffusion_network_->populateTravelChart(travel_chart);
    }

protected:

    LocationState state_;
    std::string location_name_;
    std::unique_ptr<DiseaseModel> disease_model_;
    std::unique_ptr<DiffusionNetwork> diffusion_network_;
    unsigned int location_state_refresh_interval_;
    unsigned int location_diffusion_trigger_interval_;
};

#endif
