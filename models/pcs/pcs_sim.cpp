// See "Distributed Simulation of Large-Scale PCS Networks" by Carothers et al.

#include <vector>
#include <memory>
#include <cassert>
#include <algorithm>
#include <random>
#include <cstdlib>

#include "warped.hpp"
#include "pcs_sim.hpp"

#include "tclap/ValueArg.h"

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(PcsEvent)

std::vector<std::shared_ptr<warped::Event> > PcsCell::initializeObject() {

    // Register random number generator to allow kernel to roll it back
    this->registerRNG<std::default_random_engine>(this->rng_);

    std::exponential_distribution<double> interval_expo(call_interval_mean_);
    std::vector<std::shared_ptr<warped::Event>> events;
    for (unsigned int i = 0; i < portable_init_cnt_; i++) {
        auto interval = (unsigned int) std::ceil(interval_expo(*this->rng_));
        events.emplace_back(new PcsEvent {this->name_, interval, 0, CALL_ARRIVED, NORMAL});
    }
    return events;
}

std::vector<std::shared_ptr<warped::Event> > PcsCell::receiveEvent(const warped::Event& event) {

    std::vector<std::shared_ptr<warped::Event>> events;
    auto pcs_event = static_cast<const PcsEvent&>(event);
    auto timestamp = pcs_event.event_timestamp_;

    std::exponential_distribution<double> interval_expo(1.0/call_interval_mean_);
    std::exponential_distribution<double> duration_expo(1.0/call_duration_mean_);
    std::exponential_distribution<double> move_expo(1.0/move_interval_mean_);

    switch (pcs_event.event_type_) {

        case CALL_ARRIVED: {

            state_.calls_placed_++;
            if (state_.idle_channel_cnt_) {
                state_.idle_channel_cnt_--;
                auto move_interval = (unsigned int) std::ceil(move_expo(*this->rng_));
                auto call_duration = (unsigned int) std::ceil(duration_expo(*this->rng_));

                if (move_interval < call_duration) {
                    events.emplace_back(new PcsEvent {name_, timestamp + move_interval, 
                                                            timestamp, MOVE_CALL_OUT});
                } else {
                    events.emplace_back(new PcsEvent {name_, timestamp + call_duration, 
                                                                        0, CALL_COMPLETED});
                }
            } else {
                state_.calls_dropped_++;
                auto call_interval = (unsigned int) std::ceil(interval_expo(*this->rng_));
                events.emplace_back(new PcsEvent {name_, timestamp + call_interval, 
                                                                        0, CALL_ARRIVED});
            }
        } break;

        case CALL_COMPLETED: {

            state_.calls_completed_++;
            state_.idle_channel_cnt_++;
            auto call_interval = (unsigned int) std::ceil(interval_expo(*this->rng_));
            events.emplace_back(new PcsEvent {name_, timestamp + call_interval, 0, CALL_ARRIVED});
        } break;

        case MOVE_CALL_OUT: {

            state_.idle_channel_cnt_++;
            events.emplace_back(new PcsEvent {random_move(), timestamp, 
                                            pcs_event.call_arrival_ts_, MOVE_CALL_IN});
        } break;

        case MOVE_CALL_IN: {

            bool call_terminated = true;
            if (state_.idle_channel_cnt_) {

                auto call_duration = (unsigned int) std::ceil(duration_expo(*this->rng_));
                auto completion_timestamp = pcs_event.call_arrival_ts_ + call_duration;

                if (completion_timestamp > timestamp) {
                    state_.calls_handed_over_++;
                    state_.idle_channel_cnt_--;
                    events.emplace_back(new PcsEvent {name_, completion_timestamp, 
                                                                        0, CALL_COMPLETED});
                    call_terminated = false;
                }
            }

            if (call_terminated) {
                state_.calls_dropped_++;
                auto call_interval = (unsigned int) std::ceil(interval_expo(*this->rng_));
                events.emplace_back(new PcsEvent {name_, timestamp + call_interval, 
                                                                        0, CALL_ARRIVED});
            }
        } break;

        default: {
            assert(0);
        }
    }
    return events;
}

std::string PcsCell::compute_move(direction_t direction) {

    unsigned int new_x = 0, new_y = 0;
    unsigned int current_y = index_ / num_cells_x_;
    unsigned int current_x = index_ % num_cells_x_;

    switch (direction) {

        case LEFT: {
            new_x = (current_x + num_cells_x_ - 1) % num_cells_x_;
            new_y = current_y;
        } break;

        case RIGHT: {
            new_x = (current_x + 1) % num_cells_x_;
            new_y = current_y;
        } break;

        case DOWN: {
            new_x = current_x;
            new_y = (current_y + num_cells_y_ - 1) % num_cells_y_;
        } break;

        case UP: {
            new_x = current_x;
            new_y = (current_y + 1) % num_cells_y_;
        } break;

        default: {
            std::cerr << "Invalid move direction " << direction << std::endl;
            assert(0);
        }
    }

    return std::string("Cell_") + std::to_string(new_x + (new_y * num_cells_x_));
}

std::string PcsCell::random_move() {

    std::uniform_int_distribution<unsigned int> rand_direction(0,3);
    return this->compute_move((direction_t)rand_direction(*this->rng_));
}

int main(int argc, const char **argv) {

    unsigned int num_cells_x        = 100;
    unsigned int num_cells_y        = 100;
    unsigned int max_normal_ch_cnt  = 10;
    unsigned int max_reserve_ch_cnt = 5;
    unsigned int call_interval_mean = 400;
    unsigned int call_duration_mean = 250;
    unsigned int move_interval_mean = 500;
    unsigned int num_portables      = 50;

    TCLAP::ValueArg<unsigned int> num_cells_x_arg("x", "num-cells-x", "Width of cell grid",
                                                            false, num_cells_x, "unsigned int");
    TCLAP::ValueArg<unsigned int> num_cells_y_arg("y", "num-cells-y", "Height of cell grid",
                                                            false, num_cells_y, "unsigned int");
    TCLAP::ValueArg<unsigned int> max_normal_ch_cnt_arg("n", "normal-ch-cnt",
                        "Number of normal channels", false, max_normal_ch_cnt, "unsigned int");
    TCLAP::ValueArg<unsigned int> max_reserve_ch_cnt_arg("r", "reserve-ch-cnt",
                        "Number of reserve channels", false, max_reserve_ch_cnt, "unsigned int");
    TCLAP::ValueArg<unsigned int> call_interval_mean_arg("i", "call-interval",
                                "Mean time between end of last call and a new call", 
                                                    false, call_interval_mean, "unsigned int");
    TCLAP::ValueArg<unsigned int> call_duration_mean_arg("d", "call-duration", "Mean call duration", 
                                                    false, call_duration_mean, "unsigned int");
    TCLAP::ValueArg<unsigned int> move_interval_mean_arg("m", "move-interval", 
                                "Mean time between end of last move and next move", 
                                                    false, move_interval_mean, "unsigned int");
    TCLAP::ValueArg<unsigned int> num_portables_arg("p", "portable-count", 
                                "Portables per cell", false, num_portables, "unsigned int");

    std::vector<TCLAP::Arg*> cmd_line_args = {  &num_cells_x_arg, 
                                                &num_cells_y_arg, 
                                                &max_normal_ch_cnt_arg, 
                                                &max_reserve_ch_cnt_arg, 
                                                &call_interval_mean_arg, 
                                                &call_duration_mean_arg, 
                                                &move_interval_mean_arg, 
                                                &num_portables_arg      };

    warped::Simulation simulation {"PCS Simulation", argc, argv, cmd_line_args};

    num_cells_x         = num_cells_x_arg.getValue();
    num_cells_y         = num_cells_y_arg.getValue();
    max_normal_ch_cnt   = max_normal_ch_cnt_arg.getValue();
    max_reserve_ch_cnt  = max_reserve_ch_cnt_arg.getValue();
    call_interval_mean  = call_interval_mean_arg.getValue();
    call_duration_mean  = call_duration_mean_arg.getValue();
    move_interval_mean  = move_interval_mean_arg.getValue();
    num_portables       = num_portables_arg.getValue();

    std::vector<PcsCell> objects;
    for (unsigned int i = 0; i < num_cells_x * num_cells_y; i++) {

        std::string name = std::string("Cell_") + std::to_string(i);
        objects.emplace_back(   name, num_cells_x, num_cells_y, 
                                max_normal_ch_cnt, max_reserve_ch_cnt, 
                                call_interval_mean, call_duration_mean, 
                                move_interval_mean, num_portables, i );
    }

    std::vector<warped::SimulationObject*> object_pointers;
    for (auto& o : objects) {
        object_pointers.push_back(&o);
    }
    simulation.simulate(object_pointers);

    unsigned int call_attempts = 0, channel_blocks = 0, 
                    busy_lines = 0, handoff_blocks = 0;
    for (auto& o : objects) {
        call_attempts  += o.state_.call_attempts_;
        channel_blocks += o.state_.channel_blocks_;
        busy_lines     += o.state_.busy_lines_;
        handoff_blocks += o.state_.handoff_blocks_;
    }
    std::cout << "Call attempts  : " << call_attempts  << std::endl;
    std::cout << "Channel blocks : " << channel_blocks << std::endl;
    std::cout << "Busy lines     : " << busy_lines     << std::endl;
    std::cout << "Handoff blocks : " << handoff_blocks << std::endl;

    return 0;
}
