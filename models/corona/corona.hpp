#ifndef CORONA_HPP
#define CORONA_HPP

#include <string>
#include <vector>
#include <map>
#include <random>
#include "memory.hpp"
#include "warped.hpp"
#include "diffusion.hpp"

#define CONFIG CoronaConfig::getInstance()

class CoronaConfig {
public:
    static CoronaConfig* getInstance() {
        if (!instance_) {
            instance_ = new CoronaConfig();
        }
        return instance_;
    }

    double transmissibility_                = 2.2; /* equals beta     */
    double mean_incubation_duration_        = 5.2; /* equals 1/sigma  */
    double mean_infection_duration_         = 2.3; /* equals 1/gamma  */

    double mortality_rate_                  = 0.05;

    unsigned int update_trig_interval_      = 0;
    unsigned int diffusion_trig_interval_   = 0;

private:
    static CoronaConfig* instance_;
    CoronaConfig() = default;
}

/*
 *               beta           sigma              gamma
 *  SUSCEPTIBLE =====> EXPOSED ======> INFECTIOUS ======> RECOVERED
 */
enum infection_state_t {

    SUSCEPTIBLE = 0,
    EXPOSED,
    INFECTIOUS,
    RECOVERED,
    NUM_INFECTION_STATES
};

WARPED_DEFINE_LP_STATE_STRUCT(LocationState) {

    LocationState() = default;

    LocationState(const LocationState& other) {
        for (auto i = 0U; i < infection_state_t::NUM_INFECTION_STATES; i++) {
            population_[i] = other.population_[i];
        }
    };

    unsigned int population_[infection_state_t::NUM_INFECTION_STATES] = {0};
};

enum event_type_t {
    DIFFUSION,
    TRIGGER
};

class DiffusionEvent : public warped::Event {
public:
    DiffusionEvent() = delete;

    DiffusionEvent( const std::string receiver_name,
                    unsigned int timestamp,
                    infection_state_t infection_state)
            :   receiver_name_(receiver_name),
                arrival_ts_(timestamp),
                infection_state_(infection_state) {}

    const std::string& receiverName() const { return receiver_name_; }

    unsigned int timestamp() const { return arrival_ts_; }

    unsigned int size() const {
        return  receiver_name_.length() +
                sizeof(arrival_ts_) +
                sizeof(infection_state_);
    }

    event_type_t eventType() { return event_type_t::DIFFUSION; }

    std::string         receiver_name_;
    unsigned int        arrival_ts_;
    infection_state_t   infection_state_;

    WARPED_REGISTER_SERIALIZABLE_MEMBERS(cereal::base_class<warped::Event>(this), 
                receiver_name_, arrival_ts_, infection_state_)
};

class TriggerEvent : public warped::Event {
public:
    TriggerEvent() = delete;

    TriggerEvent(   const std::string receiver_name,
                    unsigned int timestamp,
                    bool is_diffusion = false)
            :   receiver_name_(receiver_name),
                target_ts_(timestamp),
                is_diffusion_(is_diffusion) {}

    const std::string& receiverName() const { return receiver_name_; }

    unsigned int timestamp() const { return target_ts_; }

    unsigned int size() const {
        return  receiver_name_.length() +
                sizeof(target_ts_) +
                sizeof(is_diffusion_);
    }

    event_type_t eventType() { return event_type_t::TRIGGER; }

    std::string         receiver_name_;
    unsigned int        target_ts_;
    bool                is_diffusion_;

    WARPED_REGISTER_SERIALIZABLE_MEMBERS(cereal::base_class<warped::Event>(this), 
                receiver_name_, target_ts_, is_diffusion_)
};

class Location : public warped::LogicalProcess {
public:
    Location() = delete;

    Location(   const std::string& name,
                unsigned int travel_time_to_hub,
                unsigned int max_infected_diffusion_cnt,
                unsigned int max_others_diffusion_cnt,
                unsigned int index  )
        :   LogicalProcess(name),
            state_(),
            location_name_(name),
            rng_(new std::default_random_engine(index)) {

        state_ = std::make_shared<LocationState>();
        diffusion_ = std::make_shared<Diffusion>(travel_time_to_hub,
                        max_infected_diffusion_cnt, max_others_diffusion_cnt, rng_);
    }

    virtual warped::LPState& getState() override { return *state_; }

    virtual std::vector<std::shared_ptr<warped::Event>> initializeLP() override;

    virtual std::vector<std::shared_ptr<warped::Event>> receiveEvent(
                                            const warped::Event& event) override;

    void populateTravelDistances(std::map<std::string, unsigned int> travel_chart) {
        diffusion_network_->populateTravelChart(travel_chart);
    }

    void reaction() {
        ptts();

        double prob_latent = 1.0 - CONFIG->latent_infectivity_;
        prob_latent = pow(prob_latent, state_->population_[infection_state_t::LATENT]);

        double prob_incubate = 1.0 - CONFIG->incubating_infectivity_;
        prob_incubate = pow(prob_incubate, state_->population_[infection_state_t::INCUBATING]);

        double prob_infect = 1.0 - CONFIG->infectious_infectivity_;
        prob_infect = pow(prob_infect, state_->population_[infection_state_t::INFECTIOUS]);

        double prob_asympt = 1.0 - CONFIG->asympt_infectivity_;
        prob_asympt = pow(prob_asympt, state_->population_[infection_state_t::ASYMPT]);

        double disease_prob = 1.0 - prob_latent * prob_incubate * prob_infect * prob_asympt;
        unsigned int affected =
            std::max(1, disease_prob * state_->population_[infection_state_t::UNINFECTED]);
        unsigned int new_latent = std::max(1, affected * CONFIG->transmissibility_);
        state_->population_[infection_state_t::UNINFECTED]  -= affected;
        state_->population_[infection_state_t::LATENT]      += new_latent;
        state_->population_[infection_state_t::INCUBATING]  += (affected - new_latent);
    }

    // Report population, infected cnt, recovered count and death count
    std::string printState() {
        auto population = 0U;
        for (auto i = 0; i < infection_state_t::NUM_INFECTION_STATES; i++) {
            population += state_->population_[i];
        }
        unsigned int deaths = state_->population_[RECOVERED] * CONFIG->mortality_rate_;
        return std::make_tuple(population, state_->population_[INFECTIOUS],
                                        state_->population_[RECOVERED] - deaths, deaths);
    }

    std::string getLocationName() { return location_name_; }

protected:
    std::shared_ptr<LocationState> state_;
    std::string location_name_;
    std::shared_ptr<Diffusion> diffusion_;
    std::shared_ptr<std::default_random_engine> rng_;

private:
    void ptts() {
        // LATENT ==> INFECTIOUS
        unsigned int new_infectious = CONFIG->latent_transition_rate_ *
                                state_->population_[infection_state_t::LATENT];
        state_->population_[infection_state_t::LATENT]      -= new_infectious;
        state_->population_[infection_state_t::INFECTIOUS]  += new_infectious;

        // INFECTIOUS ==> RECOVERED
        unsigned int new_recovered = CONFIG->infectious_transition_rate_ *
                                state_->population_[infection_state_t::INFECTIOUS];
        state_->population_[infection_state_t::INFECTIOUS]  -= new_recovered;
        state_->population_[infection_state_t::RECOVERED]   += new_recovered;

        // INFECTIOUS ==> DECEASED
        unsigned int new_deaths = CONFIG->mortality_rate_ *
                                state_->population_[infection_state_t::INFECTIOUS];
        state_->population_[infection_state_t::INFECTIOUS]  -= new_deaths;
        state_->population_[infection_state_t::DECEASED]    += new_deaths;

        // INCUBATING ==> ASYMPT
        unsigned int new_asympt =
            std::max(1, CONFIG->incubating_transition_rate_ *
                    state_->population_[infection_state_t::INCUBATING]);
        state_->population_[infection_state_t::INCUBATING]  -= new_asympt;
        state_->population_[infection_state_t::ASYMPT]      += new_asympt;

        // ASYMPT ==> UNINFECTED
        unsigned int new_uninfected =
            std::max(1, CONFIG->asympt_transition_ratio_ *
                    state_->population_[infection_state_t::UNINFECTED]);
        state_->population_[infection_state_t::ASYMPT]      -= new_uninfected;
        state_->population_[infection_state_t::UNINFECTED]  += new_uninfected;
    }
};

#endif
