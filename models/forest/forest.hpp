//Implementation of a Forest Fire Simulation
#ifndef FOREST_HPP_DEFINED
#define FOREST_HPP_DEFINED

#include <string>
#include <vector>
#include <memory>
#include <random>

#include "warped.hpp"


WARPED_DEFINE_LP_STATE_STRUCT(ForestState) {
   enum burning_status_{
      UNBURNT,
      GROWTH,
      DECAY,
      BURNT_OUT
   }; 

   unsigned int heat_content_;
};

enum forest_event_t {
        RADIATION_IN,
        RADIATION_OUT,
        IGNITION,
        PEAK  
};

enum direction_t {  //The direction of the spread of fire from the currently burning LP

    NORTH,
    NORTH_WEST,
    NORTH_EAST,
    SOUTH,
    SOUTH_WEST,
    SOUTH_EAST,
    EAST,
    WEST,
};

class ForestEvent : public warped::Event {  //Class Definition of a forest fire event
    public:
        ForestEvent() = default;
        ForestEvent(const std::string& receiver_name, const forest_event_t type,
                                                        const unsigned int timestamp)
            : receiver_name_(receiver_name), type_(type), ts_(timestamp) {} //Initializing Class Components
        
        const std::string& receiverName() const { return receiver_name_; }
        unsigned int timestamp() const { return ts_; }

        std::string receiver_name_;
        forest_event_t type_;
        unsigned int ts_;

        WARPED_REGISTER_SERIALIZABLE_MEMBERS(cereal::base_class<warped::Event>(this), receiver_name_, type_, ts_)

};

class Forest : public warped::LogicalProcess{
    public:
        Forest( const std::string& name,
                const unsigned int size_x,    
                const unsigned int size_y,    
                const unsigned int ignition_threshold,
                const unsigned int heat_rate,
                const unsigned int peak_threshold,
                const unsigned int radiation_percent,
                const unsigned int burnout_threshold,
                const unsigned int index)
                
        :   LogicalProcess(name),
            state_(),
            rng_(new std::default_random_engine(index)),
            size_x_(size_x),    
            size_y_(size_y),   
            ignition_threshold_(ignition_threshold),
            peak_threshold_(peak_threshold),
            radiation_percent_(radiation_percent),
            burnout_threshold_(burnout_threshold),
            index_(index){
                state_.burning_status_ = UNBURNT;
                heat_content_ = 0;
            }
            
            virtual std::vector<std::shared_ptr<warped::Event> > initializeLP() override;
            virtual std::vector<std::shared_ptr<warped::Event> > receiveEvent(const warped::Event&);
            virtual warped::LPState& getState() { return this->state_; }
            
            ForestState state_;
            
            static inline std::string lp_name(const unsigned int);
            
            
            
            
    protected:
        std::shared_ptr<std::default_random_engine> rng_;
        const unsigned int size_x_;   //the x coordinate of the LP on the picture
        const unsigned int size_y_;    //the y coordinate of the LP on the picture
        const unsigned int ignition_threshold_; // threshold of heat that the LP needs to ignite
        const unsigned int peak_threshold_; //threshold of heat reached by the fire before it stops growing
        const unsigned int radiation_percent_; //Percent of heat radiation released at each event
        const unsigned int burnout_threshold_; //Threshold of Heat that the lp needs to reach to burn out
        const unsigned int index_; //The identifier used by the model to distinguish between LPs
        
        std::string compute_move(direction_t direction);

}

#endif
