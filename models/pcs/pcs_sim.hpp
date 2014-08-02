// This model was ported from the ROSS pcs model. See https://github.com/carothersc/ROSS
// Also see "Distributed Simulation of Large-Scale PCS Networks" by Carothers et al.

#ifndef PCS_SIM_HPP_DEFINED
#define PCS_SIM_HPP_DEFINED

#include <string>
#include <vector>
#include <memory>

#include "warped.hpp"

#include "MLCG.h"
#include "NegExp.h"

// These may be moved to config file or command line later
#define NUM_CELLS_X 1024     //256
#define NUM_CELLS_Y 1024     //256

#define NUM_VP_X 512 
#define NUM_VP_Y 512

#define MAX_NORMAL_CHANNELS 10
#define MAX_RESERVE_CHANNELS 0

/*
 * This is in Mins 
 */
#define MOVE_CALL_MEAN 4500.0
#define NEXT_CALL_MEAN 360.0
#define CALL_TIME_MEAN 180.0

/*
 * When Normal_Channels == 0, then all have been used 
 */
#define NORM_CH_BUSY ( !( SV->Normal_Channels & 0xffffffff ) )

/*
 * When Reserve_Channels == 0, then all have been used 
 */
#define RESERVE_CH_BUSY ( !( SV->Reserve_Channels & 0xffffffff ) )

WARPED_DEFINE_OBJECT_STATE_STRUCT(PcsState) {
  double          const_state_1;
  int             const_state_2;
  // int             normal_channels;
  // int             reserve_channels;
  int             portables_in;
  int             portables_out;
  int             call_attempts;
  int             channel_blocks;
  int             busy_lines;
  int             handoff_blocks;
  int             cell_location_x;
  int             cell_location_y;
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

enum min_t {
  COMPLETECALL,
  NEXTCALL,
  MOVECALL
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
  PcsCell(const std::string& name, const unsigned int num_cells, const unsigned int num_portables,
          const unsigned int normal_channels, const unsigned int reserve_channels,
          const unsigned int index)
    : SimulationObject(name), state(), num_cells(num_cells), num_portables(num_portables),
      normal_channels(normal_channels), reserve_channels(reserve_channels),
      // portables_in(0), portables_out(0), call_attempts(0), channel_blocks(0), busy_lines(0),
      // handoff_blocks(0), call_attempts(0)
      rng(new MLCG), index(index) { }

  double blocking_probability() {
    return ((double)(state.channel_blocks + state.handoff_blocks)) / ((double) (state.call_attempts - state.busy_lines));
  }

  virtual std::vector<std::unique_ptr<warped::Event> > createInitialEvents();
  virtual std::vector<std::unique_ptr<warped::Event> > receiveEvent(const warped::Event&);

  virtual warped::ObjectState& getState() { return this->state; }

  PcsState state;

protected:
  unsigned int num_cells;
  unsigned int num_portables;
  unsigned int normal_channels;
  unsigned int reserve_channels;
  std::unique_ptr<MLCG> rng;

private:
  std::string compute_move(const direction_t) const;
  std::string random_move() const;
  unsigned int index;
};

#endif
