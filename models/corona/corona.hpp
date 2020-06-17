#ifndef CORONA_HPP
#define CORONA_HPP

#include <string>
#include <vector>
#include <map>
#include <random>
#include "memory.hpp"
#include "warped.hpp"
#include "diffusion.hpp"
#include "libbz2support.hpp"

#include "jsoncons/json.hpp"
#include "jsoncons/json_cursor.hpp"

#define CONFIG  CoronaConfig::getInstance()
#define CONFIGFILEHANDLER   ConfigFileHandler::getInstance()

#define HOUR    1
#define DAY     24



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

class Location : public warped::LogicalProcess { // add print
public:
    Location() = delete;

    // TODO add new constructor??
    /*
    Location (std::string fips, std::string county, std::string state, std::string country,
              std::string loc_lat,
              std::string loc_lang,
              unsigned long int population,
              unsigned long int confirmed,
              unsigned long int recovered,
              unsigned long int deaths) : LogicalProcess(fips) {
        ;

        fips_code
        countyname
        statename
        // add country here??
        location_lat
        location_long
        population : -1 for unknown value
        confirmed
        recovered
        deaths


    }*/

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

    // TODO add method to get location??


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




class CoronaConfig {
public:
    static CoronaConfig* getInstance() {
        if (!instance_) {
            instance_ = new CoronaConfig();
        }
        return instance_;
    }

    // getconfig and printconfig

    double transmissibility_                = 2.2;          /* equals beta     */
    double mean_incubation_duration_        = 5.2 * DAY;    /* equals 1/sigma  */
    double mean_infection_duration_         = 2.3 * DAY;    /* equals 1/gamma  */

    double mortality_ratio_                 = 0.05;

    unsigned int update_trig_interval_      = 1 * DAY;
    unsigned int diffusion_trig_interval_   = 6 * HOUR;

    // add method to store location ref map ??
    void addMapEntry(const std::string& str, Location* loc) {
        map_name_location[str] = loc;
    }

    Location* getLocation(const std::string& str) {
        if (map_name_location.find(str) == map_name_location.end()) {
            return nullptr;
        }

        return map_name_location[str];
    }


private:

    static CoronaConfig* instance_;
    CoronaConfig() = default;
    std::map<std::string, Location*> map_name_location;
};



class ConfigFileHandler {

public:
    static ConfigFileHandler* getInstance() {
        if (!instance_) {
            instance_ = new ConfigFileHandler();
        }
        return instance_;
    }

    void openbz2File(std::string fname, std::vector<Location> *lps)  {
        m_lps = lps;
        m_fname = fname;

        std::unique_ptr<std::istream> ifs(BZ2FILE->readbz2file(fname));

        cur = std::unique_ptr<jsoncons::json_cursor> (
                    new jsoncons::json_cursor(*ifs));
    }

    void getValuesFromJsonStream() {

        bool flag_disease_model_key_seen = false;
        bool inOuterArray = false;

        jsoncons::json_decoder<jsoncons::json> decoder;

        for (; !cur->done(); cur->next()) {

            const auto& event = cur->current();

            switch (event.event_type()) {

            case jsoncons::staj_event_type::begin_array:

                if (inOuterArray == false) {
                    inOuterArray = true;
                } else {
                    cur->read_to(decoder);

                    jsoncons::json j = decoder.get_result();

                    // read in values from json array for each individual "Location"
                    std::string fips_code = j[0].as<std::string>();
                    std::string county = j[1].as<std::string>();
                    std::string state = j[2].as<std::string>();
                    std::string country = j[3].as<std::string>();
                    std::string loc_lat = j[4].as<std::string>();
                    std::string loc_lang = j[5].as<std::string>();
                    unsigned long int population = j[6].as<unsigned long int>();
                    unsigned long int infected = j[7].as<unsigned long int>();
                    unsigned long int recovered = j[8].as<unsigned long int>();
                    unsigned long int deaths = j[9].as<unsigned long int>();

                    // add values to Location LP's
                    // m_lps->emplace_back(Location(fips_code, county, state, country, loc_lat, loc_lang,
                                        //        population, infected, recovered, deaths));

                    CONFIG->addMapEntry(fips_code + "_" + county + "_" + state + "_"
                                        + country, &m_lps->back());
                    // CONFIG->map_name_location[std::string()] = &m_lps.back();
                }

                break;

            case jsoncons::staj_event_type::end_array:
                break;

            case jsoncons::staj_event_type::begin_object:

                if (flag_disease_model_key_seen == true) {
                    flag_disease_model_key_seen = false;

                    cur->read_to(decoder);

                    jsoncons::json j = decoder.get_result();

                    // read in disease model
                    CONFIG->transmissibility_ = j["transmissibility"].as<double>();
                    CONFIG->mean_incubation_duration_ = j["mean_incubation_duration_in_days"].as<double>();
                    CONFIG->mean_infection_duration_ = j["mean_infection_duration_in_days"].as<double>();
                    CONFIG->mortality_ratio_ = j["mortality_ratio"].as<double>();
                    CONFIG->update_trig_interval_ = j["update_trig_interval_in_hrs"].as<unsigned int>();
                    CONFIG->diffusion_trig_interval_ = j["diffusion_trig_interval_in_hrs"].as<unsigned int>();
                }
                break;

            case jsoncons::staj_event_type::end_object:
                break;

            case jsoncons::staj_event_type::key:

                if (event.get<jsoncons::string_view>() == "disease_model") {
                    flag_disease_model_key_seen = true;
                }
                break;

            case jsoncons::staj_event_type::string_value:
                break;
            case jsoncons::staj_event_type::null_value:
                break;
            case jsoncons::staj_event_type::bool_value:
                break;
            case jsoncons::staj_event_type::int64_value:
                break;
            case jsoncons::staj_event_type::uint64_value:
                break;
            case jsoncons::staj_event_type::double_value:
                break;
            default:
                break;
            }
        }

    }


    void writeJsonToFile() {


        jsoncons::json jsontowrite;

        jsoncons::json diseasemodel_arr(jsoncons::json_array_arg, {CONFIG->transmissibility_, CONFIG->mean_incubation_duration_,
                                        CONFIG->mean_infection_duration_, CONFIG->mortality_ratio_, CONFIG->update_trig_interval_,
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

        std::stringstream sst;

        jsonprintable.dump(sst);

        //send to print function
        BZ2FILE->writebz2file(m_fname, sst.str());

    }

private:

    ConfigFileHandler() = default;

    static ConfigFileHandler* instance_;

    std::unique_ptr<jsoncons::json_cursor> cur;

    std::vector <Location> *m_lps;
    std::string m_fname;


};

#endif
