#ifndef PANDEMIC_HPP
#define PANDEMIC_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <random>
#include <tuple>
#include "memory.hpp"
#include "warped.hpp"
#include "graph.hpp"

#include "jsoncons/json.hpp"
#include "jsoncons/json_cursor.hpp"

#define CONFIG              PandemicConfig::getInstance()
#define CONFIGFILEHANDLER   ConfigFileHandler::getInstance()

#define TIME_UNITS_IN_HOUR  1
#define TIME_UNITS_IN_DAY   (24 * TIME_UNITS_IN_HOUR)

#define AVG_TRANSPORT_SPEED 10

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

    double transmissibility_                = 2.2;          /* equals beta     */
    double mean_incubation_duration_        = 5.2 * TIME_UNITS_IN_DAY;    /* equals 1/sigma  */
    double mean_infection_duration_         = 2.3 * TIME_UNITS_IN_DAY;    /* equals 1/gamma  */

    double mortality_ratio_                 = 0.05;

    /* NOTE: An arbitrary factor to account for discrepancy between actual
             and reported confirmed cases.
       Source: https://fortune.com/2020/06/25/us-coronavirus-cases-how-many-\
               total-20-million-asymptomatic-confirmed-tests-covid-19-case-count-cdc-estimate/
     */
    double exposed_confirmed_ratio_         = 10.0;

    unsigned int update_trig_interval_in_hrs      = 1 * TIME_UNITS_IN_DAY;
    unsigned int diffusion_trig_interval_in_hrs   = 6 * TIME_UNITS_IN_HOUR;

    void addMapEntry(const std::string& str,
            std::tuple<std::string, std::string, std::string, float, float, long int> map_val) {
        locations_[str] = map_val;
    }

    std::tuple<std::string, std::string, std::string, float, float, long int>*
                                                            getLocation(const std::string& str) {
        auto it = locations_.find(str);
        assert(it != locations_.end());
        return &(it->second);
    }

    std::unordered_map<std::string,
        std::tuple<std::string, std::string, std::string, float, float, long int>> locations_;

private:
    static PandemicConfig* instance_;
    PandemicConfig() = default;
};

class Location : public warped::LogicalProcess {
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
        state_->population_[infection_state_t::EXPOSED] =
                    CONFIG->exposed_confirmed_ratio_ * (double)num_confirmed;
        state_->population_[infection_state_t::INFECTIOUS]  = num_confirmed;
        state_->population_[infection_state_t::RECOVERED]   = num_recovered;
        state_->population_[infection_state_t::DECEASED]    = num_deaths;
    }

    virtual warped::LPState& getState() override { return *state_; }

    virtual std::vector<std::shared_ptr<warped::Event>> initializeLP() override;

    virtual std::vector<std::shared_ptr<warped::Event>> receiveEvent(
                                            const warped::Event& event) override;

    std::string pickLocation() {
        unsigned int num_locations = adjacent_nodes_.size();
        assert(num_locations);
        std::uniform_int_distribution<int> distribution(0, num_locations-1);
        auto location_id = (unsigned int) distribution(*rng_);
        return adjacent_nodes_[location_id];
    }

    unsigned int travelTimeToLocation(std::string target_loc) {

        auto curr_it = CONFIG->locations_.find(location_name_);
        assert(curr_it != CONFIG->locations_.end());

        auto target_it = CONFIG->locations_.find(target_loc);
        assert(target_it != CONFIG->locations_.end());

        double distance = calc_haversine_distance(  std::get<3>(target_it->second),
                                                    std::get<4>(target_it->second),
                                                    std::get<3>(curr_it->second),
                                                    std::get<4>(curr_it->second)  );
        return (int)(distance/AVG_TRANSPORT_SPEED);
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

    std::shared_ptr<LocationState> getLocationState() { return state_; }
    
protected:
    std::shared_ptr<LocationState> state_;
    std::string location_name_;
    std::shared_ptr<std::default_random_engine> rng_;
    std::vector<std::string> adjacent_nodes_;

private:
    double calc_haversine_distance(double lat1, double long1, double lat2, double long2) {

        const double earth_radius = 6372.8;
        const double pi_val = 3.14159265358979323846;

        double lat1_rad = lat1 * pi_val / 180.0;
        double long1_rad = long1 * pi_val / 180.0;
        double lat2_rad = lat2 * pi_val / 180.0;
        double long2_rad = long2 * pi_val / 180.0;

        double diff_lat = lat2_rad - lat1_rad;
        double diff_long = long2_rad - long1_rad;

        double temp_val = std::asin(std::sqrt(
                std::sin(diff_lat / 2) * std::sin(diff_lat / 2) +
                std::cos(lat1_rad) * std::cos(lat2_rad) *
                std::sin(diff_long / 2) * std::sin(diff_long / 2)));

        return 2.0 * earth_radius * temp_val;
    }

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
        m_lps_ = lps;
        m_input_fname_ = fname;

        cur_ = std::unique_ptr<jsoncons::json_cursor> (new jsoncons::json_cursor(
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
        CONFIG->update_trig_interval_in_hrs = input_data["disease_model"]["update_trig_interval_in_hrs"]
            .as<unsigned int>() * TIME_UNITS_IN_HOUR;
        CONFIG->diffusion_trig_interval_in_hrs = input_data["disease_model"]["diffusion_trig_interval_in_hrs"]
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
            long int total_population_cnt = location_data[10].as<long int>();
            long int num_susceptible = total_population_cnt -
                            num_confirmed - num_deaths - num_recovered - num_active;

            m_lps_->emplace_back(Location(fips_code, num_confirmed, num_deaths, num_recovered, num_active,
                                         num_susceptible, std::stoi(fips_code)));

            CONFIG->addMapEntry(fips_code, std::make_tuple(county, state, country, loc_lat, loc_long,
                                                           total_population_cnt));
        }

        // Create the Network Graph
        std::vector<std::string> nodes;
        for (auto& lp : *m_lps_) {
            nodes.push_back(lp.getLocationName());
        }

        Graph *graph = nullptr;
        /* If the choice is Watts-Strogatz */
        if (input_data["diffusion_model"]["graph_type"].as<std::string>() == "Watts-Strogatz") {
            std::string params = input_data["diffusion_model"]["graph_param_str"].as<std::string>();
            graph = new WattsStrogatz(nodes, params);

        } else if (input_data["diffusion_model"]["graph_type"].as<std::string>() == "Barabasi-Albert") {
            /* If the choice is Barabasi-Albert */
            std::string params = input_data["diffusion_model"]["graph_param_str"].as<std::string>();
            graph = new BarabasiAlbert(nodes, params);

        } else { // Invalid choice
            std::cerr << "Invalid choice of diffusion network." << std::endl;
            assert(0);
        }
        for (auto& lp : *m_lps_) {
            lp.adjacent_nodes_ = graph->adjacencyList(lp.getLocationName());
        }
        delete graph;
    }

    void writeSimulationOutputToJsonFile(const std::string& out_fname) {
        // TODO vivek: add diffusion_model
        jsoncons::json jsontowrite;

        jsoncons::json disease_model(jsoncons::json_object_arg, {
                {"date_date", jsoncons::json::null},
                {"diffusion_trig_interval_in_hrs", CONFIG->diffusion_trig_interval_in_hrs},
                {"mean_incubation_duration_in_days", CONFIG->mean_incubation_duration_ / TIME_UNITS_IN_DAY},
                {"mean_infection_duration_in_days", CONFIG->mean_infection_duration_ / TIME_UNITS_IN_DAY},
                {"mortality_ratio", CONFIG->mortality_ratio_},
                {"transmissibility", CONFIG->transmissibility_},
                {"update_trig_interval_in_hrs", CONFIG->update_trig_interval_in_hrs}
            });

        jsontowrite["disease_model"] = std::move(disease_model);
        jsontowrite["locations"] = jsoncons::json(jsoncons::json_array_arg, {});

        for (auto it = m_lps_->begin(); it != m_lps_->end(); ++it) {
            std::string fips_code = it->getLocationName();

            std::tuple<std::string, std::string, std::string, float, float, long int>*
                p_map_location_val = CONFIG->getLocation(fips_code);

            std::string county = std::get<0>(*p_map_location_val);
            std::string state = std::get<1>(*p_map_location_val);
            std::string country_region = std::get<2>(*p_map_location_val);
            float loc_lat = std::get<3>(*p_map_location_val);
            float loc_long = std::get<4>(*p_map_location_val);
            long int total_population_cnt = std::get<5>(*p_map_location_val);

            long int num_susceptible = it->getLocationState()->population_[infection_state_t::SUSCEPTIBLE];
            long int num_exposed = it->getLocationState()->population_[infection_state_t::EXPOSED];
            long int num_confirmed = it->getLocationState()->population_[infection_state_t::INFECTIOUS];
            long int num_recovered = it->getLocationState()->population_[infection_state_t::RECOVERED];
            long int num_deaths = it->getLocationState()->population_[infection_state_t::DECEASED];

            jsontowrite["locations"].push_back(jsoncons::json(jsoncons::json_array_arg, {
                        fips_code, county, state, country_region, loc_lat, loc_long, num_confirmed, num_deaths,
                        num_recovered, num_confirmed, total_population_cnt}));
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

    std::unique_ptr<jsoncons::json_cursor> cur_;

    std::vector <Location> *m_lps_;
    std::string m_input_fname_;
};


#endif
