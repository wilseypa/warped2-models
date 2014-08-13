// An implementation of Fujimoto's airport model
// Ported from the ROSS airport model (https://github.com/carothersc/ROSS/blob/master/ross/models/airport)
// Author: Eric Carver (carverer@mail.uc.edu)

#include <string>
#include <vector>
#include <memory>

#include "warped.hpp"

WARPED_DEFINE_OBJECT_STATE_STRUCT(AirportState) {
  unsigned int landings;
  unsigned int planes_flying;
  unsigned int planes_grounded;
  unsigned int wait_time;
  unsigned int furthest_flight_landing;
};

enum airport_event_t {
  ARRIVAL,
  DEPARTURE,
  LANDING
};

class AirportEvent : public warped::Event {
public:
  AirportEvent() = default;
  AirportEvent(const std::string& receiver_name, const std::string& creator_name,
               const airport_event_t type, const unsigned int timestamp,
               const unsigned int waiting_time, const unsigned int saved_furthest_flight_landing)
    : receiver_name(receiver_name), creator_name(creator_name), type(type),
      ts(timestamp), waiting_time(waiting_time),
      saved_furthest_flight_landing(saved_furthest_flight_landing) {}

  const std::string& receiverName() const { return receiver_name; }
  unsigned int timestamp() const { return ts; }

  std::string receiver_name;
  std::string creator_name;
  airport_event_t type;
  unsigned int ts;
  unsigned int waiting_time;
  unsigned int saved_furthest_flight_landing;

  WARPED_REGISTER_SERIALIZABLE_MEMBERS(receiver_name, creator_name, type, ts, waiting_time,
                                       saved_furthest_flight_landing)
};

class Airport : public warped::SimulationObject {
public:
  Airport(const std::string& name, const unsigned int num_airports, const unsigned int num_planes)
    : SimulationObject(name), state(), num_airports(num_airports), num_planes(num_planes) {}

  virtual std::vector<std::unique_ptr<warped::Event> > createInitialEvents();
  virtual std::vector<std::unique_ptr<warped::Event> > receiveEvent(const warped::Event&);
  virtual warped::ObjectState& getState() { return this->state; }

  AirportState state;

protected:
  const unsigned int num_airports;
  const unsigned int num_planes;

  
