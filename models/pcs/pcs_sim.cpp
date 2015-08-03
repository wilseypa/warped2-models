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

#define TS_OFFSET 1

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(PcsEvent)

std::vector<std::shared_ptr<warped::Event> > PcsCell::initializeObject() {

    // Register random number generator to allow kernel to roll it back
    this->registerRNG<std::default_random_engine>(this->rng_);

    std::poisson_distribution<unsigned int> duration_expo(call_duration_mean_);
    std::poisson_distribution<unsigned int> move_expo(move_interval_mean_);
    std::poisson_distribution<unsigned int> interval_expo(call_interval_mean_);

    std::vector<std::shared_ptr<warped::Event>> events;

    for (unsigned int i = 0; i < portable_init_cnt_; i++) {

        unsigned int complete_call_ts = duration_expo(*this->rng_) + TS_OFFSET;
        unsigned int move_call_ts = move_expo(*this->rng_) + TS_OFFSET;
        unsigned int next_call_ts = interval_expo(*this->rng_) + TS_OFFSET;

        auto next_action = min_ts(complete_call_ts, next_call_ts, move_call_ts);
        switch (next_action) {
            case COMPLETECALL: {
                // If channels available, consider it as an ongoing call
                if (state_.idle_channel_cnt_) {
                    state_.idle_channel_cnt_--;
                    events.emplace_back(new PcsEvent {this->name_, complete_call_ts, 
                                            complete_call_ts, next_call_ts, 
                                            move_call_ts, COMPLETE_CALL_METHOD});
                } else { // Channels not available
                    complete_call_ts += next_call_ts;
                    state_.channel_blocks_++;
                    // If move_call_ts < next_call_ts, start the handover process
                    if (move_call_ts <= next_call_ts) {
                        events.emplace_back(new PcsEvent {this->name_, move_call_ts, 
                                            complete_call_ts, next_call_ts, 
                                            move_call_ts, MOVE_CALL_OUT_METHOD});
                    } else { // Else start a new call
                        events.emplace_back(new PcsEvent {this->name_, next_call_ts, 
                                            complete_call_ts, next_call_ts, 
                                            move_call_ts, NEXT_CALL_METHOD});
                    }
                }
            } break;

            case MOVECALL: {
                /* Since no channels acquired, complete_call_ts < next_call_ts will 
                   create an ambiguity in the receiveEvent() */
                complete_call_ts += (complete_call_ts <= next_call_ts) ? next_call_ts : 0;
                events.emplace_back(new PcsEvent {this->name_, move_call_ts, 
                                            complete_call_ts, next_call_ts, 
                                            move_call_ts, MOVE_CALL_OUT_METHOD});
            } break;

            case NEXTCALL: {
                events.emplace_back(new PcsEvent {this->name_, next_call_ts, 
                                            complete_call_ts, next_call_ts, 
                                            move_call_ts, NEXT_CALL_METHOD});
            } break;
        }
    }
    return events;
}

std::vector<std::shared_ptr<warped::Event> > PcsCell::receiveEvent(const warped::Event& event) {

    std::vector<std::shared_ptr<warped::Event>> events;
    auto pcs_event = static_cast<const PcsEvent&>(event);

    std::poisson_distribution<unsigned int> duration_expo(call_duration_mean_);
    std::poisson_distribution<unsigned int> move_expo(move_interval_mean_);
    std::poisson_distribution<unsigned int> interval_expo(call_interval_mean_);

    unsigned int complete_call_ts = 0, move_call_ts = 0, next_call_ts = 0;
    action_t next_action;

    switch (pcs_event.method_) {

        case NEXT_CALL_METHOD: {

            move_call_ts = pcs_event.move_call_ts_;
            complete_call_ts = pcs_event.complete_call_ts_;
            next_call_ts = pcs_event.next_call_ts_;
            assert(next_call_ts < complete_call_ts);
            state_.call_attempts_++;

            if (!state_.idle_channel_cnt_) { // Channels not available
                state_.channel_blocks_++;
                next_call_ts += interval_expo(*this->rng_) + TS_OFFSET;
                complete_call_ts = next_call_ts + duration_expo(*this->rng_) + TS_OFFSET;
                if (move_call_ts <= next_call_ts) {
                    events.emplace_back(new PcsEvent {this->name_, move_call_ts, 
                                        complete_call_ts, next_call_ts, 
                                        move_call_ts, MOVE_CALL_OUT_METHOD});
                } else {
                    events.emplace_back(new PcsEvent {this->name_, next_call_ts, 
                                        complete_call_ts, next_call_ts, 
                                        move_call_ts, NEXT_CALL_METHOD});
                }
            } else { // Channels available
                state_.idle_channel_cnt_--;
                next_call_ts = complete_call_ts + interval_expo(*this->rng_) + TS_OFFSET;
                if (complete_call_ts < move_call_ts) {
                    events.emplace_back(new PcsEvent {this->name_, complete_call_ts, 
                                        complete_call_ts, next_call_ts, 
                                        move_call_ts, COMPLETE_CALL_METHOD});
                } else {
                    events.emplace_back(new PcsEvent {this->name_, move_call_ts, 
                                        complete_call_ts, next_call_ts, 
                                        move_call_ts, MOVE_CALL_OUT_METHOD});
                }
            }
        } break;

        case COMPLETE_CALL_METHOD: {

            state_.idle_channel_cnt_++;
            next_call_ts = pcs_event.next_call_ts_;
            move_call_ts = pcs_event.move_call_ts_;
            complete_call_ts = next_call_ts + duration_expo(*this->rng_) + TS_OFFSET;

            next_action = min_ts(complete_call_ts, next_call_ts, move_call_ts);
            switch (next_action) {
                case MOVECALL: {
                    events.emplace_back(new PcsEvent {this->name_, move_call_ts, 
                                        complete_call_ts, next_call_ts, 
                                        move_call_ts, MOVE_CALL_OUT_METHOD});
                } break;

                case COMPLETECALL: {
                    assert(0);
                } break;

                case NEXTCALL: {
                    events.emplace_back(new PcsEvent {this->name_, next_call_ts, 
                                        complete_call_ts, next_call_ts, 
                                        move_call_ts, NEXT_CALL_METHOD});
                } break;
            }
        } break;

        case MOVE_CALL_OUT_METHOD: {

            complete_call_ts = pcs_event.complete_call_ts_;
            next_call_ts = pcs_event.next_call_ts_;
            move_call_ts = pcs_event.move_call_ts_;

            if (complete_call_ts < next_call_ts) {
                state_.idle_channel_cnt_++;
            }
            events.emplace_back(new PcsEvent {random_move(), move_call_ts, 
                                        complete_call_ts, next_call_ts, 
                                        move_call_ts, MOVE_CALL_IN_METHOD});
        } break;

        case MOVE_CALL_IN_METHOD: {

            complete_call_ts = pcs_event.complete_call_ts_;
            next_call_ts = pcs_event.next_call_ts_;
            move_call_ts = pcs_event.move_call_ts_ + move_expo(*this->rng_) + TS_OFFSET;
            next_action = min_ts(complete_call_ts, next_call_ts, move_call_ts);

            // Call handover only if complete_call_ts <= next_call_ts
            if (complete_call_ts <= next_call_ts) {

                // No channels available
                if (!state_.idle_channel_cnt_) {
                    state_.handoff_blocks_++;
                    state_.channel_blocks_++;

                    complete_call_ts = next_call_ts + duration_expo(*this->rng_) + TS_OFFSET;

                    // End call and schedule a new one if next_call_ts < move_call_ts
                    if (next_call_ts < move_call_ts) {
                        events.emplace_back(new PcsEvent {this->name_, next_call_ts, 
                                                        complete_call_ts, next_call_ts, 
                                                        move_call_ts, NEXT_CALL_METHOD});
                    } else {
                        events.emplace_back(new PcsEvent {this->name_, move_call_ts, 
                                                        complete_call_ts, next_call_ts, 
                                                        move_call_ts, MOVE_CALL_OUT_METHOD});
                    }
                } else { // Channels available, complete call
                    state_.idle_channel_cnt_--;

                    switch (next_action) {
                        case NEXTCALL: {
                            assert(0);
                        } break;

                        case COMPLETECALL: {
                            events.emplace_back(new PcsEvent {this->name_, complete_call_ts, 
                                                        complete_call_ts, next_call_ts, 
                                                        move_call_ts, COMPLETE_CALL_METHOD});
                        } break;

                        case MOVECALL: {
                            events.emplace_back(new PcsEvent {this->name_, move_call_ts, 
                                                        complete_call_ts, next_call_ts, 
                                                        move_call_ts, MOVE_CALL_OUT_METHOD});
                        } break;
                    }
                }
            } else { // Portable was not busy
                switch (next_action) {
                    case COMPLETECALL: {
                        assert(0);
                    } break;

                    case NEXTCALL: {
                        events.emplace_back(new PcsEvent {this->name_, next_call_ts, 
                                                        complete_call_ts, next_call_ts, 
                                                        move_call_ts, NEXT_CALL_METHOD});
                    } break;

                    case MOVECALL: {
                        events.emplace_back(new PcsEvent {this->name_, move_call_ts, 
                                                        complete_call_ts, next_call_ts, 
                                                        move_call_ts, MOVE_CALL_OUT_METHOD});
                    } break;
                }
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

action_t PcsCell::min_ts(   unsigned int complete_call_ts, 
                            unsigned int next_call_ts, 
                            unsigned int move_call_ts) {

    action_t next_action;
    // If next call ts is greater than complete call ts
    if (complete_call_ts <= next_call_ts) {
        // If complete call == move call, move call takes preceedence
        next_action = (complete_call_ts < move_call_ts) ? COMPLETECALL : MOVECALL;
    } else {
        // If next call == move call, move call takes preceedence
        next_action = (next_call_ts < move_call_ts) ? NEXTCALL : MOVECALL;
    }
    return next_action;
}

int main(int argc, const char **argv) {

    unsigned int num_cells_x        = 100;
    unsigned int num_cells_y        = 100;
    unsigned int max_channel_cnt    = 15;
    unsigned int call_interval_mean = 200;
    unsigned int call_duration_mean = 50;
    unsigned int move_interval_mean = 100;
    unsigned int num_portables      = 50;

    TCLAP::ValueArg<unsigned int> num_cells_x_arg("x", "num-cells-x", "Width of cell grid",
                                                            false, num_cells_x, "unsigned int");
    TCLAP::ValueArg<unsigned int> num_cells_y_arg("y", "num-cells-y", "Height of cell grid",
                                                            false, num_cells_y, "unsigned int");
    TCLAP::ValueArg<unsigned int> max_channel_cnt_arg("n", "channel-cnt", "Number of channels", 
                                                        false, max_channel_cnt, "unsigned int");
    TCLAP::ValueArg<unsigned int> call_interval_mean_arg("i", "call-interval",
                                "Mean time between end of last call and a new call", 
                                                    false, call_interval_mean, "unsigned int");
    TCLAP::ValueArg<unsigned int> call_duration_mean_arg("d", "call-duration", 
                                "Mean call duration", false, call_duration_mean, "unsigned int");
    TCLAP::ValueArg<unsigned int> move_interval_mean_arg("m", "move-interval", 
                                "Mean time between end of last move and next move", 
                                                    false, move_interval_mean, "unsigned int");
    TCLAP::ValueArg<unsigned int> num_portables_arg("p", "portable-count", 
                                "Portables per cell", false, num_portables, "unsigned int");

    std::vector<TCLAP::Arg*> cmd_line_args = {  &num_cells_x_arg, 
                                                &num_cells_y_arg, 
                                                &max_channel_cnt_arg, 
                                                &call_interval_mean_arg, 
                                                &call_duration_mean_arg, 
                                                &move_interval_mean_arg, 
                                                &num_portables_arg      };

    warped::Simulation simulation {"PCS Simulation", argc, argv, cmd_line_args};

    num_cells_x         = num_cells_x_arg.getValue();
    num_cells_y         = num_cells_y_arg.getValue();
    max_channel_cnt     = max_channel_cnt_arg.getValue();
    call_interval_mean  = call_interval_mean_arg.getValue();
    call_duration_mean  = call_duration_mean_arg.getValue();
    move_interval_mean  = move_interval_mean_arg.getValue();
    num_portables       = num_portables_arg.getValue();

    std::vector<PcsCell> objects;
    for (unsigned int i = 0; i < num_cells_x * num_cells_y; i++) {

        std::string name = std::string("Cell_") + std::to_string(i);
        objects.emplace_back(name, num_cells_x, num_cells_y, max_channel_cnt, 
                call_interval_mean, call_duration_mean, move_interval_mean, num_portables, i);
    }

    std::vector<warped::SimulationObject*> object_pointers;
    for (auto& o : objects) {
        object_pointers.push_back(&o);
    }
    simulation.simulate(object_pointers);

    unsigned int call_attempts = 0, channel_blocks = 0, handoff_blocks = 0;
    for (auto& o : objects) {
        call_attempts  += o.state_.call_attempts_;
        channel_blocks += o.state_.channel_blocks_;
        handoff_blocks += o.state_.handoff_blocks_;
    }
    std::cout << "Call attempts  : " << call_attempts  << std::endl;
    std::cout << "Channel blocks : " << channel_blocks << std::endl;
    std::cout << "Handoff blocks : " << handoff_blocks << std::endl;

    return 0;
}
