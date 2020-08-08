#ifndef PANDEMIC_HPP
#define PANDEMIC_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <random>
#include <tuple>
#include "memory.hpp"
#include "warped.hpp"
#include "diffusion.hpp"

#include "jsoncons/json.hpp"
#include "jsoncons/json_cursor.hpp"

#define CONFIG              PandemicConfig::getInstance()
#define CONFIGFILEHANDLER   ConfigFileHandler::getInstance()

#define TIME_UNITS_IN_HOUR  1
#define TIME_UNITS_IN_DAY   (24 * TIME_UNITS_IN_HOUR)


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
    NUM_INFECTION_STATES // ??
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


class PandemicConfig {
public:
    static PandemicConfig* getInstance() {
        if (!instance_) {
            instance_ = new PandemicConfig();
        }
        return instance_;
    }

    // getconfig and printconfig

    double transmissibility_                = 2.2;          /* equals beta     */
    double mean_incubation_duration_        = 5.2 * TIME_UNITS_IN_DAY;    /* equals 1/sigma  */
    double mean_infection_duration_         = 2.3 * TIME_UNITS_IN_DAY;    /* equals 1/gamma  */

    double mortality_ratio_                 = 0.05;

    /* An arbitrary factor to account for discrepancy between actual and reported confirmed cases */
    double exposed_confirmed_ratio_         = 10.0;
    
    unsigned int update_trig_interval_      = 1 * TIME_UNITS_IN_DAY;
    unsigned int diffusion_trig_interval_   = 6 * TIME_UNITS_IN_HOUR;

    void addMapEntry(const std::string& str, std::tuple<std::string, std::string, std::string, float,
                     float> map_val) {
        map_name_location[str] = map_val;
    }

    const std::tuple<std::string, std::string, std::string, float, float>* getLocation(const std::string& str) {
        if (map_name_location.find(str) == map_name_location.end()) {
            return nullptr;
        }

        return &map_name_location[str];
    }


private:

    static PandemicConfig* instance_;
    PandemicConfig() = default;
    std::unordered_map<std::string, std::tuple<std::string, std::string, std::string, float,
                                               float>> map_name_location;
};


class Location : public warped::LogicalProcess { // add print
public:
    Location() = delete;

    Location(   const std::string& name,
                long int num_confirmed,
                long int num_deaths,
                long int num_recovered,
                long int num_active,
                long int population_cnt,
                unsigned int index  )
        :   LogicalProcess(name),
            state_(),
            location_name_(name),
            rng_(new std::default_random_engine(index)) {

        state_ = std::make_shared<LocationState>();

        state_->population_[infection_state_t::SUSCEPTIBLE] = population_cnt;
        // TODO multiply with factor??
        state_->population_[infection_state_t::EXPOSED] = CONFIG->exposed_confirmed_ratio_ * (double)num_confirmed;
        state_->population_[infection_state_t::INFECTIOUS] = num_confirmed; // TODO num_active; // infectious = num_confirmed  rid of num_active
        state_->population_[infection_state_t::RECOVERED] = num_recovered;
        state_->population_[infection_state_t::DECEASED] = num_deaths;

        // TODO error: ‘travel_time_to_hub’ was not declared in this scope
        diffusion_ = std::make_shared<Diffusion>(travel_time_to_hub,
                                                 max_infected_diffusion_cnt, max_others_diffusion_cnt, rng_);
    }

    virtual warped::LPState& getState() override { return *state_; }

    virtual std::vector<std::shared_ptr<warped::Event>> initializeLP() override;

    virtual std::vector<std::shared_ptr<warped::Event>> receiveEvent(
                                            const warped::Event& event) override;

    void populateTravelDistances(std::map<std::string, unsigned int> travel_chart) {
        // TODO error: ‘diffusion_network_’ was not declared in this scope
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
        return location_name_                                      + ","
            + std::to_string(state_->population_[infection_state_t::SUSCEPTIBLE]) + ","
            + std::to_string(state_->population_[infection_state_t::INFECTIOUS])  + ","
            + std::to_string(state_->population_[infection_state_t::RECOVERED])   + ","
            + std::to_string(state_->population_[infection_state_t::DECEASED])    + "\n";
    }

    std::string getLocationName() { return location_name_; }

protected:
    std::shared_ptr<LocationState> state_;
    std::string location_name_;
    std::shared_ptr<Diffusion> diffusion_;
    std::shared_ptr<std::default_random_engine> rng_;
};




class ConfigFileHandler {

public:
    static ConfigFileHandler* getInstance() {
        if (!instance_) {
            instance_ = new ConfigFileHandler();
        }
        return instance_;
    }

    void openFile(std::string fname, std::vector<Location> *lps) {
        m_lps = lps;
        m_input_fname = fname;

        cur = std::unique_ptr<jsoncons::json_cursor> (new jsoncons::json_cursor(
                      new std::ifstream(fname)));
    }

    void getValuesFromJsonFile(const std::string& fname) {
        std::ifstream is(fname);

        jsoncons::json input_data = jsoncons::json::parse(is);

        CONFIG->transmissibility_ = input_data["disease_model"]["transmissibility"].as<double>();
        CONFIG->mean_incubation_duration_ = input_data["disease_model"]["mean_incubation_duration_in_days"]
            .as<double>() * TIME_UNITS_IN_DAY;
        CONFIG->mean_infection_duration_ = input_data["disease_model"]["mean_infection_duration_in_days"]
            .as<double>() * TIME_UNITS_IN_DAY;
        CONFIG->mortality_ratio_ = input_data["disease_model"]["mortality_ratio"].as<double>();
        CONFIG->update_trig_interval_ = input_data["disease_model"]["update_trig_interval_in_hrs"]
            .as<unsigned int>() * TIME_UNITS_IN_HOUR;
        CONFIG->diffusion_trig_interval_ = input_data["disease_model"]["diffusion_trig_interval_in_hrs"]
            .as<unsigned int>() * TIME_UNITS_IN_HOUR;


        for (const auto& location_data : input_data["locations"].array_range()) {
            // read in values from json array for each individual "Location"
            std::string fips_code = location_data[0].as<std::string>();
            std::string county = location_data[1].as<std::string>();
            std::string state = location_data[2].as<std::string>();
            std::string country = location_data[3].as<std::string>();
            float loc_lat = location_data[4].as<float>();
            float loc_long = location_data[5].as<float>();
            long int num_confirmed = location_data[6].as<long int>();
            long int num_deaths = location_data[7].as<long int>();
            long int num_recovered = location_data[8].as<long int>();
            long int num_active = location_data[9].as<long int>();
            std::string combined_key = location_data[10].as<std::string>();
            long int population_cnt = location_data[11].as<long int>();

            // add values to Location LP's
            m_lps->emplace_back(Location(fips_code, num_confirmed, num_deaths, num_recovered, num_active,
                                         population_cnt, std::stoi(fips_code)));

            CONFIG->addMapEntry(fips_code, std::make_tuple(county, state, country, loc_lat, loc_long));
        }
    }


    void writeJsonToFile() {


        jsoncons::json jsontowrite;

        jsoncons::json diseasemodel_arr(jsoncons::json_array_arg, {CONFIG->transmissibility_,
                                                                   CONFIG->mean_incubation_duration_,
                                                                   CONFIG->mean_infection_duration_,
                                                                   CONFIG->mortality_ratio_,
                                                                   CONFIG->update_trig_interval_,
                                                                   CONFIG->diffusion_trig_interval_});

        jsontowrite["disease_model"] = std::move(diseasemodel_arr);

        // create json object for "Location" object
        jsoncons::json location_arr(jsoncons::json_array_arg);

        for (auto it = m_lps->begin(); it != m_lps->end(); ++it) {
            location_arr.push_back(jsoncons::json (jsoncons::json_array_arg, {
                                                       // TODO add Location elements
                                                       "a", "b", "c"
                                   }));
        }

        jsontowrite["locations"] = std::move(location_arr);

        jsoncons::json_printable<jsoncons::json> jsonprintable(jsoncons::pretty_print(jsontowrite));

        // TODO outfile name

        std::ofstream outfile;
        outfile.open(m_out_fname);
        outfile << jsonprintable;

        outfile.close();
    }

private:

    ConfigFileHandler() = default;

    static ConfigFileHandler* instance_;

    std::unique_ptr<jsoncons::json_cursor> cur;

    std::vector <Location> *m_lps;
    std::string m_input_fname;
    std::string m_out_fname;
};


#endif
