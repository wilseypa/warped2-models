// A customized implementation of volcano model

#include <cassert>
#include <random>
#include "volcano.hpp"
#include "tclap/ValueArg.h"

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(ParticleEvent)

std::vector<std::shared_ptr<warped::Event> > GridPosition::initializeLP() {

    // Register random number generator
    this->registerRNG<std::default_random_engine>(this->rng_);

    std::uniform_real_distribution<double> gamma_expo(0.0, this->max_gamma_);
    std::uniform_real_distribution<double> theta_expo(0.0, this->max_theta_);
    std::uniform_real_distribution<double>   eta_expo(0.0, this->max_eta_);
    std::exponential_distribution<double> interval_expo(1.0/this->mean_burst_interval_);

    std::vector<std::shared_ptr<warped::Event> > events;
    if (!this->index_) { // Only origin generates the particles
        for (unsigned int i = 0; i < this->num_particles_per_blast_; i++) {
            auto gamma = (double) gamma_expo(*this->rng_);
            auto theta = (double) theta_expo(*this->rng_);
            auto   eta = (double)   eta_expo(*this->rng_);

            double vel_x = this->mean_velocity_magnitude_ * 
                        (sin(theta) * cos(eta) * cos(gamma) + cos(theta) * sin(gamma));
            double vel_y = this->mean_velocity_magnitude_ * sin(theta) * sin(eta);
            double vel_z = this->mean_velocity_magnitude_ * 
                        (cos(theta) * cos(gamma) - sin(theta) * cos(eta) * sin(gamma));

            auto pos_x = (unsigned int) vel_x;
            auto pos_y = (unsigned int) vel_y;
            auto pos_z = (unsigned int) (vel_z - 4.905);
            if (    (pos_x >= this->max_grid_x_) || 
                    (pos_y >= this->max_grid_y_) || 
                    (pos_z >= this->max_grid_z_)   ) continue;

            auto index = (pos_x * this->max_grid_y_ + pos_y) * this->max_grid_z_ + pos_z;
            if (!index) continue; // Particles rejected if at origin after time interval increment
            events.emplace_back(new ParticleEvent {
                    lpName(index), ENTER_PARTICLE, vel_x, vel_y, vel_z, 0, 1});
        }

        // Generate the next particle burst
        unsigned int interval = (unsigned int) std::ceil(interval_expo(*this->rng_));
        events.emplace_back(new ParticleEvent {lpName(0), NEXT_BURST, 0, 0, 0, 0, interval});

    } else { // Update the trajectories of particles in the grid
        events.emplace_back(
                new ParticleEvent {lpName(this->index_), EXIT_PARTICLES, 0, 0, 0, 0, 1});
    }
    return events;
}

inline std::string GridPosition::lpName(const unsigned int lp_index) {

    return std::string("Grid_") + std::to_string(lp_index);
}

std::vector<std::shared_ptr<warped::Event> > 
        GridPosition::receiveEvent(const warped::Event& event) {

    std::vector<std::shared_ptr<warped::Event> > events;
    auto received_event = static_cast<const ParticleEvent&>(event);
    auto ts = received_event.ts_;

    std::uniform_real_distribution<double> gamma_expo(0.0, this->max_gamma_);
    std::uniform_real_distribution<double> theta_expo(0.0, this->max_theta_);
    std::uniform_real_distribution<double>   eta_expo(0.0, this->max_eta_);
    std::exponential_distribution<double> interval_expo(1.0/this->mean_burst_interval_);

    switch (received_event.type_) {

        case ENTER_PARTICLE: {
            state_.particle_cnt_++;
            state_.particle_details_ += std::to_string(received_event.origin_time_) + "," + 
                                        std::to_string(received_event.vel_x_) + "," + 
                                        std::to_string(received_event.vel_y_) + "," + 
                                        std::to_string(received_event.vel_z_) + ";";
        } break;

        case EXIT_PARTICLES: {
            std::string *details = new std::string [state_.particle_cnt_];
            std::size_t start_pos = 0;
            std::size_t found = state_.particle_details_.find(";");
            unsigned int i = 0;
            while (found != std::string::npos) {
                details[i++] = state_.particle_details_.substr (start_pos, found-start_pos);
                start_pos = found+1;
                found = state_.particle_details_.find(";", start_pos);
            }

            // Update the trajectories of particles in the grid
            for (i = 0; i < state_.particle_cnt_; i++) {
                unsigned int origin_time = 0;
                double vel_x = 0.0, vel_y = 0.0, vel_z = 0.0;
                sscanf(details[i].c_str(), "%u,%lf,%lf,%lf", 
                                    &origin_time, &vel_x, &vel_y, &vel_z);

                auto delta_t = ts + 1 - origin_time;
                auto pos_x = (unsigned int) (vel_x * delta_t);
                auto pos_y = (unsigned int) (vel_y * delta_t);
                auto pos_z = (unsigned int) ((vel_z - 4.905 * delta_t) * delta_t);
                if (    (pos_x >= this->max_grid_x_) || 
                        (pos_y >= this->max_grid_y_) || 
                        (pos_z >= this->max_grid_z_)   ) continue;

                auto index = (pos_x * this->max_grid_y_ + pos_y) * this->max_grid_z_ + pos_z;
                events.emplace_back(new ParticleEvent {
                    lpName(index), ENTER_PARTICLE, vel_x, vel_y, vel_z, origin_time, ts + 1});
            }
            delete[] details;
            state_.particle_cnt_ = 0;
            state_.particle_details_ = "";

            events.emplace_back(
                new ParticleEvent {lpName(this->index_), EXIT_PARTICLES, 0, 0, 0, 0, ts + 1});

        } break;

        case NEXT_BURST: {
            for (unsigned int i = 0; i < this->num_particles_per_blast_; i++) {
                auto gamma = (double) gamma_expo(*this->rng_);
                auto theta = (double) theta_expo(*this->rng_);
                auto   eta = (double)   eta_expo(*this->rng_);

                double vel_x = this->mean_velocity_magnitude_ * 
                        (sin(theta) * cos(eta) * cos(gamma) + cos(theta) * sin(gamma));
                double vel_y = this->mean_velocity_magnitude_ * sin(theta) * sin(eta);
                double vel_z = this->mean_velocity_magnitude_ * 
                        (cos(theta) * cos(gamma) - sin(theta) * cos(eta) * sin(gamma));

                auto pos_x = (unsigned int) vel_x;
                auto pos_y = (unsigned int) vel_y;
                auto pos_z = (unsigned int) (vel_z - 4.905);
                if (    (pos_x >= this->max_grid_x_) || 
                        (pos_y >= this->max_grid_y_) || 
                        (pos_z >= this->max_grid_z_)   ) continue;

                auto index = (pos_x * this->max_grid_y_ + pos_y) * this->max_grid_z_ + pos_z;
                if (!index) continue; // Particles rejected if at origin after interval increment
                events.emplace_back(new ParticleEvent {lpName(index), 
                                    ENTER_PARTICLE, vel_x, vel_y, vel_z, ts, ts + 1});
            }

            // Generate the next particle burst
            unsigned int interval = ts + (unsigned int) std::ceil(interval_expo(*this->rng_));
            events.emplace_back(new ParticleEvent {lpName(0), NEXT_BURST, 0, 0, 0, 0, interval});

        } break;
    }
    return events;
}

int main(int argc, const char** argv) {

    unsigned int grid_dimension_x           = 50;
    unsigned int grid_dimension_y           = 50;
    unsigned int grid_dimension_z           = 50;
    unsigned int num_particles_per_blast    = 50000;
    unsigned int mean_burst_interval        = 10;
    double       max_gamma                  = 5;
    double       max_theta                  = 5;
    double       max_eta                    = 5;
    unsigned int mean_velocity_magnitude    = 50;

    TCLAP::ValueArg<unsigned int> grid_dimension_x_arg("x", "grid-dimension-x", 
                "grid dimension on x-axis", false, grid_dimension_x, "unsigned int");
    TCLAP::ValueArg<unsigned int> grid_dimension_y_arg("y", "grid-dimension-y", 
                "grid dimension on y-axis", false, grid_dimension_y, "unsigned int");
    TCLAP::ValueArg<unsigned int> grid_dimension_z_arg("z", "grid-dimension-z", 
                "grid dimension on z-axis", false, grid_dimension_z, "unsigned int");
    TCLAP::ValueArg<unsigned int> num_particles_per_blast_arg("n", "num-particles-per-blast", 
                "number of particles released per blast", false, 
                num_particles_per_blast, "unsigned int");
    TCLAP::ValueArg<unsigned int> mean_burst_interval_arg("i", "mean-burst-interval", 
                "mean burst interval", false, mean_burst_interval, "unsigned int");
    TCLAP::ValueArg<double> max_gamma_arg("g", "max-gamma", "max gamma", false, max_gamma, "double");
    TCLAP::ValueArg<double> max_theta_arg("t", "max-theta", "max gamma", false, max_theta, "double");
    TCLAP::ValueArg<double> max_eta_arg("e", "max-eta", "max eta", false, max_eta, "double");
    TCLAP::ValueArg<unsigned int> mean_velocity_magnitude_arg("v", "mean-velocity-magnitude", 
                "mean velocity magnitude", false, mean_velocity_magnitude, "unsigned int");

    std::vector<TCLAP::Arg*> cmd_line_args = {  &grid_dimension_x_arg, 
                                                &grid_dimension_y_arg, 
                                                &grid_dimension_z_arg, 
                                                &num_particles_per_blast_arg, 
                                                &mean_burst_interval_arg,
                                                &max_gamma_arg,
                                                &max_theta_arg,
                                                &max_eta_arg,
                                                &mean_velocity_magnitude_arg
                                             };

    warped::Simulation simulation {"Volcano Simulation", argc, argv, cmd_line_args};

    grid_dimension_x        = grid_dimension_x_arg.getValue();
    grid_dimension_y        = grid_dimension_y_arg.getValue();
    grid_dimension_z        = grid_dimension_z_arg.getValue();
    num_particles_per_blast = num_particles_per_blast_arg.getValue();
    mean_burst_interval     = mean_burst_interval_arg.getValue();
    max_gamma               = max_gamma_arg.getValue();
    max_theta               = max_theta_arg.getValue();
    max_eta                 = max_eta_arg.getValue();
    mean_velocity_magnitude = mean_velocity_magnitude_arg.getValue();

    std::vector<GridPosition> lps;
    unsigned int index = 0;
    for (unsigned int x = 0; x < grid_dimension_x; x++) {
        for (unsigned int y = 0; y < grid_dimension_y; y++) {
            for (unsigned int z = 0; z < grid_dimension_z; z++) {
                lps.emplace_back(   grid_dimension_x, 
                                    grid_dimension_y, 
                                    grid_dimension_z, 
                                    num_particles_per_blast, 
                                    mean_burst_interval, 
                                    max_gamma, 
                                    max_theta, 
                                    max_eta, 
                                    mean_velocity_magnitude, 
                                    index
                                );
                index++;
            }
        }
    }

    std::vector<warped::LogicalProcess*> lp_pointers;
    for (auto& lp : lps) {
        lp_pointers.push_back(&lp);
    }
    simulation.simulate(lp_pointers);

    return 0;
}

