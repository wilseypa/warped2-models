// A customized implementation of volcano model

#ifndef VOLCANO_HPP_DEFINED
#define VOLCANO_HPP_DEFINED

#include <string>
#include <vector>
#include <memory>
#include <random>

#include "warped.hpp"

WARPED_DEFINE_LP_STATE_STRUCT(VolcanoState) {

    // Velocity vector - V1_x,V1_y,V1_z;V2_x,V2_y,V2_z,...
    // There can be multiple particles in an LP at the same instant
    // Note: We assume there are no particle collisions in this model
    std::string particle_velocity_;
};

enum volcano_event_t {

    ARRIVAL,
    DEPARTURE,
    DIRECTION_SELECT
};

class VolcanoEvent : public warped::Event {
public:
    VolcanoEvent() = default;
    VolcanoEvent(   const std::string& receiver_name, 
                    const volcano_event_t type, 
                    const unsigned int vel_x, 
                    const unsigned int vel_y, 
                    const unsigned int vel_z, 
                    const unsigned int timestamp    )
        :   receiver_name_(receiver_name), type_(type), 
            vel_x_(vel_x), vel_y_(vel_y), vel_z_(vel_z), 
            ts_(timestamp) {}

    const std::string& receiverName() const { return receiver_name_; }
    unsigned int timestamp() const { return ts_; }

    std::string receiver_name_;
    volcano_event_t type_;
    unsigned int vel_x_;
    unsigned int vel_y_;
    unsigned int vel_z_;
    unsigned int ts_;

    WARPED_REGISTER_SERIALIZABLE_MEMBERS(cereal::base_class<warped::Event>(this), 
                                receiver_name_, type_, vel_x_, vel_y_, vel_z_, ts_)
};

class Volcano : public warped::LogicalProcess {
public:
    Volcano(const std::string& name, const unsigned int index)
        :   LogicalProcess(name), 
            state_(), 
            rng_(new std::default_random_engine(index)),
            index_(index) {}

    virtual std::vector<std::shared_ptr<warped::Event> > initializeLP() override;
    virtual std::vector<std::shared_ptr<warped::Event> > receiveEvent(const warped::Event&);
    virtual warped::LPState& getState() { return this->state_; }

    VolcanoState state_;

    static inline std::string lp_name(const unsigned int);

protected:
    std::shared_ptr<std::default_random_engine> rng_;
    const unsigned int index_;
    const unsigned int position_x_;
    const unsigned int position_y_;
    const unsigned int position_z_;
};

#endif
