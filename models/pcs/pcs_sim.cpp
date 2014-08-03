// This model was ported from the ROSS pcs model. See https://github.com/carothersc/ROSS
// Also see "Distributed Simulation of Large-Scale PCS Networks" by Carothers et al.
// Author: Eric Carver carverer@mail.uc.edu

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

#include "tclap/ValueArg.h"

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(PcsEvent)

unsigned int PcsEvent::timestamp() const
{
  if ( this->move_timestamp < this->completion_timestamp ) {
    if ( this->next_timestamp < this->move_timestamp ) {
      return this->next_timestamp;
    }
    else {
      return this->move_timestamp;
    }
  }
  else {
    if ( this->next_timestamp < this->completion_timestamp ) {
      return this->next_timestamp;
    }
    else {
      return this->completion_timestamp;
    }
  }
}

std::vector<std::unique_ptr<warped::Event> > PcsCell::createInitialEvents()
{
  std::vector<std::unique_ptr<warped::Event> > events;

  NegativeExpntl move_expo(move_call_mean, this->rng.get());
  NegativeExpntl next_expo(next_call_mean, this->rng.get());
  NegativeExpntl time_expo(call_time_mean, this->rng.get());

  for (unsigned int i = 0; i < this->num_portables; i++) {
    unsigned int completion_timestamp = (unsigned int) -1;
    unsigned int move_timestamp = (unsigned int) move_expo();
    unsigned int next_timestamp = (unsigned int) next_expo();

    if ( next_timestamp < move_timestamp ) {
      // The event will be processed here
      events.emplace_back(new PcsEvent {this->name_, completion_timestamp, next_timestamp,
            move_timestamp, this->name_, NORMAL, NEXTCALL_METHOD });
    }
    else {
      // The event will be sent elsewhere
      std::string current_cell = this->name_;
      std::string new_cell = this->name_;
      while ( move_timestamp < next_timestamp ) {
        current_cell = new_cell;
        new_cell = this->random_move();
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
  auto received_event = static_cast<const PcsEvent&>(event);

  NegativeExpntl move_expo(move_call_mean, this->rng.get());
  NegativeExpntl next_expo(next_call_mean, this->rng.get());
  NegativeExpntl time_expo(call_time_mean, this->rng.get());

  switch ( received_event.method_name ) {
  case NEXTCALL_METHOD:
    {
      // Attempt to place a call
      this->state.call_attempts++;
      if ( !state.normal_channels ) {
        // All normal channels are in use; call is blocked
        this->state.channel_blocks++;
        // Reschedule the call
        unsigned int next_timestamp = received_event.next_timestamp + (unsigned int) next_expo();
        if ( next_timestamp < received_event.move_timestamp ) {
          response_events.emplace_back(new PcsEvent { this->name_, received_event.completion_timestamp,
                next_timestamp, received_event.move_timestamp, this->name_, NORMAL, NEXTCALL_METHOD });
        }
        else {
          std::string current_cell = this->name_;
          std::string new_cell = this->name_;
          while ( received_event.move_timestamp < next_timestamp ) {
            current_cell = new_cell;
            new_cell = this->random_move();
            received_event.move_timestamp += (unsigned int) move_expo();
          }
          response_events.emplace_back(new PcsEvent { this->name_, received_event.completion_timestamp,
                next_timestamp, received_event.move_timestamp, this->name_, NORMAL, NEXTCALL_METHOD });
        }
      }
      else {
        // The call can be connected; allocate a channel
        state.normal_channels--;
        // Randomize the call move time and end time
        unsigned int completion_timestamp = received_event.next_timestamp + (unsigned int) time_expo();
        unsigned int next_timestamp = received_event.next_timestamp + (unsigned int) next_expo();
      busy_line_nextcall:
        if ( next_timestamp < received_event.move_timestamp ) {
          if ( completion_timestamp < next_timestamp ) {
            // The call will be completed in this cell
            response_events.emplace_back(new PcsEvent { this->name_, completion_timestamp,
                  next_timestamp, received_event.move_timestamp, this->name_, NORMAL,
                  COMPLETIONCALL_METHOD });
          }
          else {
            // Busy line; a call will be made from this cell afterward
            state.call_attempts++;
            state.busy_lines++;
            next_timestamp += (unsigned int) next_expo();
            goto busy_line_nextcall;
          }
        }
        else {
          if ( completion_timestamp < received_event.move_timestamp ) {
            // The call will be completed in this cell
            response_events.emplace_back(new PcsEvent { this->name_, completion_timestamp,
                  next_timestamp, received_event.move_timestamp, this->name_, NORMAL,
                  COMPLETIONCALL_METHOD });
          }
          else {
            // The call was successful, but it will move cells before completion
            response_events.emplace_back(new PcsEvent { this->name_, completion_timestamp,
                  next_timestamp, received_event.move_timestamp, this->name_, NORMAL,
                  MOVECALLOUT_METHOD });
          }
        }
      }
      break;
    }
  case COMPLETIONCALL_METHOD:
    {
      // End of a call
      // Deallocate the channel
      if ( received_event.channel == NORMAL ) {
        state.normal_channels++;
      }
      else if ( received_event.channel == RESERVE ) {
        state.reserve_channels++;
      }
      else {
        std::cerr << "Invalid channel type when completing call" << std::endl;
        exit(1);
      }

      if ( received_event.next_timestamp < received_event.move_timestamp ) {
        // The portable will attempt a new call in this cell
        response_events.emplace_back(new PcsEvent { this->name_, (unsigned int) -1,
              received_event.next_timestamp, received_event.move_timestamp, this->name_, NONE, NEXTCALL_METHOD });
      }
      else {
        // The portable will move to a different cell while not on a call
        // Find out where the next call will take place
        std::string new_cell = this->name_;
        std::string current_cell = this->name_;
        unsigned int move_timestamp = received_event.move_timestamp;
        unsigned int next_timestamp = received_event.next_timestamp;
        while ( move_timestamp < next_timestamp ) {
          current_cell = new_cell;
          new_cell = this->random_move();
          move_timestamp += (unsigned int) move_expo();
        }
        response_events.emplace_back(new PcsEvent { this->name_, (unsigned int) -1,
              next_timestamp, move_timestamp, current_cell, NONE, NEXTCALL_METHOD });
      }
      break;
    }
  case MOVECALLIN_METHOD:
    {
      unsigned int move_timestamp = received_event.move_timestamp + (unsigned int) move_expo();
      if ( !state.normal_channels && !state.reserve_channels ) {
        // No normal or reserve channels are available; the call is dropped
        state.handoff_blocks++;
        if ( received_event.next_timestamp < move_timestamp ) {
          // The portable will attempt a new call in this cell
          response_events.emplace_back(new PcsEvent { this->name_, (unsigned int) -1,
                received_event.next_timestamp, move_timestamp, this->name_, NONE, NEXTCALL_METHOD });
        }
        else {
          // The portable will move to a different cell while not on a call
          // Find out where the next call will take place
          std::string new_cell = this->name_;
          std::string current_cell = this->name_;
          unsigned int next_timestamp = received_event.next_timestamp;
          while ( move_timestamp < next_timestamp ) {
            current_cell = new_cell;
            new_cell = this->random_move();
            move_timestamp += (unsigned int) move_expo();
          }
          response_events.emplace_back(new PcsEvent { this->name_, (unsigned int) -1,
                next_timestamp, move_timestamp, current_cell, NONE, NEXTCALL_METHOD });
        }
      }
      else {
        channel_t channel;
        if ( state.normal_channels ) {
          // A normal channel is available for the call; allocate it
          state.normal_channels--;
          channel = NORMAL;
        }
        else {
          // No normal channels are available; allocate a reserve channel
          state.reserve_channels--;
          channel = RESERVE;
        }
        unsigned int next_timestamp = received_event.next_timestamp;
      busy_line_movecallin:
        if ( next_timestamp < move_timestamp ) {
          if ( received_event.completion_timestamp < next_timestamp ) {
            // The call will end in this cell
            response_events.emplace_back(new PcsEvent { this->name_, received_event.completion_timestamp,
                  next_timestamp, move_timestamp, this->name_, channel,
                  COMPLETIONCALL_METHOD });
          }
          else {
            // A call will be attempted while the line is busy
            state.busy_lines++;
            state.call_attempts++;
            next_timestamp += next_expo();
            goto busy_line_movecallin;
          }
        }
        else {
          if ( received_event.completion_timestamp < move_timestamp ) {
            // The call will end in this cell
            response_events.emplace_back(new PcsEvent { this->name_, received_event.completion_timestamp,
                  next_timestamp, move_timestamp, this->name_, channel,
                  COMPLETIONCALL_METHOD });
          }
          else {
            // The call will be moved out of this cell before it ends
            response_events.emplace_back(new PcsEvent { this->name_, received_event.completion_timestamp,
                  next_timestamp, move_timestamp, this->name_, channel,
                  MOVECALLOUT_METHOD });
          }
        }
      }
      break;
    }
  case MOVECALLOUT_METHOD:
    {
      // Free a channel of whatever type this call is on
      if ( received_event.channel == NORMAL ) {
        state.normal_channels++;
      }
      else if ( received_event.channel == RESERVE ) {
        state.reserve_channels++;
      }
      else {
        std::cerr << "Invalid channel type when transferring call to new cell" << std::endl;
        exit(1);
      }
      // Move call to random adjacent cell
      response_events.emplace_back(new PcsEvent { this->name_, received_event.completion_timestamp,
            received_event.next_timestamp, received_event.move_timestamp, this->random_move(), NONE,
            MOVECALLIN_METHOD });
      break;
    }
  default:
    {
      std::cerr << "Invalid method name in event" << std::endl;
      exit(1);
    }
  }

  return response_events;
}

std::string PcsCell::compute_move(const direction_t direction) const {
  int current_x, current_y, new_x, new_y;
  current_y = ((int)this->index) / num_cells_x;
  current_x = ((int)this->index) - (current_y * num_cells_x);

  switch ( direction ) {
  case LEFT:
    new_x = ((current_x - 1) + num_cells_x) % num_cells_x;
    new_y = current_y;
    break;
  case RIGHT:
    new_x = (current_x + 1) % num_cells_x;
    new_y = current_y;
    break;
  case DOWN:
    new_x = current_x;
    new_y = ((current_y - 1) + num_cells_y) % num_cells_y;
    break;
  case UP:
    new_x = current_x;
    new_y = (current_y + 1) % num_cells_y;
    break;
  default:
    std::cerr << "Invalid move direction " << direction << std::endl;
    exit(1);
  }

  return std::string("Object ") + std::to_string(new_x + (new_y * num_cells_x));
}

std::string PcsCell::random_move() const {
  std::default_random_engine gen;
  std::uniform_int_distribution<unsigned int> rand_direction(0,3);
  return this->compute_move((direction_t)rand_direction(gen));
}

int main(int argc, const char** argv) {
  unsigned int num_cells_x = 1024;
  unsigned int num_cells_y = 1024;
  double move_call_mean = 4500.0;
  double next_call_mean = 360.0;
  double call_time_mean = 180.0;
  unsigned int normal_channels = 10;
  unsigned int reserve_channels = 0;
  unsigned int num_portables = 16;

  TCLAP::ValueArg<unsigned int> num_cells_x_arg("x", "num-cells-x", "Width of cell grid",
                                                false, num_cells_x, "unsigned int");
  TCLAP::ValueArg<unsigned int> num_cells_y_arg("y", "num-cells-y", "Height of cell grid",
                                                false, num_cells_y, "unsigned int");
  TCLAP::ValueArg<double> move_call_mean_arg("m", "move-time",
                                             "Mean time each portable resides in a cell",
                                             false, move_call_mean, "double");
  TCLAP::ValueArg<double> next_call_mean_arg("n", "next-time",
                                             "Mean time between call attempts by a portable",
                                             false, next_call_mean, "double");
  TCLAP::ValueArg<double> call_time_mean_arg("t", "call-time", "Mean call length", false,
                                             call_time_mean, "double");
  TCLAP::ValueArg<unsigned int> normal_channels_arg("s", "channels",
                                   "Number of Normal communication channels per cell",
                                                    false, normal_channels, "unsigned int");
  TCLAP::ValueArg<unsigned int> reserve_channels_arg("r", "reserve-channels",
                                     "Number of Reserve communication channels per cell",
                                                     false, reserve_channels, "unsigned int");
  TCLAP::ValueArg<unsigned int> num_portables_arg("p", "portables", "Portables per cell",
                                                  false, num_portables, "unsigned int");

  std::vector<TCLAP::Arg*> args = {&num_cells_x_arg, &num_cells_y_arg, &move_call_mean_arg,
                                   &next_call_mean_arg, &call_time_mean_arg,
                                   &normal_channels_arg, &reserve_channels_arg,
                                   &num_portables_arg};

  warped::Simulation pcs_sim {"PCS Simulation", argc, argv, args};

  num_cells_x = num_cells_x_arg.getValue();
  num_cells_y = num_cells_y_arg.getValue();
  move_call_mean = move_call_mean_arg.getValue();
  next_call_mean = next_call_mean_arg.getValue();
  call_time_mean = call_time_mean_arg.getValue();
  normal_channels = normal_channels_arg.getValue();
  reserve_channels = reserve_channels_arg.getValue();
  num_portables = num_portables_arg.getValue();

  std::vector<PcsCell> objects;

  for (unsigned int i = 0; i < num_cells_x * num_cells_y - 1; i++) {
    std::string name = std::string("Object ") + std::to_string(i);
    objects.emplace_back(name, num_cells_x, num_cells_y, num_portables, normal_channels,
                         reserve_channels, call_time_mean, next_call_mean, move_call_mean, i);
  }

  std::vector<warped::SimulationObject*> object_pointers;
  for (auto& o : objects) {
    object_pointers.push_back(&o);
  }

  pcs_sim.simulate(object_pointers);

  unsigned int handoff_blocks = 0;
  unsigned int busy_lines = 0;
  unsigned int call_attempts = 0;
  unsigned int channel_blocks = 0;

  for (auto& o : objects) {
    handoff_blocks += o.state.handoff_blocks;
    busy_lines += o.state.busy_lines;
    call_attempts += o.state.call_attempts;
    channel_blocks += o.state.channel_blocks;
  }

  std::cout << call_attempts << " calls attempted" << std::endl;
  std::cout << busy_lines << " busy lines" << std::endl;
  std::cout << channel_blocks << " blocked calls" << std::endl;
  std::cout << handoff_blocks << " dropped calls" << std::endl;

  return 0;
}
