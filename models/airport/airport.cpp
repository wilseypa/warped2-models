// An implementation of Fujimoto's airport model
// Ported from the ROSS airport model (https://github.com/carothersc/ROSS/blob/master/ross/models/airport)
// Author: Eric Carver (carverer@mail.uc.edu)

#include <vector>
#include <memory>
#include <random>

#include "warped.hpp"
#include "airport.hpp"

#include "MLCG.h"
#include "NegExp.h"

#include "tclap/ValueArg.h"

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(AirportEvent)

std::vector<std::shared_ptr<warped::Event> > Airport::createInitialEvents()
{
  std::vector<std::shared_ptr<warped::Event> > events;

  NegativeExpntl depart_expo((double)this->depart_mean_, this->rng_.get());

  for (unsigned int i = 0; i < this->num_planes_; i++) {
    events.emplace_back(new AirportEvent {this->name_, DEPARTURE, 
                                            (unsigned int)depart_expo()});
  }

  return events;
}

inline std::string Airport::object_name(const unsigned int object_index)
{
  return std::string("Object ") + std::to_string(object_index);
}

std::vector<std::shared_ptr<warped::Event> > Airport::receiveEvent(const warped::Event& event)
{
  std::vector<std::shared_ptr<warped::Event> > response_events;
  auto received_event = static_cast<const AirportEvent&>(event);

  NegativeExpntl depart_expo((double)this->depart_mean_, this->rng_.get());
  NegativeExpntl land_expo((double)this->land_mean_, this->rng_.get());

  // std::uniform_int_distribution<unsigned int> rand_direction(0,3);
  std::uniform_int_distribution<unsigned int> rand_airport(0,this->num_airports_-1);

  switch ( received_event.type_ ) {
  case DEPARTURE:
    {
      this->state_.planes_grounded_--;
      this->state_.planes_flying_++;
      this->state_.departures_++;
      // Schedule an arrival at a random airport
      unsigned int landing_time = received_event.ts_ + (unsigned int)land_expo();
      // direction_t direction = (direction_t) rand_direction(this->rng_engine_);
      unsigned int destination_index = rand_airport(this->rng_engine_);
      response_events.emplace_back(new AirportEvent {Airport::object_name(destination_index),
                                                            ARRIVAL, landing_time });
      break;
    }
  case ARRIVAL:
    {
      // Schedule a landing
      response_events.emplace_back(new AirportEvent { this->name_, LANDING, received_event.ts_ });
      break;
    }
  case LANDING:
    {
      this->state_.landings_++;
      this->state_.planes_flying_--;
      this->state_.planes_grounded_++;
      // Schedule a departure
      response_events.emplace_back(new AirportEvent { this->name_, DEPARTURE,
                                            received_event.ts_ + (unsigned int)depart_expo() });
      break;
    }
  }
  return response_events;
}

int main(int argc, const char** argv)
{
  unsigned int num_airports = 1024;
  unsigned int mean_ground_time = 10;
  unsigned int mean_flight_time = 30;
  unsigned int num_planes = 1;

  TCLAP::ValueArg<unsigned int> num_airports_arg("n", "num-airports", "Number of airports",
                                                 false, num_airports, "unsigned int");
  TCLAP::ValueArg<unsigned int> mean_ground_time_arg("g", "ground-time", "Mean time of planes waiting to depart",
                                                     false, mean_ground_time, "unsigned int");
  TCLAP::ValueArg<unsigned int> mean_flight_time_arg("f", "flight-time", "Mean flight time",
                                                     false, mean_flight_time, "unsigned int");
  TCLAP::ValueArg<unsigned int> num_planes_arg("p", "num-planes", "Number of planes per airport",
                                               false, num_planes, "unsigned int");

  std::vector<TCLAP::Arg*> args = {&num_airports_arg, &mean_ground_time_arg, &mean_flight_time_arg,
                                   &num_planes_arg};

  warped::Simulation airport_sim {"Airport Simulation", argc, argv, args};

  num_airports = num_airports_arg.getValue();
  mean_ground_time = mean_ground_time_arg.getValue();
  mean_flight_time = mean_flight_time_arg.getValue();
  num_planes = num_planes_arg.getValue();

  std::vector<Airport> objects;

  for (unsigned int i = 0; i < num_airports; i++) {
    std::string name = Airport::object_name(i);
    objects.emplace_back(name, num_airports, num_planes, mean_flight_time, mean_ground_time, i);
  }

  std::vector<warped::SimulationObject*> object_pointers;
  for (auto& o : objects) {
    object_pointers.push_back(&o);
  }

  airport_sim.simulate(object_pointers);

  unsigned int landings = 0;
  unsigned int departures = 0;

  for (auto& o : objects) {
    landings += o.state_.landings_;
    departures += o.state_.departures_;
  }

  std::cout << departures << " total departures" << std::endl;
  std::cout << landings << " total landings" << std::endl;

  return 0;
}
