// A customized implementation of volcano model on a 3D grid

#ifndef VOLCANO_HPP_DEFINED
#define VOLCANO_HPP_DEFINED

#include <string>
#include <vector>
#include <memory>
#include <random>

#include "warped.hpp"

WARPED_DEFINE_LP_STATE_STRUCT(GridPositionState) {

    unsigned int particle_cnt_;

    // Paricle details - <origin_time>,<vel_x>,<vel_y>,<vel_z>;....
    // There can be multiple particles in an LP at the same instant
    // Note: We assume there are no particle collisions in this model
    std::string particle_details_;
};

enum particle_event_t {

    ENTER_PARTICLE,
    EXIT_PARTICLES,
    NEXT_BURST
};

class ParticleEvent : public warped::Event {
public:
    ParticleEvent() = default;
    ParticleEvent(  const std::string& receiver_name, 
                    const particle_event_t type, 
                    const double vel_x, 
                    const double vel_y, 
                    const double vel_z, 
                    const unsigned int origin_time, 
                    const unsigned int timestamp    )
        :   receiver_name_(receiver_name), 
            type_(type), 
            vel_x_(vel_x), 
            vel_y_(vel_y), 
            vel_z_(vel_z), 
            origin_time_(origin_time), 
            ts_(timestamp) {}

    const std::string& receiverName() const { return receiver_name_; }
    unsigned int timestamp() const { return ts_; }

    unsigned int size() const {
        return  receiver_name_.length() +
                sizeof(type_) +
                sizeof(vel_x_) +
                sizeof(vel_y_) +
                sizeof(vel_z_) +
                sizeof(origin_time_) +
                sizeof(ts_);
    }

    std::string receiver_name_;
    particle_event_t type_;
    double vel_x_;
    double vel_y_;
    double vel_z_;
    unsigned int origin_time_;
    unsigned int ts_;

    WARPED_REGISTER_SERIALIZABLE_MEMBERS(cereal::base_class<warped::Event>(this), 
                        receiver_name_, type_, vel_x_, vel_y_, vel_z_, origin_time_, ts_)
};

class GridPosition : public warped::LogicalProcess {
public:
    GridPosition(   const unsigned int max_grid_x,
                    const unsigned int max_grid_y,
                    const unsigned int max_grid_z,
                    const unsigned int num_particles_per_blast,
                    const unsigned int mean_burst_interval,
                    const double       max_gamma,
                    const double       max_theta,
                    const double       max_eta,
                    const unsigned int mean_velocity_magnitude,
                    const unsigned int index    )
            :   LogicalProcess(lpName(index)), 
                state_(), 
                rng_(new std::default_random_engine(index)), 
                max_grid_x_(max_grid_x), 
                max_grid_y_(max_grid_y), 
                max_grid_z_(max_grid_z), 
                num_particles_per_blast_(num_particles_per_blast), 
                mean_burst_interval_(mean_burst_interval),
                max_gamma_(max_gamma),
                max_theta_(max_theta),
                max_eta_(max_eta),
                mean_velocity_magnitude_(mean_velocity_magnitude),
                index_(index) {

        state_.particle_cnt_ = 0;
        state_.particle_details_ = "";
    }

    virtual std::vector<std::shared_ptr<warped::Event> > initializeLP() override;
    virtual std::vector<std::shared_ptr<warped::Event> > receiveEvent(const warped::Event&);
    virtual warped::LPState& getState() { return this->state_; }

    GridPositionState state_;

    static inline std::string lpName(const unsigned int);

protected:
    std::shared_ptr<std::default_random_engine> rng_;
    const unsigned int max_grid_x_;
    const unsigned int max_grid_y_;
    const unsigned int max_grid_z_;
    const unsigned int num_particles_per_blast_;
    const unsigned int mean_burst_interval_;
    const double       max_gamma_;
    const double       max_theta_;
    const double       max_eta_;
    const unsigned int mean_velocity_magnitude_;
    const unsigned int index_;
};

#endif
