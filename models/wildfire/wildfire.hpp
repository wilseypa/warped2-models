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

/* Class Definition of a Wildfire event */
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

    std::string receiver_name_;
    cell_event_t type_;
    unsigned int heat_content_;
    unsigned int ts_;

    WARPED_REGISTER_SERIALIZABLE_MEMBERS(cereal::base_class<warped::Event>(this), 
                                            receiver_name_, type_, heat_content_, ts_)
};

class Cell : public warped::LogicalProcess{
public:
    Cell( const unsigned int size_x,
            const unsigned int size_y,
            unsigned char **combustible_map,
            const unsigned int ignition_threshold,
            const unsigned int heat_rate,
            const unsigned int peak_threshold,
            const unsigned int radiation_fraction,
            const unsigned int burnout_threshold,
            const unsigned int heat_content,
            const unsigned int index    )

        :   LogicalProcess(lp_name(index)),
            state_(),
            size_x_(size_x),
            size_y_(size_y),
            ignition_threshold_(ignition_threshold),
            heat_rate_(heat_rate),
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

    virtual std::vector<std::shared_ptr<warped::Event> > initializeLP() override;
    virtual std::vector<std::shared_ptr<warped::Event> > receiveEvent(const warped::Event&);
    virtual warped::LPState& getState() { return this->state_; }

    CellState state_;
    static inline std::string lp_name( const unsigned int );

    const unsigned int  size_x_;                    // Width of the vegetation grid
    const unsigned int  size_y_;                    // Height of the vegetation grid
    const unsigned int  ignition_threshold_;        // Min heat content needed to ignite an LP
    const unsigned int  heat_rate_;                 // Speed at which the fire grows in an LP
    const unsigned int  peak_threshold_;            // Max heat content threshold of an LP
    const unsigned int  radiation_fraction_;        // Heat fraction radiated out by a burning LP
    const unsigned int  burnout_threshold_;         // Heat content threshold for a burnt out LP
    bool                connection_[DIRECTION_MAX]; // True when LP exists in adjacent node
    const unsigned int  index_;                     // Unique LP ID

    std::string find_cell( direction_t direction ); // Find adjacent cell in a certain direction

    // Find whether connection exits to a neighbor
    bool neighbor_conn( direction_t direction, unsigned char **combustible_map );
};

#endif
