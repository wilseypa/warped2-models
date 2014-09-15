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
  unsigned int normal_channels_;
  unsigned int reserve_channels_;
  unsigned int call_attempts_;
  unsigned int channel_blocks_;
  unsigned int busy_lines_;
  unsigned int handoff_blocks_;
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
           channel_t channel, method_name_t method_name)
    : receiver_name_(receiver_name), completion_timestamp_(completion_timestamp),
      next_timestamp_(next_timestamp), move_timestamp_(move_timestamp), 
      channel_(channel), method_name_(method_name) {}

  const std::string& receiverName() const { return receiver_name_; }
  unsigned int timestamp() const;

  std::string receiver_name_;
  unsigned int completion_timestamp_;
  unsigned int next_timestamp_;
  unsigned int move_timestamp_;
  channel_t channel_;
  method_name_t method_name_;

  WARPED_REGISTER_SERIALIZABLE_MEMBERS(cereal::base_class<warped::Event>(this), 
                                        receiver_name_, completion_timestamp_, next_timestamp_, 
                                        move_timestamp_, channel_, method_name_)
};

class PcsCell : public warped::SimulationObject {
public:
  PcsCell(const std::string& name, const unsigned int num_cells_x,
          const unsigned int num_cells_y, const unsigned int num_portables,
          const unsigned int normal_channels, const unsigned int reserve_channels,
          const double call_time_mean, const double next_call_mean,
          const double move_call_mean, const unsigned int index)
    : SimulationObject(name), state_(), num_cells_x_(num_cells_x), num_cells_y_(num_cells_y),
      num_portables_(num_portables), call_time_mean_(call_time_mean),
      next_call_mean_(next_call_mean), move_call_mean_(move_call_mean), index_(index),
      rng_(new MLCG)
  {
    state_.normal_channels_ = normal_channels;
    state_.reserve_channels_ = reserve_channels;
  }

  double blocking_probability() {
    return ((double)(state_.channel_blocks_ + state_.handoff_blocks_)) / 
                ((double) (state_.call_attempts_ - state_.busy_lines_));
  }

  virtual std::vector<std::shared_ptr<warped::Event> > createInitialEvents();
  virtual std::vector<std::shared_ptr<warped::Event> > receiveEvent(const warped::Event&);

  virtual warped::ObjectState& getState() { return this->state_; }

  PcsState state_;

protected:
  const unsigned int num_cells_x_;
  const unsigned int num_cells_y_;
  const unsigned int num_portables_;
  const double call_time_mean_;
  const double next_call_mean_;
  const double move_call_mean_;

private:
  std::string compute_move(const direction_t) const;
  std::string random_move() const;
  const unsigned int index_;
  std::shared_ptr<MLCG> rng_;
};

#endif
