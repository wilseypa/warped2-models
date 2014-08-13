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

virtual std::vector<std::unique_ptr<warped::Event> > Airport::receiveEvent(const warped::Event& event)
{
  std::vector<std::unique_ptr<warped::Event> > response_events;
  auto received_event = static_cast<const AirportEvent&>(event);

  std::exponential_distribution<unsigned int> depart_expo(this->depart_mean);
  std::exponential_distribution<unsigned int> land_expo(this->land_mean);

  switch ( received_event.type ) {
  case DEPARTURE: {
    this->state.grounded_planes--;
    unsigned int landing_time = received_event.ts + land_expo(this->rng_engine);
    
