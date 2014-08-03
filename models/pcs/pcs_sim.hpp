// This model was ported from the ROSS pcs model. See https://github.com/carothersc/ROSS
// Also see "Distributed Simulation of Large-Scale PCS Networks" by Carothers et al.
// Author: Eric Carver carverer@mail.uc.edu

#ifndef PCS_SIM_HPP_DEFINED
#define PCS_SIM_HPP_DEFINED

#include <string>
#include <vector>
#include <memory>

#include "warped.hpp"

#include "MLCG.h"
#include "NegExp.h"

WARPED_DEFINE_OBJECT_STATE_STRUCT(PcsState) {
  unsigned int normal_channels;
  unsigned int reserve_channels;
  unsigned int call_attempts;
  unsigned int channel_blocks;
  unsigned int busy_lines;
  unsigned int handoff_blocks;
};

enum method_name_t {
  NEXTCALL_METHOD,
  COMPLETIONCALL_METHOD,
  MOVECALLIN_METHOD,
  MOVECALLOUT_METHOD
};

enum channel_t {
  NONE,
  NORMAL,
  RESERVE
};

enum direction_t {
  LEFT,
  RIGHT,
  DOWN,
  UP
};

class PcsEvent : public warped::Event {
public:
  PcsEvent() = default;
  PcsEvent(const std::string receiver_name, const unsigned int completion_timestamp,
           const unsigned int next_timestamp, const unsigned int move_timestamp,
           const std::string creator_name, channel_t channel, method_name_t method_name)
    : receiver_name(receiver_name), creator_name(creator_name),
      completion_timestamp(completion_timestamp),
      next_timestamp(next_timestamp), move_timestamp(move_timestamp), 
      channel(channel), method_name(method_name) {}

  const std::string& receiverName() const { return receiver_name; }
  unsigned int timestamp() const;

  std::string receiver_name;
  std::string creator_name;
  unsigned int completion_timestamp;
  unsigned int next_timestamp;
  unsigned int move_timestamp;
  channel_t channel;
  method_name_t method_name;

  WARPED_REGISTER_SERIALIZABLE_MEMBERS(creator_name, receiver_name, completion_timestamp,
                                       next_timestamp, move_timestamp, channel, method_name)
};

class PcsCell : public warped::SimulationObject {
public:
  PcsCell(const std::string& name, const unsigned int num_cells_x,
          const unsigned int num_cells_y, const unsigned int num_portables,
          const unsigned int normal_channels, const unsigned int reserve_channels,
          const double call_time_mean, const double next_call_mean,
          const double move_call_mean, const unsigned int index)
    : SimulationObject(name), state(), num_cells_x(num_cells_x), num_cells_y(num_cells_y),
      num_portables(num_portables), call_time_mean(call_time_mean),
      next_call_mean(next_call_mean), move_call_mean(move_call_mean), index(index),
      rng(new MLCG)
  {
    state.normal_channels = normal_channels;
    state.reserve_channels = reserve_channels;
  }

  double blocking_probability() {
    return ((double)(state.channel_blocks + state.handoff_blocks)) / ((double) (state.call_attempts - state.busy_lines));
  }

  virtual std::vector<std::unique_ptr<warped::Event> > createInitialEvents();
  virtual std::vector<std::unique_ptr<warped::Event> > receiveEvent(const warped::Event&);

  virtual warped::ObjectState& getState() { return this->state; }

  PcsState state;

protected:
  const unsigned int num_cells_x;
  const unsigned int num_cells_y;
  const unsigned int num_portables;
  const double call_time_mean;
  const double next_call_mean;
  const double move_call_mean;

private:
  std::string compute_move(const direction_t) const;
  std::string random_move() const;
  const unsigned int index;
  std::unique_ptr<MLCG> rng;
};

#endif
