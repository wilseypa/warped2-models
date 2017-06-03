/* Implementation of a Forest Fire Simulation */

#include <cassert>
#include <random>
#include "forest.hpp"
#include "tclap/ValueArg.h"

#define IGNITION_DELAY      1
#define RADIATION_DELAY     1
#define RADIATION_INTERVAL  5


WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(ForestEvent)

std::vector<std::shared_ptr<warped::Event> > Forest::initializeLP() {

    std::vector<std::shared_ptr<warped::Event> > events;

    /* If heat content exceeds ignition threshold, schedule ignition */
    if (state_.heat_content_ >= ignition_threshold_) {
        events.emplace_back( new ForestEvent {name_, IGNITION, IGNITION_DELAY} );
    }
    return events;
}

inline std::string Forest::lp_name(const unsigned int lp_index){

    return std::string("Cell_") + std::to_string(lp_index);
}

std::vector<std::shared_ptr<warped::Event> > Forest::receiveEvent(const warped::Event& event) {

    std::vector<std::shared_ptr<warped::Event> > response_events;
    auto received_event = static_cast<const ForestEvent&>(event);

    switch (received_event.type_) {

        case RADIATION: {

            if (this->state_.burn_status_ == BURNT_OUT) break;

            state_.heat_content_ += received_event.heat_content_;

            /* If heat exceeds ignition threshold and vegtation is unburnt, schedule ignition */
            if ( (state_.burn_status_ == UNBURNT) && 
                    (state_.heat_content_ >= ignition_threshold_) ) {
                response_events.emplace_back( 
                    new ForestEvent {name, IGNITION, received_event.ts_ + IGNITION_DELAY} );
            }
        } break;

        case RADIATION_TIMER: {

            unsigned int heat_radiated_out = 
                std::static_cast<unsigned int> (state_.heat_content_ * radiation_fraction_);
            state_.heat_content_ -= heat_radiated_out;

            /* Schedule radiation events for surrounding LPs */
            for( auto direction = NORTH; direction < DIRECTION_MAX; direction++ ) {
                if (!connection_[direction]) continue;
                response_events.emplace_back( new ForestEvent {find_cell(direction), 
                                    RADIATION, heat_radiated_out/DIRECTION_MAX, 
                                            received_event.ts_ + RADIATION_DELAY} );
            }

            /* Check if cell has burnt out */
            if (state_.heat_content_ <= burnout_threshold_) {
                state_.burn_status_ = BURNT_OUT;

            } else { /* Else schedule next radiation */
                response_events.emplace_back( new ForestEvent {name_, 
                        RADIATION_TIMER, received_event.ts_ + RADIATION_INTERVAL} );
            }
        } break;

        case IGNITION: {

            state_.burn_status_= BURNING;

            /* Schedule Peak Event */
            unsigned int peak_time = received_event.ts_ + 
                        ((peak_threshold_ - ignition_threshold_) / heat_rate_);
            response_events.emplace_back( new ForestEvent {name_, PEAK, peak_time} );
        } break;

        case PEAK: {

            state_.heat_content_ = state_.heat_content_ + (peak_threshold_ - ignition_threshold_);

            /* Schedule first Radiation Timer */
            response_events.emplace_back( 
                    new ForestEvent {name_, RADIATION_TIMER, received_event.ts_} );
        } break;
    }
    return response_events;
}

std::string Forest::find_cell( direction_t direction ) {

    unsigned int new_x = 0, new_y = 0;
    unsigned int current_y = index_ / size_x_;
    unsigned int current_x = index_ % size_x_;

    switch( direction ) {

        case NORTH: {
            new_x = current_x;
            new_y = (current_y + size_y_ - 1) % size_y_;
        } break;

        case NORTH_EAST: {
            new_x = (current_x + 1) % size_x_;
            new_y = (current_y + size_y_ - 1) % size_y_;
        } break;

        case EAST: {
            new_x = (current_x + 1) % size_x_;
            new_y = current_y;
        } break;

        case SOUTH_EAST: {
            new_x = (current_x + 1) % size_x_;
            new_y = (current_y + 1) % size_y_;
        } break;

        case SOUTH: {
            new_x = current_x;
            new_y = (current_y + 1) % size_y_;
        } break;

        case SOUTH_WEST: {
            new_x = (current_x + size_x_ - 1) % size_x_;
            new_y = (current_y + 1) % size_y_;
        } break;

        case WEST: {
            new_x = (current_x + size_x_ - 1) % size_x_;
            new_y = current_y;
        } break;

        case NORTH_WEST: {
            new_x = (current_x + size_x_ - 1) % size_x_;
            new_y = (current_y + size_y_ - 1) % size_y_;
        } break;

        default: {
            std::cerr << "Invalid move direction " << direction << std::endl;
            assert(0);
        }
    }
    return lp_name( new_x + new_y * size_x_ );
}


int main(int argc, char *argv[]) {

    std::string config_filename = "map_hawaii.bmp";
    unsigned int heat_rate = 100;
    double radiation_fraction_ = 0.05;
    unsigned int burnout_threshold = 50;
    unsigned int burn_start_x = 500;
    unsigned int burn_start_y = 501;

    TCLAP::ValueArg<std::string> config_arg("m", "map", "Forest model vegetation config", 
                                                                false, config_filename, "string");
    TCLAP::ValueArg<unsigned int> heat_rate_arg("h", "heat-rate", "Speed of growth of the fire",
                                                                false, heat_rate, "unsigned int");
    TCLAP::ValueArg<double> radiation_fraction_arg("r", "radiation-percent", 
             "Percent of Heat released every timstamp", false, radiation_fraction, "double");
    TCLAP::ValueArg<unsigned int> burnout_threshold_arg("b", "burnout-threshold",
                                    "Amount of heat needed for a cell to burn out", false, 
                                                               burnout_threshold, "unsigned int");
    TCLAP::ValueArg<unsigned int> burn_start_x_arg("x", "burn-start-x",
                                                     "x coordinate of the start of fire", false,
                                                                    burn_start_x, "unsigned int");
    TCLAP::ValueArg<unsigned int> burn_start_y_arg("y", "burn-start-y",
                                                     "y coordinate of the start of fire", false,
                                                                    burn_start_y, "unsigned int");

    std::vector<TCLAP::Arg*> args = {&config_arg, &heat_rate_arg,
                                        &radiation_fraction_arg, &burnout_threshold_arg,
                                                    &burn_start_x_arg, &burn_start_y_arg};

    config_filename = config_arg.getValue();
    heat_rate = heat_rate_arg.getValue();
    radiation_fraction = radiation_fraction_arg.getValue();
    burnout_threshold = burnout_threshold.getValue();
    burn_start_x = burn_start_x.getValue();
    burn_start_y = burn_start_y.getValue();

    warped::Simulation forest_sim {"Forest Simulation", argc, argv, args};

    FILE *fp = fopen(img_name.c_str(), "rb");
    if (!fp) throw "Incorrect name of vegetation map";

    /* Read the 54-byte header */
    unsigned char info[54];
    fread(info, sizeof(unsigned char), 54, fp);

    /* Extract image height and width from header */
    width  = *(unsigned int *)&info[18];
    height = *(unsigned int *)&info[22];

    unsigned int row_padded = (width*3 + 3) & (~3);
    unsigned char *data = new unsigned char[row_padded];

    std::vector<Forest> lps;

    for( unsigned int i = 0; i < height; i++ ) {

        fread(data, sizeof(unsigned char), row_padded, fp);

        for(unsigned int j = 0; j < width*3; j += 3) {

            unsigned int index = i*width + j/3; 
            /* Placeholder equations for threshold calculation */
            unsigned int ignition_threshold = 
                std::static_cast<unsigned int>(data[j] + data[j+1] + data[j+2]);
            unsigned int peak_threshold = ignition_threshold * 2;

            /* Set The black squares ignition threshold to an unreachable number */
            if(ignition_threshold == 0  && peak_threshold == 0){
                ignition_threshold = 1000000;
                peak_threshold = 10000000;
            }   

            std::string name = Forest::lp_name(index);
            lps.emplace_back(name, width, height, ignition_threshold, heat_rate, 
                                     peak_threshold, radiation_fraction, burnout_threshold, index);
 
            /* If the LP being created is the start of the fire then give it intial heat content */
            if(i == burn_start_x && j == burn_start_y){
                state_.heat_content_ = ignition_threshold_ + 10;
            }
        }
    }
    fclose(fp);

    std::vector<warped::LogicalProcess*> lp_pointers;
    for (auto& lp : lps) {
        lp_pointers.push_back(&lp);
    }

    forest_sim.simulate(lp_pointers);
    
    unsigned int cells_burned = 0;

    for(auto& lp: lps){
        if( lp.state_.burn_status == BURNT_OUT ){
            cells_burned++;
        }
    }

    std::cout<<"Total trees burned: " << cells_burned << std::endl;

    return 0;

}
