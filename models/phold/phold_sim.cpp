// An implementation of the synthetic PHOLD simulation model
// See "Performance of Time Warp Under Synthetic Workloads" by R.M. Fujimoto

#include <string>
#include <vector>
#include <memory>
#include <iostream>

#include "warped.hpp"
#include "tclap/ValueArg.h"

#include "MLCG.h"
#include "Normal.h"
#include "Poisson.h"
#include "Binomial.h"
#include "Uniform.h"
#include "NegExp.h"
#include "DiscUnif.h"

enum distribution_t {UNIFORM, POISSON, EXPONENTIAL, NORMAL, BINOMIAL, FIXED,
                     ALTERNATE, ROUNDROBIN, CONDITIONAL, ALL};

WARPED_DEFINE_OBJECT_STATE_STRUCT(PholdState) { };

class PholdEvent : public warped::Event {
public:
  PholdEvent() = default;
  PholdEvent(const std::string& receiver_name, const unsigned int timestamp,
             const std::string& creator_name)
    : creator_name(creator_name), receiver_name(receiver_name), time_stamp(timestamp) {}

  const std::string& receiverName() const { return receiver_name; }
  unsigned int timestamp() const { return time_stamp; }

  std::string creator_name;
  std::string receiver_name;
  unsigned int time_stamp;

  WARPED_REGISTER_SERIALIZABLE_MEMBERS(creator_name, receiver_name, time_stamp)
};

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(PholdEvent)

class PholdObject : public warped::SimulationObject {
public:
  PholdObject(const std::string& name, const unsigned int initial_events,
              const unsigned int num_objects, const distribution_t distribution,
              const double distribution_mean = 1.0)
    : SimulationObject(name), state(), initial_events(initial_events),
      num_objects(num_objects), rng(new MLCG), distribution(distribution),
      distribution_mean(distribution_mean) {}

  warped::ObjectState& getState() { return this->state; }

  std::vector<std::unique_ptr<warped::Event> > createInitialEvents() {
    std::vector<std::unique_ptr<warped::Event> > events;
    for (unsigned int i = 0; i < this->initial_events; i++) {
      events.emplace_back(new PholdEvent { this->get_destination(), this->get_timestamp_delay(),
            this->name });
    }
    return events;
  }

  std::vector<std::unique_ptr<warped::Event> > receiveEvent(const warped::Event& event) {
    std::vector<std::unique_ptr<warped::Event> > response_events;
    auto received_event = static_cast<const PholdEvent&>(event);

    response_events.emplace_back(new PholdEvent { this->get_destination(),
          event.timestamp() + this->get_timestamp_delay(), this->name });

    return response_events;
  }

protected:
  std::string name;
  PholdState state;
  const unsigned int initial_events;
  const unsigned int num_objects;
  std::unique_ptr<MLCG> rng;
  const distribution_t distribution;
  const double distribution_mean;

  std::string get_destination() const {
    DiscreteUniform Dest(0, ((int)(num_objects))-1, this->rng.get());
    unsigned int destination_number = (unsigned int) Dest();
    return std::string("Object ") + std::to_string(destination_number);
  }

  unsigned int get_timestamp_delay() const {

    double delay;

    switch ( this->distribution ) {
    case UNIFORM :
      {
        Uniform uniform(this->distribution_mean, 0.0, this->rng.get());
        delay = uniform();
        break;
      }
    case NORMAL :
      {
        Normal normal(this->distribution_mean, 0.0, this->rng.get());
        delay = normal();
        break;
      }
    case BINOMIAL :
      {
        Binomial binomial((int)this->distribution_mean, 0.0, this->rng.get());
        delay = binomial();
        break;
      }
    case POISSON :
      {
        Poisson poisson(this->distribution_mean, this->rng.get());
        delay = poisson();
        break;
      }
    case EXPONENTIAL :
      {
        NegativeExpntl expo(this->distribution_mean, this->rng.get());
        delay = expo();
        break;
      }
    case FIXED :
      delay = (unsigned int)this->distribution_mean ;
      break;
    default :
      delay = 0;
      std::cerr << "Improper Distribution for a Source Object!!!" << std::endl;
      break;
    }
    return ( (unsigned int) delay );
  }
};

int main(int argc, const char** argv) {
  double distribution_mean = 1.0;
  unsigned int num_initial_events = 10;
  unsigned int num_objects = 10;
  distribution_t distribution = EXPONENTIAL;

  TCLAP::ValueArg<double> distribution_mean_arg("m", "mean", "mean delay for events", false,
                                                distribution_mean, "double");
  TCLAP::ValueArg<unsigned int> num_initial_events_arg("e", "events", "number of initial events per object",
                                                       false, num_initial_events, "unsigned int");
  TCLAP::ValueArg<unsigned int> num_objects_arg("n", "num_objects", "number of simulation objects",
                                                false, num_objects, "unsigned int");

  std::vector<TCLAP::Arg*> args = {&distribution_mean_arg, &num_initial_events_arg, &num_objects_arg};

  warped::Simulation phold_sim {"PHOLD Simulation", argc, argv, args};

  distribution_mean = distribution_mean_arg.getValue();
  num_initial_events = num_initial_events_arg.getValue();
  num_objects = num_objects_arg.getValue();

  std::vector<PholdObject> objects;

  for (unsigned int i = 0; i < num_objects; i++) {
    std::string name = std::string("Object ") + std::to_string(i);
    objects.emplace_back(name, num_initial_events, num_objects, distribution, distribution_mean);
  }

  std::vector<warped::SimulationObject*> object_pointers;
  for (auto& o : objects) {
    object_pointers.push_back(&o);
  }

  phold_sim.simulate(object_pointers);

  return 0;
}
