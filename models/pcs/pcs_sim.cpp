// See "Distributed Simulation of Large-Scale PCS Networks" by Carothers et al.

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

std::vector<std::shared_ptr<warped::Event> > PcsCell::createInitialEvents() {

    std::vector<std::shared_ptr<warped::Event>> events;
    for (unsigned int i = 0; i < this->state_.portables_.size(); i++) {
        unsigned int call_timestamp = this->state_.portables_[i]->call_interval_;
        events.emplace_back(new PcsEvent {this->name_, call_timestamp, 
                                    this->state_.portables_[i], CALL_ARRIVAL});
    }
    return events;
}

std::vector<std::shared_ptr<warped::Event> > PcsCell::receiveEvent(const warped::Event& event) {

    std::vector<std::shared_ptr<warped::Event>> events;
    auto pcs_event = static_cast<const PcsEvent&>(event);

    switch (pcs_event.event_type_) {

        case CALL_ARRIVAL: {

        } break;

        case CALL_COMPLETION: {

        } break;

        case PORTABLE_MOVE_IN: {

        } break;

        case PORTABLE_MOVE_OUT: {

        } break;

        default: {}
    }
    return events;
}


#if 0
        case NEXTCALL_METHOD: {
      // Attempt to place a call
      this->state_.call_attempts_++;
      if ( !state_.normal_channels_ ) {
        // All normal channels are in use; call is blocked
        this->state_.channel_blocks_++;
        // Reschedule the call
        unsigned int next_timestamp = received_event.next_timestamp_ + (unsigned int) next_expo();
        if ( next_timestamp < received_event.move_timestamp_ ) {
          response_events.emplace_back(new PcsEvent { this->name_, received_event.completion_timestamp_,
                                next_timestamp, received_event.move_timestamp_, NORMAL, NEXTCALL_METHOD });
        } else {
          std::string current_cell = this->name_;
          std::string new_cell = this->name_;
          while ( received_event.move_timestamp_ < next_timestamp ) {
            current_cell = new_cell;
            new_cell = this->random_move();
            received_event.move_timestamp_ += (unsigned int) move_expo();
          }
          response_events.emplace_back(new PcsEvent { this->name_, received_event.completion_timestamp_,
                                next_timestamp, received_event.move_timestamp_, NORMAL, NEXTCALL_METHOD });
        }
      } else {
        // The call can be connected; allocate a channel
        state_.normal_channels_--;
        // Randomize the call move time and end time
        unsigned int completion_timestamp = received_event.next_timestamp_ + (unsigned int) time_expo();
        unsigned int next_timestamp = received_event.next_timestamp_ + (unsigned int) next_expo();
      busy_line_nextcall:
        if ( next_timestamp < received_event.move_timestamp_ ) {
          if ( completion_timestamp < next_timestamp ) {
            // The call will be completed in this cell
            response_events.emplace_back(new PcsEvent { this->name_, completion_timestamp,
                            next_timestamp, received_event.move_timestamp_, NORMAL, COMPLETIONCALL_METHOD });
          } else {
            // Busy line; a call will be made from this cell afterward
            state_.call_attempts_++;
            state_.busy_lines_++;
            next_timestamp += (unsigned int) next_expo();
            goto busy_line_nextcall;
          }
        } else {
          if ( completion_timestamp < received_event.move_timestamp_ ) {
            // The call will be completed in this cell
            response_events.emplace_back(new PcsEvent { this->name_, completion_timestamp,
                  next_timestamp, received_event.move_timestamp_, NORMAL, COMPLETIONCALL_METHOD });
          } else {
            // The call was successful, but it will move cells before completion
            response_events.emplace_back(new PcsEvent { this->name_, completion_timestamp,
                  next_timestamp, received_event.move_timestamp_, NORMAL, MOVECALLOUT_METHOD });
          }
        }
      }
      break;
    }
  case COMPLETIONCALL_METHOD:
    {
      // End of a call
      // Deallocate the channel
      if ( received_event.channel_ == NORMAL ) {
        state_.normal_channels_++;
      } else if ( received_event.channel_ == RESERVE ) {
        state_.reserve_channels_++;
      } else {
        std::cerr << "Invalid channel type when completing call" << std::endl;
        exit(1);
      }

      if ( received_event.next_timestamp_ < received_event.move_timestamp_ ) {
        // The portable will attempt a new call in this cell
        response_events.emplace_back(new PcsEvent { this->name_, (unsigned int) -1,
              received_event.next_timestamp_, received_event.move_timestamp_, NONE, NEXTCALL_METHOD });
      } else {
        // The portable will move to a different cell while not on a call
        // Find out where the next call will take place
        std::string new_cell = this->name_;
        std::string current_cell = this->name_;
        unsigned int move_timestamp = received_event.move_timestamp_;
        unsigned int next_timestamp = received_event.next_timestamp_;
        while ( move_timestamp < next_timestamp ) {
          current_cell = new_cell;
          new_cell = this->random_move();
          move_timestamp += (unsigned int) move_expo();
        }
        response_events.emplace_back(new PcsEvent { this->name_, (unsigned int) -1,
                                            next_timestamp, move_timestamp, NONE, NEXTCALL_METHOD });
      }
      break;
    }
  case MOVECALLIN_METHOD:
    {
      unsigned int move_timestamp = received_event.move_timestamp_ + (unsigned int) move_expo();
      if ( !state_.normal_channels_ && !state_.reserve_channels_ ) {
        // No normal or reserve channels are available; the call is dropped
        state_.handoff_blocks_++;
        if ( received_event.next_timestamp_ < move_timestamp ) {
          // The portable will attempt a new call in this cell
          response_events.emplace_back(new PcsEvent { this->name_, (unsigned int) -1,
                        received_event.next_timestamp_, move_timestamp, NONE, NEXTCALL_METHOD });
        } else {
          // The portable will move to a different cell while not on a call
          // Find out where the next call will take place
          std::string new_cell = this->name_;
          std::string current_cell = this->name_;
          unsigned int next_timestamp = received_event.next_timestamp_;
          while ( move_timestamp < next_timestamp ) {
            current_cell = new_cell;
            new_cell = this->random_move();
            move_timestamp += (unsigned int) move_expo();
          }
          response_events.emplace_back(new PcsEvent { this->name_, (unsigned int) -1,
                                        next_timestamp, move_timestamp, NONE, NEXTCALL_METHOD });
        }
      } else {
        channel_t channel;
        if ( state_.normal_channels_ ) {
          // A normal channel is available for the call; allocate it
          state_.normal_channels_--;
          channel = NORMAL;
        } else {
          // No normal channels are available; allocate a reserve channel
          state_.reserve_channels_--;
          channel = RESERVE;
        }
        unsigned int next_timestamp = received_event.next_timestamp_;
      busy_line_movecallin:
        if ( next_timestamp < move_timestamp ) {
          if ( received_event.completion_timestamp_ < next_timestamp ) {
            // The call will end in this cell
            response_events.emplace_back(new PcsEvent { this->name_, received_event.completion_timestamp_,
                                next_timestamp, move_timestamp, channel, COMPLETIONCALL_METHOD });
          } else {
            // A call will be attempted while the line is busy
            state_.busy_lines_++;
            state_.call_attempts_++;
            next_timestamp += next_expo();
            goto busy_line_movecallin;
          }
        } else {
          if ( received_event.completion_timestamp_ < move_timestamp ) {
            // The call will end in this cell
            response_events.emplace_back(new PcsEvent { this->name_, received_event.completion_timestamp_,
                                        next_timestamp, move_timestamp, channel, COMPLETIONCALL_METHOD });
          } else {
            // The call will be moved out of this cell before it ends
            response_events.emplace_back(new PcsEvent { this->name_, received_event.completion_timestamp_,
                                        next_timestamp, move_timestamp, channel, MOVECALLOUT_METHOD });
          }
        }
      }
      break;
    }
  case MOVECALLOUT_METHOD:
    {
      // Free a channel of whatever type this call is on
      if ( received_event.channel_ == NORMAL ) {
        state_.normal_channels_++;
      } else if ( received_event.channel_ == RESERVE ) {
        state_.reserve_channels_++;
      } else {
        std::cerr << "Invalid channel type when transferring call to new cell" << std::endl;
        exit(1);
      }
      // Move call to random adjacent cell
      response_events.emplace_back(new PcsEvent { this->name_, received_event.completion_timestamp_,
            received_event.next_timestamp_, received_event.move_timestamp_, NONE, MOVECALLIN_METHOD });
      break;
    }
  default:
    {
      std::cerr << "Invalid method name in event" << std::endl;
      exit(1);
    }
  }
#endif


std::string PcsCell::compute_move(direction_t direction) {

    int new_x = 0, new_y = 0;
    int current_y = ((int)this->index_) / num_cells_x_;
    int current_x = ((int)this->index_) - (current_y * num_cells_x_);

    switch (direction) {

        case LEFT: {
            new_x = ((current_x - 1) + num_cells_x_) % num_cells_x_;
            new_y = current_y;
        } break;

        case RIGHT: {
            new_x = (current_x + 1) % num_cells_x_;
            new_y = current_y;
        } break;

        case DOWN: {
            new_x = current_x;
            new_y = ((current_y - 1) + num_cells_y_) % num_cells_y_;
        } break;

        case UP: {
            new_x = current_x;
            new_y = (current_y + 1) % num_cells_y_;
        } break;

        default: {
            std::cerr << "Invalid move direction " << direction << std::endl;
            assert(0);
        }
    }

    return std::string("Object ") + std::to_string(new_x + (new_y * num_cells_x_));
}

std::string PcsCell::random_move() {

    std::default_random_engine gen;
    std::uniform_int_distribution<unsigned int> rand_direction(0,3);
    return this->compute_move((direction_t)rand_direction(gen));
}

int main(int argc, const char **argv) {

    unsigned int num_cells_x        = 256;
    unsigned int num_cells_y        = 256;
    unsigned int call_interval_mean = 360;
    unsigned int call_duration_mean = 150;
    unsigned int channel_cnt        = 10;
    unsigned int num_portables      = 25;

    TCLAP::ValueArg<unsigned int> num_cells_x_arg("x", "num-cells-x", "Width of cell grid",
                                                            false, num_cells_x, "unsigned int");
    TCLAP::ValueArg<unsigned int> num_cells_y_arg("y", "num-cells-y", "Height of cell grid",
                                                            false, num_cells_y, "unsigned int");
    TCLAP::ValueArg<double> call_interval_mean_arg("i", "call-interval",
                                "Mean time between end of last call and a new call", 
                                                    false, call_interval_mean, "unsigned int");
    TCLAP::ValueArg<double> call_duration_mean_arg("d", "call-duration", "Mean call duration", 
                                                    false, call_duration_mean, "unsigned int");
    TCLAP::ValueArg<unsigned int> channel_cnt_arg("c", "channel-count",
                    "Number of communication channels", false, channel_cnt, "unsigned int");
    TCLAP::ValueArg<unsigned int> num_portables_arg("p", "portable-count", 
                            "Portables per cell", false, num_portables, "unsigned int");

    std::vector<TCLAP::Arg*> cmd_line_args = {&num_cells_x_arg, &num_cells_y_arg, 
                                                &call_interval_mean_arg, &call_duration_mean_arg, 
                                                &channel_cnt_arg, &num_portables_arg};

    warped::Simulation simulation {"PCS Simulation", argc, argv, cmd_line_args};

    num_cells_x         = num_cells_x_arg.getValue();
    num_cells_y         = num_cells_y_arg.getValue();
    call_interval_mean  = call_interval_mean_arg.getValue();
    call_duration_mean  = call_duration_mean_arg.getValue();
    channel_cnt         = channel_cnt_arg.getValue();
    num_portables       = num_portables_arg.getValue();

    std::vector<PcsCell> objects;
    for (unsigned int i = 0; i < num_cells_x * num_cells_y; i++) {
        std::string name = std::string("Object ") + std::to_string(i);
        objects.emplace_back(name, num_cells_x, num_cells_y, num_portables, 
                                channel_cnt, call_interval_mean, call_duration_mean, i);
    }

    std::vector<warped::SimulationObject*> object_pointers;
    for (auto& o : objects) {
        object_pointers.push_back(&o);
    }
    simulation.simulate(object_pointers);

    return 0;
}
