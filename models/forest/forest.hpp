//Implementation of a Forest Fire Simulation
#ifndef FOREST_HPP_DEFINED
#define FOREST_HPP_DEFINED

#include <string>
#include <vector>
#include <memory>
#include <random>

#include "warped.hpp"


/* Status of the fire */
enum status_t {

    UNBURNT,
    GROWTH,
    DECAY,
    BURNT_OUT
};

WARPED_DEFINE_LP_STATE_STRUCT(ForestState) {

    status_t burn_status;
    unsigned int heat_content_;
};

/*Forest fire events */
enum forest_event_t {

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
    NORTH_WEST
};

/* Class Definition of a forest fire event */
class ForestEvent : public warped::Event {
public:
    ForestEvent() = default;
    ForestEvent(const std::string& receiver_name,
                const forest_event_t type,
                const unsigned int heat_content,
                const unsigned int timestamp)
        : receiver_name_(receiver_name), type_(type), ts_(timestamp) {}
        
    const std::string& receiverName() const { return receiver_name_; }
    unsigned int timestamp() const { return ts_; }

    std::string receiver_name_;
    forest_event_t type_;
    unsigned int heat_content_;
    unsigned int ts_;

    WARPED_REGISTER_SERIALIZABLE_MEMBERS(cereal::base_class<warped::Event>(this), 
                                            receiver_name_, type_, heat_content_, ts_)
};

class Forest : public warped::LogicalProcess{
public:
    Forest( const std::string& name,
            const unsigned int size_x,    
            const unsigned int size_y,    
            const unsigned int ignition_threshold,
            const unsigned int heat_rate,
            const unsigned int peak_threshold,
            const unsigned int radiation_fraction,
            const unsigned int burnout_threshold,
            const unsigned int index    )
                
    :   LogicalProcess(name),
        state_(),
        rng_(new std::default_random_engine(index)),
        size_x_(size_x),    
        size_y_(size_y),   
        ignition_threshold_(ignition_threshold),
        heat_rate_(heat_rate),
        peak_threshold_(peak_threshold),
        radiation_fraction_(radiation_fraction),
        burnout_threshold_(burnout_threshold),
        index_(index) {

        /* Initialize the state variables */
        state_.burning_status_ = UNBURNT;
        state_.heat_content_ = 0;
    }
            
    virtual std::vector<std::shared_ptr<warped::Event> > initializeLP() override;
    virtual std::vector<std::shared_ptr<warped::Event> > receiveEvent(const warped::Event&);
    virtual warped::LPState& getState() { return this->state_; }
            
    ForestState state_;
    static inline std::string lp_name(const unsigned int);

protected:
    std::shared_ptr<std::default_random_engine> rng_;
    const unsigned int size_x_;   //The width of the picture being used
    const unsigned int size_y_;    //The height of the picture being used
    const unsigned int ignition_threshold_;// threshold of heat that the LP needs to ignite
    const unsigned int heat_rate_; // The speed at which the fire is burning
    const unsigned int peak_threshold_; //threshold of heat reached by the fire before it stops growing
    const unsigned int radiation_fraction_; //Percent of heat radiation released at each event
    const unsigned int burnout_threshold_; //Threshold of Heat that the lp needs to reach to burn out
    const unsigned int index_; //The identifier used by the model to distinguish between LPs
 
    std::string compute_spread(); //Function to Spread the heat to adjacent cells
}

#endif
