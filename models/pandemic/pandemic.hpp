#ifndef PANDEMIC_HPP
#define PANDEMIC_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <random>
#include <tuple>
#include <cmath>
#include <algorithm>
#include "memory.hpp"
#include "warped.hpp"
#include "graph.hpp"

#include "jsoncons/json.hpp"

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
    UPDATE_TIMER,
    DIFFUSION_TIMER
};

class PandemicEvent : public warped::Event {
public:
    PandemicEvent() = default;

    PandemicEvent(  const std::string target,
                    unsigned int ts,
                    event_type_t type,
                    infection_state_t state = infection_state_t::NUM_INFECTION_STATES ) :
        target_(target),
        ts_(ts),
        type_(type),
        state_(state) {}

    const std::string& receiverName() const { return target_; }

    unsigned int timestamp() const { return ts_; }

    unsigned int size() const {
        return  target_.length() + sizeof(ts_) + sizeof(type_) + sizeof(state_);
    }

    event_type_t eventType() { return type_; }

    std::string         target_;
    unsigned int        ts_;
    event_type_t        type_;
    infection_state_t   state_;

    WARPED_REGISTER_SERIALIZABLE_MEMBERS(cereal::base_class<warped::Event>(this), target_, ts_, type_, state_)
};

enum location_field_t {
    COUNTY_NAME,
    STATE,
    COUNTRY,
    LATITUDE,
    LONGITUDE,
    POPULATION_SIZE,
    NUM_FIELDS
};

class PandemicConfig {
public:
    static PandemicConfig* getInstance() {
        if (!instance_) {
            instance_ = new PandemicConfig();
        }
        return instance_;
    }

    double transmissibility_                = 2.2;          /* equals beta     */
    double exposed_confirmed_ratio_         = 2.5;
    double mean_incubation_duration_        = 5.2 * TIME_UNITS_IN_DAY;    /* equals 1/sigma  */
    double mean_infection_duration_         = 2.3 * TIME_UNITS_IN_DAY;    /* equals 1/gamma  */

    double mortality_ratio_                 = 0.05;

    /* NOTE: An arbitrary factor to account for discrepancy between actual
             and reported confirmed cases.
       Source: https://fortune.com/2020/06/25/us-coronavirus-cases-how-many-\
                total-20-million-asymptomatic-confirmed-tests-covid-19-case-count-cdc-estimate/

                double exposed_confirmed_ratio_         = 10.0; */
    
    unsigned int update_trig_interval_in_hrs_;
    unsigned int diffusion_trig_interval_in_hrs_;
    unsigned int avg_transport_speed_;
    unsigned int max_diffusion_cnt_;

    void addMapEntry(const std::string& str,
            std::tuple<std::string, std::string, std::string, double, double, unsigned long> val) {
        locations_[str] = val;
    }

    std::tuple<std::string, std::string, std::string,
               double, double, unsigned long> getLocation(const std::string& str) {
        auto it = locations_.find(str);
        assert(it != locations_.end());
        return it->second;
    }

    unsigned int haversine_distance(const std::string& sender, const std::string& receiver) {

        auto sender_it = locations_.find(sender);
        assert(sender_it != locations_.end());
        auto s_lati = std::get<location_field_t::LATITUDE> (sender_it->second);
        auto s_long = std::get<location_field_t::LONGITUDE>(sender_it->second);

        auto receiver_it = locations_.find(receiver);
        assert(receiver_it != locations_.end());
        auto r_lati = std::get<location_field_t::LATITUDE> (receiver_it->second);
        auto r_long = std::get<location_field_t::LONGITUDE>(receiver_it->second);

        const double earth_radius = 6372.8;
        const double pi_val = 3.14159265358979323846;

        double s_lati_rad = s_lati * pi_val / 180.0;
        double s_long_rad = s_long * pi_val / 180.0;
        double r_lati_rad = r_lati * pi_val / 180.0;
        double r_long_rad = r_long * pi_val / 180.0;

        double diff_lati = r_lati_rad - s_lati_rad;
        double diff_long = r_long_rad - s_long_rad;

        double val = std::asin(std::sqrt(
                std::sin(diff_lati / 2) * std::sin(diff_lati / 2) +
                std::cos(s_lati_rad) * std::cos(r_lati_rad) *
                std::sin(diff_long / 2) * std::sin(diff_long / 2)));

        return static_cast<unsigned int>(2.0 * earth_radius * val);
    }

private:
    static PandemicConfig* instance_;
    PandemicConfig() = default;

    std::unordered_map<std::string,
        std::tuple<std::string, std::string, std::string, double, double, unsigned long>> locations_;
};

class Location : public warped::LogicalProcess {
public:
    Location() = delete;

    /* NOTE : There is an ambiguity in naming convention in JHU data. For our model,
              JHU's active count is our confirmed count. Active count refers to the
              number of infected who are still alive and not recovered.
     */
    Location(   const std::string& name,
                // double transmissibility_,
                unsigned long num_confirmed,
                unsigned long num_deaths,
                unsigned long num_recovered,
                unsigned long population_size,
                unsigned int index  )
        :   LogicalProcess(name),
            state_(),
            location_name_(name),
            rng_(new std::default_random_engine(index)) {
        // dynamic_transmissibility_ = transmissibility_;
        state_ = std::make_shared<LocationState>();
        state_->population_[infection_state_t::EXPOSED] =
            std::min<unsigned long>(CONFIG->exposed_confirmed_ratio_ * (double)num_confirmed, population_size/2);

        state_->population_[infection_state_t::INFECTIOUS]  = num_confirmed;
        state_->population_[infection_state_t::RECOVERED]   = num_recovered;
        state_->population_[infection_state_t::DECEASED]    = num_deaths;
        state_->population_[infection_state_t::SUSCEPTIBLE] = population_size
                                - state_->population_[infection_state_t::EXPOSED]
                                - state_->population_[infection_state_t::INFECTIOUS]
                                - state_->population_[infection_state_t::RECOVERED]
                                - state_->population_[infection_state_t::DECEASED];
    }

    virtual warped::LPState& getState() override { return *state_; }

    virtual std::vector<std::shared_ptr<warped::Event>> initializeLP() override;

    virtual std::vector<std::shared_ptr<warped::Event>> receiveEvent(
                                            const warped::Event& event) override;

    std::string diffusionTarget() {
        unsigned int num_locations = adjacent_nodes_.size();
        assert(num_locations);
        std::uniform_int_distribution<int> distribution(0, num_locations-1);
        auto location_id = (unsigned int) distribution(*rng_);
        return adjacent_nodes_[location_id];
    }

    unsigned int travelTimeToTarget(std::string target_loc) {
        return CONFIG->haversine_distance(location_name_, target_loc) / CONFIG->avg_transport_speed_;
    }

    unsigned int diffusionCount() {
        std::uniform_int_distribution<int> distribution(0, CONFIG->max_diffusion_cnt_);
        return distribution(*rng_);
    }

    void reaction(unsigned int timestamp) {

        timestamp += 0;

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
        // double exponent = (double) -1 * (48 * timestamp);
        double exponent = (double) -1 * (timestamp / 70);

        double dynamic_transmissibility_ = CONFIG->transmissibility_ * pow(2.71828, exponent);

        // unsigned int delta_S = (int)( dynamic_transmissibility_ *
        unsigned int delta_S = ( dynamic_transmissibility_ *
                            state_->population_[infection_state_t::SUSCEPTIBLE] *
                            state_->population_[infection_state_t::INFECTIOUS] ) / N;

        // std::cout << "dynamic_transmissibility_[" << pow(2.71828, exponent) << "]\n"
        //           << "delta_S " << delta_S << std::endl;

        state_->population_[infection_state_t::SUSCEPTIBLE] -= delta_S;
        state_->population_[infection_state_t::EXPOSED]     += delta_S;


        unsigned int delta_E = state_->population_[infection_state_t::EXPOSED] /
                                                    CONFIG->mean_incubation_duration_;


        // unsigned int delta_E = state_->population_[infection_state_t::EXPOSED] / pow(2.71828,
        //                                                                             CONFIG->mean_incubation_duration_ * 24 / timestamp);

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
        return location_name_                                                     + ","
            + std::to_string(state_->population_[infection_state_t::SUSCEPTIBLE]) + ","
            + std::to_string(state_->population_[infection_state_t::INFECTIOUS])  + ","
            + std::to_string(state_->population_[infection_state_t::RECOVERED])   + ","
            + std::to_string(state_->population_[infection_state_t::DECEASED])    + "\n";
    }

    std::string getLocationName() { return location_name_; }

    std::shared_ptr<LocationState> getLocationState() { return state_; }

    std::vector<std::string> adjacent_nodes_;

protected:
    std::shared_ptr<LocationState> state_;
    std::string location_name_;
    std::shared_ptr<std::default_random_engine> rng_;
    // double dynamic_transmissibility_;
};

class ConfigFileHandler {

public:
    static ConfigFileHandler* getInstance() {
        if (!instance_) {
            instance_ = new ConfigFileHandler();
        }
        return instance_;
    }

    void readConfig(const std::string& fname, std::vector<Location>& lps) {

        std::ifstream is(fname);
        jsoncons::ojson data = jsoncons::ojson::parse(is);

        CONFIG->transmissibility_ = data["disease_model"]["transmissibility"].as<double>();

        CONFIG->exposed_confirmed_ratio_ = data["disease_model"]["exposed_confirmed_ratio"].as<double>();

        CONFIG->mean_incubation_duration_ =
            data["disease_model"]["mean_incubation_duration_in_days"].as<double>() * TIME_UNITS_IN_DAY;

        CONFIG->mean_infection_duration_ =
            data["disease_model"]["mean_infection_duration_in_days"].as<double>() * TIME_UNITS_IN_DAY;

        CONFIG->mortality_ratio_ = data["disease_model"]["mortality_ratio"].as<double>();

        CONFIG->update_trig_interval_in_hrs_ =
            data["disease_model"]["update_trig_interval_in_hrs"].as<unsigned int>() * TIME_UNITS_IN_HOUR;

        CONFIG->diffusion_trig_interval_in_hrs_ =
            data["diffusion_model"]["diffusion_trig_interval_in_hrs"].as<unsigned int>() * TIME_UNITS_IN_HOUR;

        CONFIG->avg_transport_speed_ =
            data["diffusion_model"]["avg_transport_speed"].as<unsigned int>();

        CONFIG->max_diffusion_cnt_ =
            data["diffusion_model"]["max_diffusion_cnt"].as<unsigned int>();

        // Read values from json array for each location.
        // NOTE : Number of confirmed for this model is the same as number of active in JHU data
        unsigned int index = 0;
        for (const auto& location : data["locations"].array_range()) {

            std::string fips_code    = location[loc_data_field_t::FIPS_CODE].as<std::string>();
            std::string county       = location[loc_data_field_t::COUNTY_NAME].as<std::string>();
            std::string state        = location[loc_data_field_t::STATE].as<std::string>();
            std::string country      = location[loc_data_field_t::COUNTRY].as<std::string>();
            double latitude          = location[loc_data_field_t::LATITUDE].as<double>();
            double longitude         = location[loc_data_field_t::LONGITUDE].as<double>();
            unsigned long deaths     = location[loc_data_field_t::NUM_DEATHS].as<unsigned long>();
            unsigned long recovered  = location[loc_data_field_t::NUM_RECOVERED].as<unsigned long>();
            unsigned long confirmed  = location[loc_data_field_t::NUM_ACTIVE].as<unsigned long>();  // ratio of actual vs. calculate actual active values for july 1st
            unsigned long population = location[loc_data_field_t::POPULATION_SIZE].as<unsigned long>();

            lps.emplace_back(Location(fips_code, confirmed, deaths, recovered, population, index++));
            CONFIG->addMapEntry(fips_code, std::make_tuple(county, state, country, latitude, longitude, population));
        }

        /* Create the Network Graph */
        std::vector<std::string> nodes;
        for (auto& lp : lps) {
            nodes.push_back(lp.getLocationName());
        }

        Graph *graph = nullptr;

        graph_type_     = data["diffusion_model"]["graph_type"].as<std::string>();
        graph_params_   = data["diffusion_model"]["graph_params"].as<std::string>();

        /* If the choice is Watts-Strogatz */
        if (graph_type_ == "Watts-Strogatz") {
            graph = new WattsStrogatz(nodes, graph_params_);

        } else if (graph_type_ == "Barabasi-Albert") {
            /* If the choice is Barabasi-Albert */
            graph = new BarabasiAlbert(nodes, graph_params_);

        } else { // Invalid choice
            std::cerr << "Invalid choice of diffusion network." << std::endl;
            assert(0);
        }

        for (auto& lp : lps) {
            lp.adjacent_nodes_ = graph->adjacencyList(lp.getLocationName());
        }

        delete graph;
    }

    void writeConfig(const std::string& out_fname, const std::vector<Location>& lps) {

        jsoncons::ojson jsontowrite;
        jsoncons::ojson disease_model(jsoncons::json_object_arg, {
                {"transmissibility", CONFIG->transmissibility_}, // TODO
                {"mean_incubation_duration_in_days", CONFIG->mean_incubation_duration_ / TIME_UNITS_IN_DAY},
                {"mean_infection_duration_in_days", CONFIG->mean_infection_duration_ / TIME_UNITS_IN_DAY},
                {"mortality_ratio", CONFIG->mortality_ratio_},
                {"update_trig_interval_in_hrs", CONFIG->update_trig_interval_in_hrs_}
            });
        jsontowrite["disease_model"] = std::move(disease_model);

        jsoncons::ojson diffusion_model(jsoncons::json_object_arg, {
                {"graph_type", graph_type_},
                {"graph_params", graph_params_},
                {"diffusion_trig_interval_in_hrs", CONFIG->diffusion_trig_interval_in_hrs_}
            });
        jsontowrite["diffusion_model"] = std::move(diffusion_model);

        jsontowrite["locations"] = jsoncons::ojson(jsoncons::json_array_arg, {});
        for (auto lp : lps) {
            std::string fips_code    = lp.getLocationName();

            // get tuple
            auto details             = CONFIG->getLocation(fips_code);

            std::string county       = std::get<location_field_t::COUNTY_NAME>(details);
            std::string state        = std::get<location_field_t::STATE>(details);
            std::string country      = std::get<location_field_t::COUNTRY>(details);
            double latitude          = std::get<location_field_t::LATITUDE>(details);
            double longitude         = std::get<location_field_t::LONGITUDE>(details);
            unsigned long population = std::get<location_field_t::POPULATION_SIZE>(details);
            unsigned long active     = lp.getLocationState()->population_[infection_state_t::INFECTIOUS];
            unsigned long recovered  = lp.getLocationState()->population_[infection_state_t::RECOVERED];
            unsigned long deaths     = lp.getLocationState()->population_[infection_state_t::DECEASED];

            /* NOTE : Active count for JHU data is confirmed count in this model */
            auto confirmed = active + deaths + recovered;
            assert(confirmed <= population);

            jsontowrite["locations"].push_back(jsoncons::ojson(jsoncons::json_array_arg, {
                        fips_code, county, state, country, latitude, longitude,
                        confirmed, deaths, recovered, active, population}));
        }

        std::ofstream outfile;
        outfile.open(out_fname);

        jsoncons::json_options options;
        options.indent_size(2);

        outfile << jsoncons::pretty_print(jsontowrite, options);

        outfile.close();
    }

private:

    ConfigFileHandler() = default;
    static ConfigFileHandler* instance_;

    std::string graph_type_;
    std::string graph_params_;

    enum loc_data_field_t {
        FIPS_CODE = 0,
        COUNTY_NAME,
        STATE,
        COUNTRY,
        LATITUDE,
        LONGITUDE,
        NUM_CONFIRMED,
        NUM_DEATHS,
        NUM_RECOVERED,
        NUM_ACTIVE,
        POPULATION_SIZE,
        NUM_FIELDS
    };
};

#endif
