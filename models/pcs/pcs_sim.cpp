// See "Distributed Simulation of Large-Scale PCS Networks" by Carothers et al.

#include <vector>
#include <memory>
#include <cassert>
#include <algorithm>
#include <random>
#include <cstdlib>

#include "warped.hpp"
#include "pcs_sim.hpp"

#include "MLCG.h"
#include "NegExp.h"

#include "tclap/ValueArg.h"

#define NEG_EXPL_OFFSET 10

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(PcsEvent)

std::vector<std::shared_ptr<warped::Event> > PcsCell::createInitialEvents() {

    std::vector<std::shared_ptr<warped::Event>> events;
    for (auto it = state_.portables_.begin(); it != state_.portables_.end(); it++) {
        auto portable = it->second;
        events.emplace_back(new PcsEvent {this->name_, portable->call_interval_, 
                                                            portable, CALL_ARRIVAL});
    }
    return events;
}

std::vector<std::shared_ptr<warped::Event> > PcsCell::receiveEvent(const warped::Event& event) {

    std::vector<std::shared_ptr<warped::Event>> events;
    auto pcs_event = static_cast<const PcsEvent&>(event);
    auto timestamp = pcs_event.event_timestamp_;
    auto pid       = pcs_event.pid_;

    switch (pcs_event.event_type_) {

        case CALL_ARRIVAL: {

            if (state_.idle_channel_cnt_) {

                state_.idle_channel_cnt_--;
                state_.portables_[pid]->is_busy_ = true;
                state_.portables_[pid]->call_arrival_ts_ = timestamp;

                NegativeExpntl move_expo(pcs_event.call_duration_, this->rng_.get());
                auto move_interval = (unsigned int) move_expo() + NEG_EXPL_OFFSET;
                if (move_interval < state_.portables_[pid]->call_duration_) {
                    events.emplace_back(new PcsEvent {name_, timestamp + move_interval, 
                                                state_.portables_[pid], PORTABLE_MOVE_OUT});
                } else {
                    events.emplace_back(new PcsEvent {name_, 
                                        timestamp + state_.portables_[pid]->call_duration_, 
                                                    state_.portables_[pid], CALL_COMPLETION});
                }
            } else {
                events.emplace_back(new PcsEvent {name_, 
                                        timestamp + state_.portables_[pid]->call_interval_, 
                                                        state_.portables_[pid], CALL_ARRIVAL});
            }
        } break;

        case CALL_COMPLETION: {

            state_.idle_channel_cnt_++;
            state_.portables_[pid]->is_busy_ = false;
            events.emplace_back(new PcsEvent {name_, 
                                        timestamp + state_.portables_[pid]->call_interval_, 
                                                        state_.portables_[pid], CALL_ARRIVAL});
        } break;

        case PORTABLE_MOVE_OUT: {

            auto portable = state_.portables_[pid];
            state_.portables_.erase(pid);
            state_.idle_channel_cnt_++;
            events.emplace_back(new PcsEvent {random_move(), timestamp + NEG_EXPL_OFFSET, 
                                                                portable, PORTABLE_MOVE_IN});
            std::move(portable);

        } break;

        case PORTABLE_MOVE_IN: {

            auto portable = std::make_shared<Portable> (pid, 
                                                        pcs_event.is_busy_, 
                                                        pcs_event.call_arrival_ts_, 
                                                        pcs_event.call_interval_, 
                                                        pcs_event.call_duration_);
            state_.portables_.insert(state_.portables_.begin(), 
                std::pair <unsigned int, std::shared_ptr<Portable>> (portable->pid_, portable));

            bool call_terminated = true;
            if (portable->is_busy_ && state_.idle_channel_cnt_) {
                auto completion_timestamp = pcs_event.call_arrival_ts_ + pcs_event.call_duration_;
                if (completion_timestamp > timestamp) {
                    state_.idle_channel_cnt_--;
                    events.emplace_back(new PcsEvent {name_, completion_timestamp, 
                                                                    portable, CALL_COMPLETION});
                    call_terminated = false;
                }
            }

            if (call_terminated) {
                portable->is_busy_ = false;
                events.emplace_back(new PcsEvent {name_, 
                                        timestamp + state_.portables_[pid]->call_interval_, 
                                                        state_.portables_[pid], CALL_ARRIVAL});
            }
        } break;

        default: {
            assert(0);
        }
    }
    return events;
}

std::string PcsCell::compute_move(direction_t direction) {

    int new_x = 0, new_y = 0;
    int current_y = ((int)this->index_) / num_cells_x_;
    int current_x = ((int)this->index_) - (current_y * num_cells_x_);

    switch (direction) {

        case LEFT: {
            new_x = ((current_x - 1) + num_cells_x_) % num_cells_x_;
            new_y = current_y;
        } break;

        case RIGHT: {
            new_x = (current_x + 1) % num_cells_x_;
            new_y = current_y;
        } break;

        case DOWN: {
            new_x = current_x;
            new_y = ((current_y - 1) + num_cells_y_) % num_cells_y_;
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

    return std::string("Object ") + std::to_string(new_x + (new_y * num_cells_x_));
}

std::string PcsCell::random_move() {

    std::default_random_engine gen;
    std::uniform_int_distribution<unsigned int> rand_direction(0,3);
    return this->compute_move((direction_t)rand_direction(gen));
}

int main(int argc, const char **argv) {

    unsigned int num_cells_x        = 100;
    unsigned int num_cells_y        = 100;
    unsigned int call_interval_mean = 500;
    unsigned int call_duration_mean = 250;
    unsigned int channel_cnt        = 10;
    unsigned int num_portables      = 25;

    TCLAP::ValueArg<unsigned int> num_cells_x_arg("x", "num-cells-x", "Width of cell grid",
                                                            false, num_cells_x, "unsigned int");
    TCLAP::ValueArg<unsigned int> num_cells_y_arg("y", "num-cells-y", "Height of cell grid",
                                                            false, num_cells_y, "unsigned int");
    TCLAP::ValueArg<unsigned int> call_interval_mean_arg("i", "call-interval",
                                "Mean time between end of last call and a new call", 
                                                    false, call_interval_mean, "unsigned int");
    TCLAP::ValueArg<unsigned int> call_duration_mean_arg("d", "call-duration", "Mean call duration", 
                                                    false, call_duration_mean, "unsigned int");
    TCLAP::ValueArg<unsigned int> channel_cnt_arg("n", "channel-count",
                        "Number of communication channels", false, channel_cnt, "unsigned int");
    TCLAP::ValueArg<unsigned int> num_portables_arg("p", "portable-count", 
                            "Portables per cell", false, num_portables, "unsigned int");

    std::vector<TCLAP::Arg*> cmd_line_args = {&num_cells_x_arg, &num_cells_y_arg, 
                                                &call_interval_mean_arg, &call_duration_mean_arg, 
                                                &channel_cnt_arg, &num_portables_arg};

    warped::Simulation simulation {"PCS Simulation", argc, argv, cmd_line_args};

    num_cells_x         = num_cells_x_arg.getValue();
    num_cells_y         = num_cells_y_arg.getValue();
    call_interval_mean  = call_interval_mean_arg.getValue();
    call_duration_mean  = call_duration_mean_arg.getValue();
    channel_cnt         = channel_cnt_arg.getValue();
    num_portables       = num_portables_arg.getValue();

    std::vector<PcsCell> objects;
    unsigned int pid = 0;
    std::shared_ptr<MLCG> rng = std::make_shared<MLCG> ();

    for (unsigned int i = 0; i < num_cells_x * num_cells_y; i++) {

        std::vector<std::shared_ptr<Portable>> portables;

        NegativeExpntl interval_expo(call_interval_mean, rng.get());
        NegativeExpntl duration_expo(call_duration_mean, rng.get());

        for (unsigned int i = 0; i < num_portables; i++) {
            pid++;
            auto interval = (unsigned int) interval_expo() + NEG_EXPL_OFFSET;
            auto duration = (unsigned int) duration_expo() + NEG_EXPL_OFFSET;
            auto portable = std::make_shared<Portable> (pid, false, 0, interval, duration);
            portables.push_back(portable);
        }
        std::string name = std::string("Object ") + std::to_string(i);
        objects.emplace_back(name, num_cells_x, num_cells_y, 
                                        channel_cnt, portables, i);
    }
    std::move(rng);

    std::vector<warped::SimulationObject*> object_pointers;
    for (auto& o : objects) {
        object_pointers.push_back(&o);
    }
    simulation.simulate(object_pointers);

    return 0;
}
