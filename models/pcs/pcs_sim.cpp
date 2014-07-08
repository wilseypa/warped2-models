// This model was ported from the ROSS pcs model. See https://github.com/carothersc/ROSS
// Also see "Distributed Simulation of Large-Scale PCS Networks" by Carothers et al.

#include <vector>
#include <memory>
#include <cassert>

#include "warped.hpp"
#include "pcs_sim.hpp"

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(PcsEvent);

min_t PcsEvent::min_timestamp() {
  if ( this->next_timestamp < this->completion_timestamp ) {
    if ( this->next_timestamp < this->move_timestamp ) {
      return NEXTCALL;
    }
    else {
      return MOVECALL;
    }
  }
  else {
    if ( this->completion_timestamp < this->move_timestamp ) {
      return COMPLETECALL;
    }
    else {
      return MOVECALL;
    }
  }
  assert(0);
}

std::vector<std::unique_ptr<warped::Event> > PcsCell::createInitialEvents()
{
  std::vector<std::unique_ptr<warped::Event> > events;

  for (int i = 0; i < this->num_portables; i++) {
    unsigned int completion_timestamp = (unsigned int) -1;
    unsigned int move_timestamp = 

  return events;
}

std::vector<std::unique_ptr<warped::Event> > PcsCell::receiveEvent(const warped::Event& event)
{
  std::vector<std::unique_ptr<warped::Event> > response_events;
  auto received_event = static_cast<const PcsEvent&> event;

  // TODO

  return response_events;
}

