// An implementation of Fujimoto's airport model
// Ported from the ROSS airport model (https://github.com/carothersc/ROSS/blob/master/ross/models/airport)
// Author: Eric Carver (carverer@mail.uc.edu)

#include <vector>
#include <memory>
#include <random>

#include "warped.hpp"
#include "airport.hpp"

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(AirportEvent)

virtual std::vector<std::unique_ptr<warped::Event> > Airport::createInitialEvents()
{
  std::vector<std::unique_ptr<warped::Event> > events;

  std::exponential_distribution<unsigned int> depart_expo(this->depart_mean);

  for (int i = 0; i < this->num_planes; i++) {
    events.emplace_back(new AirportEvent {this->name, this->name, DEPARTURE, depart_expo(rng_engine), 0, 0});
  }

  return events;
}

static inline std::string Airport::object_name(const unsigned int object_index)
{
  return std::string("Object ") + std::to_string(object_index);
}

virtual std::vector<std::unique_ptr<warped::Event> > Airport::receiveEvent(const warped::Event& event)
{
  std::vector<std::unique_ptr<warped::Event> > response_events;
  auto received_event = static_cast<const AirportEvent&>(event);

  std::exponential_distribution<unsigned int> depart_expo(this->depart_mean);
  std::exponential_distribution<unsigned int> land_expo(this->land_mean);
  // std::uniform_int_distribution<unsigned int> rand_direction(0,3);
  std::uniform_int_distribution<unsigned int> rand_airport(0,this->num_airports-1);

  switch ( received_event.type ) {
  case DEPARTURE:
    {
      this->state.planes_grounded--;
      this->state.planes_flying++;
      this->state.departures++;
      // Schedule an arrival at a random airport
      unsigned int landing_time = received_event.ts + land_expo(this->rng_engine);
      // direction_t direction = (direction_t) rand_direction(this->rng_engine);
      unsigned int destination_index = rand_airport(this->rng_engine);
      response_events.emplace_back(new AirportEvent {Airport::object_name(destination_index), this->name,
            ARRIVAL, landing_time });
      break;
    }
  case ARRIVAL:
    {
      // Schedule a landing
      response_events.emplace_back(new AirportEvent { this->name, this->name, LANDING, received_event.ts });
      break;
    }
  case LANDING:
    {
      this->state.landings++;
      this->state.planes_flying--;
      this->state.planes_grounded++;
      // Schedule a departure
      response_events.emplace_back(new AirportEvent { this->name, this->name, DEPARTURE,
            received_event.ts + depart_expo(this->rng_engine) });
      break;
    }
  }
  return response_events;
}
