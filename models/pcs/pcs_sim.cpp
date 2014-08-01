// This model was ported from the ROSS pcs model. See https://github.com/carothersc/ROSS
// Also see "Distributed Simulation of Large-Scale PCS Networks" by Carothers et al.

#include <vector>
#include <memory>
#include <cassert>
#include <algorithm>
#include <random>
#include <cstdlib>

#include "warped.hpp"
#include "pcs_sim.hpp"

#include "MLCG.h"
#include "NegExp.h"

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
    unsigned int move_timestamp = (unsigned int) move_expo();
    unsigned int next_timestamp = (unsigned int) next_expo();

    if ( next_timestamp < move_timestamp ) {
      // The event will be processed here
      events.emplace_back(new PcsEvent {this->name, completion_timestamp, next_timestamp,
            move_timestamp, this->name, NORMAL, NEXTCALL_METHOD });
    }
    else {
      // The event will be sent elsewhere
      std::string current_cell = this->name;
      std::string new_cell = this->name;
      std::default_random_engine gen;
      std::uniform_int_distribution<unsigned int> rand_direction(0,3);
      while ( move_timestamp > next_timestamp ) {
        current_cell = new_cell;
        new_cell = this->compute_move(rand_direction(gen));
        move_timestamp += (unsigned int) next_expo();
      }
      events.emplace_back(new PcsEvent {current_cell, completion_timestamp, next_timestamp,
            move_timestamp, new_cell, NORMAL, NEXTCALL_METHOD });
    }
  }
  return events;
}

std::vector<std::unique_ptr<warped::Event> > PcsCell::receiveEvent(const warped::Event& event)
{
  std::vector<std::unique_ptr<warped::Event> > response_events;
  auto received_event = static_cast<const PcsEvent&> event;

  if ( this->reserve_cell ) {
    
  }
  else {
    switch ( event.method_name ) {
    case NEXTCALL_METHOD:
      this->state.call_attempts++;
      if ( !this->normal_channels ) {
        // All normal channels are in use; call is blocked
        this->state.channel_blocks++;
        // Reschedule the call
        event.next_timestamp += (unsigned int) next_expo();
        if ( event.next_timestamp < event.move_timestamp ) {
          response_events.emplace_back(new PcsEvent { event });
        }
        else {
          std::string current_cell = this->name;
          std::string new_cell = this->name;
          std::default_random_engine gen;
          std::uniform_int_distribution<unsigned int> rand_direction(0,3);
          while ( event.move_timestamp > event.next_timestamp ) {
            current_cell = new_cell;
            new_cell = this->compute_move(rand_direction(gen));
            event.move_timestamp += (unsigned int) next_expo();
          }
          response_events.emplace_back(new PcsEvent { event });
        }
      }
      else {
        // The call can be connected; allocate a channel
        this->normal_channels--;
        // Randomize the call move time and end time
        unsigned int completion_timestamp = event.next_timestamp + (unsigned int) time_expo();
        unsigned int next_timestamp = event.next_timestamp + (unsigned int) next_expo();
      retry_call:
        if ( next_timestamp < event.move_timestamp ) {
          if ( completion_timestamp < next_timestamp ) {
            // The call will be completed in this cell
            response_events.emplace_back(new PcsEvent { this->name, completion_timestamp,
                  next_timestamp, event.move_timestamp, this->name, NORMAL,
                  COMPLETIONCALL_METHOD });
          }
          else {
            // Busy line; a call will be made from this cell afterward
            state.call_attempts++;
            state.busy_lines++;
            next_timestamp += (unsigned int) next_expo();
            goto retry_call;
          }
        }
        else {
          if ( completion_timestamp < event.move_timestamp ) {
            // The call will be completed in this cell
            response_events.emplace_back(new PcsEvent { this->name, completion_timestamp,
                  next_timestamp, event.move_timestamp, this->name, NORMAL,
                  COMPLETIONCALL_METHOD });
          }
          else {
            // The call was successful, but it will move cells before completion
            response_events.emplace_back(new PcsEvent { this->name, completion_timestamp,
                  next_timestamp, event.move_timestamp, this->name, NORMAL,
                  MOVECALLOUT_METHOD });
          }
        }
      break;
    case COMPLETIONCALL_METHOD:
      break;
    case MOVECALLIN_METHOD:
      break;
    case MOVECALLOUT_METHOD:
      break;
    default:
      std::cerr << "Invalid method name in event" << std::endl;
      exit(1);
  }

  return response_events;
}

std::string PcsCell::compute_move(const direction_t direction) {
  int current_x, current_y, new_x, new_y;
  current_y = ((int)this->index) / NUM_CELLS_X;
  current_x = ((int)this->index) - (current_y * NUM_CELLS_X);

  switch ( direction ) {
  case LEFT:
    new_x = ((current_x - 1) + NUM_CELLS_X) % NUM_CELLS_X;
    new_y = current_y;
    break;
  case RIGHT:
    new_x = (current_x + 1) % NUM_CELLS_X;
    new_y = current_y;
    break;
  case DOWN:
    new_x = current_x;
    new_y = ((current_y - 1) + NUM_CELLS_Y) % NUM_CELLS_Y;
    break;
  case UP:
    new_x = current_x;
    new_y = (current_y + 1) % NUM_CELLS_Y;
    break;
  default:
    std::cerr << "Invalid move direction " << direction << std::endl;
    exit(1);
  }

  return std::string("Object ") + std::to_string(new_x + (new_y * NUM_CELLS_X));
}
