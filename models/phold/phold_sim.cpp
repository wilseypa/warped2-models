// An implementation of the synthetic PHOLD simulation model
// See "Performance of Time Warp Under Synthetic Workloads" by R.M. Fujimoto

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <memory>

#include "warped.hpp"
#include "tclap/ValueArg.h"

std::random_device rd;

enum distribution_t {UNIFORM, POISSON, EXPONENTIAL, NORMAL, BINOMIAL, FIXED,
                     ALTERNATE, ROUNDROBIN, CONDITIONAL, ALL};

WARPED_DEFINE_LP_STATE_STRUCT(PholdState) {
    unsigned int messages_sent_;
    unsigned int messages_received_;
};

class PholdEvent : public warped::Event {
public:
    PholdEvent() = default;
    PholdEvent(const std::string& receiver_name, const unsigned int timestamp)
                    : receiver_name_(receiver_name), time_stamp_(timestamp) {}

    const std::string& receiverName() const { return receiver_name_; }
    unsigned int timestamp() const { return time_stamp_; }

    std::string receiver_name_;
    unsigned int time_stamp_;

    WARPED_REGISTER_SERIALIZABLE_MEMBERS(cereal::base_class<warped::Event>(this), 
                                                        receiver_name_, time_stamp_)
};

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(PholdEvent)

class PholdLP : public warped::LogicalProcess {
public:
    PholdLP(const std::string& name, unsigned int initial_events,
                unsigned int num_lps, distribution_t distribution,
                double distribution_mean = 1.0)
        : LogicalProcess(name), state_(), initial_events_(initial_events),
            num_lps_(num_lps), rng_(new std::default_random_engine(rd())),
            distribution_(distribution), distribution_mean_(distribution_mean) {}

    warped::LPState& getState() { return this->state_; }

    std::vector<std::shared_ptr<warped::Event> > initializeLP() override {

        this->registerRNG(this->rng_);

        std::vector<std::shared_ptr<warped::Event> > events;
        for (unsigned int i = 0; i < this->initial_events_; i++) {
            ++this->state_.messages_sent_;
            events.emplace_back(new PholdEvent { this->get_destination(), 
                                            this->get_timestamp_delay() });
        }
        return events;
    }

    std::vector<std::shared_ptr<warped::Event>> receiveEvent(const warped::Event& event) {
        ++this->state_.messages_received_;
        std::vector<std::shared_ptr<warped::Event> > response_events;
        auto received_event = static_cast<const PholdEvent&>(event);
        response_events.emplace_back(new PholdEvent { this->get_destination(),
                                    event.timestamp() + this->get_timestamp_delay() });
        ++this->state_.messages_sent_;
        return response_events;
    }

    PholdState state_;

protected:
    const unsigned int initial_events_;
    const unsigned int num_lps_;
    std::shared_ptr<std::default_random_engine> rng_;
    const distribution_t distribution_;
    const double distribution_mean_;

    std::string get_destination() const {
        std::uniform_int_distribution<int> dest(0, (int)(num_lps_-1));
        unsigned int destination_number = (unsigned int) dest(*this->rng_);
        return std::string("LP ") + std::to_string(destination_number);
    }

    unsigned int get_timestamp_delay() const {
        double delay;
        switch ( this->distribution_ ) {
            case UNIFORM : {
                std::uniform_int_distribution<int> uniform(0.0, 2*this->distribution_mean_);
                delay = uniform(*this->rng_);
            } break;

            case NORMAL : {
                std::normal_distribution<double> normal(this->distribution_mean_, 1.0);
                delay = (unsigned int) normal(*this->rng_);
            } break;

            case BINOMIAL : {
                std::binomial_distribution<int> binomial((int)this->distribution_mean_, 0.0);
                delay = binomial(*this->rng_);
            } break;

            case POISSON : {
                std::poisson_distribution<int> poisson((int)this->distribution_mean_);
                delay = poisson(*this->rng_);
            } break;

            case EXPONENTIAL : {
                std::exponential_distribution<double> expo(1.0/this->distribution_mean_);
                delay = (unsigned int) expo(*this->rng_);
            } break;

            case FIXED : {
                delay = (unsigned int) this->distribution_mean_ ;
            } break;

            default : {
                delay = 0;
                std::cerr << "Improper Distribution for a Source LP!!!" << std::endl;
            }
        }
        return ( (unsigned int) delay );
    }
};

int main(int argc, const char** argv) {

    double distribution_mean = 10.0;
    unsigned int num_initial_events = 1;
    unsigned int num_lps = 10000;
    std::string distribution = "EXPONENTIAL";
    std::string log_statistics = "no";

    TCLAP::ValueArg<double> distribution_mean_arg("m", "mean", 
                                                    "mean delay for events", 
                                                    false, distribution_mean, "double");
    TCLAP::ValueArg<unsigned int> num_initial_events_arg("e", "events", 
                                                    "number of initial events per lp",
                                                    false, num_initial_events, "unsigned int");
    TCLAP::ValueArg<unsigned int> num_lps_arg("n", "num_lps", 
                                                    "number of lps",
                                                    false, num_lps, "unsigned int");
    TCLAP::ValueArg<std::string> distribution_arg("d", "distribution", "Statistical distribution \
                                                    for timestamp increment\nUNIFORM, POISSON, \
                                                    EXPONENTIAL, NORMAL, BINOMIAL, or FIXED", 
                                                    false, distribution, "string");
    TCLAP::ValueArg<std::string> log_statistics_arg("l", "log", 
                                                    "Post-simulation log needed - yes or no", 
                                                    false, log_statistics, "string");

    std::vector<TCLAP::Arg*> args = {&distribution_mean_arg, &num_initial_events_arg, 
                                        &num_lps_arg, &distribution_arg, &log_statistics_arg};

    warped::Simulation phold_sim {"PHOLD Simulation", argc, argv, args};

    distribution_mean = distribution_mean_arg.getValue();
    num_initial_events = num_initial_events_arg.getValue();
    num_lps = num_lps_arg.getValue();
    distribution = distribution_arg.getValue();
    log_statistics = log_statistics_arg.getValue();

    std::transform(distribution.begin(), distribution.end(), distribution.begin(), toupper);
    distribution_t dist;

    if ( distribution == "UNIFORM" ) {
        dist = UNIFORM;
        if ( distribution_mean <= 1.0 ) {
            std::cerr << "Warning: Uniform distribution needs mean > 1.0." << std::endl;
        }
    } else if ( distribution == "NORMAL" ) {
        dist = NORMAL;
    } else if ( distribution == "BINOMIAL" ) {
        dist = BINOMIAL;
        std::cerr << "Warning: binomial distribution may not advance simulation." << std::endl;
    } else if ( distribution == "POISSON" ) {
        dist = POISSON;
    } else if ( distribution == "EXPONENTIAL"  ) {
        dist = EXPONENTIAL;
    } else if ( distribution == "FIXED" ) {
        dist = FIXED;
        if ( distribution_mean < 1.0 ) {
            std::cerr << "Warning: Fixed timestamp increment less than 1.0 will not \
                                                        advance simulation." << std::endl;
        }
    } else {
        std::cerr << "Invalid distribution argument. It must be UNIFORM, POISSON, \
                                    EXPONENTIAL, NORMAL, BINOMIAL, or FIXED." << std::endl;
        exit(1);
    }

    std::vector<PholdLP> lps;
    for (unsigned int i = 0; i < num_lps; i++) {
        std::string name = std::string("LP ") + std::to_string(i);
        lps.emplace_back(name, num_initial_events, num_lps, dist, distribution_mean);
    }

    std::vector<warped::LogicalProcess*> lp_pointers;
    for (auto& lp : lps) {
        lp_pointers.push_back(&lp);
    }

    phold_sim.simulate(lp_pointers);

    if (log_statistics == "yes") {
        for (auto& lp : lps) {
            std::cout << lp.name_ << " sent " << lp.state_.messages_sent_ << " and received "
                                << lp.state_.messages_received_ << " messages." << std::endl;
        }
    }

    return 0;
}
