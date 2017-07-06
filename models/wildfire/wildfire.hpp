//Implementation of a WildFire Simulation
#ifndef WILDFIRE_HPP_DEFINED
#define WILDFIRE_HPP_DEFINED

#include <string>
#include <vector>

#include "warped.hpp"


/* Status of the fire */
enum cell_status_t {

    UNBURNT,
    BURNING,
    BURNT_OUT
};

WARPED_DEFINE_LP_STATE_STRUCT(CellState) {

    cell_status_t   burn_status_;
    unsigned int    heat_content_;
};

/*Wildfire events */
enum cell_event_t {

    RADIATION_TIMER,
    RADIATION,
    IGNITION,
    PEAK  
};

/*The direction of the spread of fire from the currently burning LP*/
enum direction_t {  

    NORTH,
    NORTH_EAST,
    EAST,
    SOUTH_EAST,
    SOUTH,
    SOUTH_WEST,
    WEST,
    NORTH_WEST,
    DIRECTION_MAX
};

/*! Class Definition of a Wildfire event */
class CellEvent : public warped::Event {
public:
    CellEvent() = default;
    CellEvent(  const std::string& receiver_name,
                const cell_event_t type,
                const unsigned int heat_content,
                const unsigned int timestamp    )
        :   receiver_name_(receiver_name),
            type_(type),
            heat_content_(heat_content),
            ts_(timestamp) {}

    const std::string& receiverName() const { return receiver_name_; }
    unsigned int timestamp() const { return ts_; }

    std::string receiver_name_; /*! Name of the LP that is recieving the event */
    cell_event_t type_; /*! The type of event being sent */
    unsigned int heat_content_; /*! Heat being sent in the event */
    unsigned int ts_; /*! Timestamp used by the kernel that identifies when the event takes place */

    WARPED_REGISTER_SERIALIZABLE_MEMBERS(cereal::base_class<warped::Event>(this), 
                                            receiver_name_, type_, heat_content_, ts_)
};

/*! Class that will hold all of the attributes of the LP's */
class Cell : public warped::LogicalProcess{
public:
    Cell(   const unsigned int num_rows,
            const unsigned int num_cols,
            unsigned char      **combustible_map,
            const unsigned int ignition_threshold,
            const unsigned int growth_rate,
            const unsigned int peak_threshold,
            const double       radiation_fraction,
            const unsigned int burnout_threshold,
            const unsigned int heat_content,
            const unsigned int index            )

        :   LogicalProcess(lp_name(index)),
            state_(),
            num_rows_(num_rows),
            num_cols_(num_cols),
            ignition_threshold_(ignition_threshold),
            growth_rate_(growth_rate),
            peak_threshold_(peak_threshold),
            radiation_fraction_(radiation_fraction),
            burnout_threshold_(burnout_threshold),
            index_(index) {

        /* Initialize the state variables */
        state_.burn_status_     = UNBURNT;
        state_.heat_content_    = heat_content;

        /* Populate the connection_matrix */
        for (unsigned int direction = NORTH; direction < DIRECTION_MAX; direction++) {
            connection_[direction] = neighbor_conn( (direction_t)direction, combustible_map );
        }
    }

    /*! Function to start the fire by comparing the heat content of the LP with
      that LP's ignition threshold */
    virtual std::vector<std::shared_ptr<warped::Event> > initializeLP() override;

    /*! Function that will handle all events being sent to and from LP's */
    virtual std::vector<std::shared_ptr<warped::Event> > receiveEvent(const warped::Event&);

    /*! Returns the state of Burn that the LP in */
    virtual warped::LPState& getState() { return this->state_; }

    CellState state_;
    /*! Simple function that returns the name of an LP using th index*/
    static inline std::string lp_name( const unsigned int );

    const unsigned int  num_rows_;                  /*! Number of rows in the vegetation grid */
    const unsigned int  num_cols_;                  /*! Number of columns in the vegetation grid*/
    const unsigned int  ignition_threshold_;        /*! Minimum  heat content needed to ignite an LP*/
    const unsigned int  growth_rate_;               /*! Speed at which the fire grows in an LP */
    const unsigned int  peak_threshold_;            /*! Maximum heat content threshold of an LP*/
    const double        radiation_fraction_;        /*! Percentage of heat  radiated out by a burning LP*/
    const unsigned int  burnout_threshold_;         /*! Heat content threshold for a burnt out LP*/
    bool                connection_[DIRECTION_MAX]; /*! Boolean that returns true when an LP exists in adjacent node*/
    const unsigned int  index_;                     /*! Unique LP number that is used by the kernel to find an certain LP */

    std::string find_cell( direction_t direction ); /*! Function to find an adjacent cell in a certain direction */

    /*! Function to find whether connection exits to a neighbor in a certain given direction */
    bool neighbor_conn( direction_t direction, unsigned char **combustible_map );
};

#endif
