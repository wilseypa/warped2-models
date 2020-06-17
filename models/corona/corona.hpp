#ifndef CORONA_HPP
#define CORONA_HPP

#include <string>
#include <vector>
#include <map>
#include <random>
#include "memory.hpp"
#include "warped.hpp"
#include "diffusion.hpp"

#define CONFIG  CoronaConfig::getInstance()
#define HOUR    1
#define DAY     24

class CoronaConfig {
public:
    static CoronaConfig* getInstance() {
        if (!instance_) {
            instance_ = new CoronaConfig();
        }
        return instance_;
    }

    double transmissibility_                = 2.2;          /* equals beta     */
    double mean_incubation_duration_        = 5.2 * DAY;    /* equals 1/sigma  */
    double mean_infection_duration_         = 2.3 * DAY;    /* equals 1/gamma  */

    double mortality_ratio_                 = 0.05;

    unsigned int update_trig_interval_      = 1 * DAY;
    unsigned int diffusion_trig_interval_   = 6 * HOUR;

private:
    static CoronaConfig* instance_;
    CoronaConfig() = default;
};

/*
 *  beta  <= transmissibility
 *  sigma <= 1 / mean_incubation_period
 *  gamma <= 1 / mean_infection_period
 *
 *              beta         sigma            gamma           mortality
 *  SUSCEPTIBLE ===> EXPOSED ====> INFECTIOUS ====> RECOVERED ========> DECEASED
 */
enum infection_state_t {

    SUSCEPTIBLE = 0,
    EXPOSED,
    INFECTIOUS,
    RECOVERED,
    DECEASED,
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
        auto N = 0U;
        for (auto i = 0; i < infection_state_t::DECEASED; i++) {
            N += state_->population_[i];
        }

        /*
         *  S' = -(beta * S * I) / N
         *  E' = (beta * S * I) / N - sigma * E
         *  I' = sigma * E - gamma * I
         *  R' = gamma * I - mortality_ratio * R
         *  D' = mortality_ratio * R
         */
        unsigned int delta_S = ( CONFIG->transmissibility_ *
                            state_->population_[infection_state_t::SUSCEPTIBLE] *
                            state_->population_[infection_state_t::INFECTIOUS] ) / N;

        state_->population_[infection_state_t::SUSCEPTIBLE] -= delta_S;
        state_->population_[infection_state_t::EXPOSED]     += delta_S;


        unsigned int delta_E = state_->population_[infection_state_t::EXPOSED] /
                                                    CONFIG->mean_incubation_duration_;

        state_->population_[infection_state_t::EXPOSED]     -= delta_E;
        state_->population_[infection_state_t::INFECTIOUS]  += delta_E;

        unsigned int delta_I = state_->population_[infection_state_t::INFECTIOUS] /
                                                    CONFIG->mean_infection_duration_;

        state_->population_[infection_state_t::INFECTIOUS]  -= delta_I;
        state_->population_[infection_state_t::RECOVERED]   += delta_I;

        unsigned int delta_D = CONFIG->mortality_ratio_ * delta_I;

        state_->population_[infection_state_t::RECOVERED]   -= delta_D;
        state_->population_[infection_state_t::DECEASED]    += delta_D;
    }

    // Report population, infected cnt, recovered count and death count
    std::string printState() {
        return  location_name_                                      + ","
                state_->population_[infection_state_t::SUSCEPTIBLE] + ","
                state_->population_[infection_state_t::INFECTIOUS]  + ","
                state_->population_[infection_state_t::RECOVERED]   + ","
                state_->population_[infection_state_t::DECEASED]    + "\n";
    }

    std::string getLocationName() { return location_name_; }

protected:
    std::shared_ptr<LocationState> state_;
    std::string location_name_;
    std::shared_ptr<Diffusion> diffusion_;
    std::shared_ptr<std::default_random_engine> rng_;
};

#endif
