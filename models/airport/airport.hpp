// An implementation of Fujimoto's airport model
// Ported from the ROSS airport model (https://github.com/carothersc/ROSS/blob/master/ross/models/airport)
// Author: Eric Carver (carverer@mail.uc.edu)

#ifndef AIRPORT_HPP_DEFINED
#define AIRPORT_HPP_DEFINED

#include <string>
#include <vector>
#include <memory>

#include "warped.hpp"

WARPED_DEFINE_OBJECT_STATE_STRUCT(AirportState) {
  unsigned int landings;
  unsigned int departures;
  unsigned int planes_flying;
  unsigned int planes_grounded;
};

enum airport_event_t {
  ARRIVAL,
  DEPARTURE,
  LANDING
};

enum direction_t {
  NORTH,
  EAST,
  SOUTH,
  WEST
};

class AirportEvent : public warped::Event {
public:
  AirportEvent() = default;
  AirportEvent(const std::string& receiver_name, const std::string& creator_name,
               const airport_event_t type, const unsigned int timestamp)
    : receiver_name(receiver_name), creator_name(creator_name), type(type), ts(timestamp) {}

  const std::string& receiverName() const { return receiver_name; }
  unsigned int timestamp() const { return ts; }

  std::string receiver_name;
  std::string creator_name;
  airport_event_t type;
  unsigned int ts;

  WARPED_REGISTER_SERIALIZABLE_MEMBERS(receiver_name, creator_name, type, ts);
};

class Airport : public warped::SimulationObject {
public:
  Airport(const std::string& name, const unsigned int num_airports, const unsigned int num_planes,
          const unsigned int land_mean, const unsigned int depart_mean, const unsigned int index)
    : SimulationObject(name), state(), num_airports(num_airports), num_planes(num_planes),
      land_mean(land_mean), depart_mean(depart_mean), index(index) {}

  virtual std::vector<std::unique_ptr<warped::Event> > createInitialEvents();
  virtual std::vector<std::unique_ptr<warped::Event> > receiveEvent(const warped::Event&);
  virtual warped::ObjectState& getState() { return this->state; }

  AirportState state;

  static inline std::string object_name(const unsigned int);

protected:
  std::default_random_engine rng_engine;
  const unsigned int num_airports;
  const unsigned int num_planes;
  const unsigned int land_mean;
  const unsigned int depart_mean;
  const unsigned int index;
};

#endif
