// This model was ported from the ROSS pcs model. See https://github.com/carothersc/ROSS

#include "warped.hpp"

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

#define BIG_N (double)16.0

/*
 * When Normal_Channels == 0, then all have been used 
 */
#define NORM_CH_BUSY ( !( SV->Normal_Channels & 0xffffffff ) )

/*
 * When Reserve_Channels == 0, then all have been used 
 */
#define RESERVE_CH_BUSY ( !( SV->Reserve_Channels & 0xffffffff ) )

WARPED_DEFINE_OBJECT_STATE_STRUCT(PCSState) {
  double          const_state_1;
  int             const_state_2;
  int             normal_channels;
  int             reserve_channels;
  int             portables_in;
  int             portables_out;
  int             call_attempts;
  int             channel_blocks;
  int             busy_lines;
  int             handoff_blocks;
  int             cell_location_x;
  int             cell_location_y;
};

class PCSEvent : public warped::Event {
public:
  PCSEvent() = default;
  PCSEvent(const PCSState& state, 
