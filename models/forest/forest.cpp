/* Implementation of a Forest Fire Simulation */

#include <cassert>
#include "forest.hpp"
#include "tclap/ValueArg.h"

/* Event timer delays */
#define IGNITION_DELAY              1
#define RADIATION_DELAY             1
#define RADIATION_INTERVAL          5

/* Combustion parameters */
#define PEAK_TO_IGN_THRES_RATIO     2
#define INITIAL_HEAT_CONTENT        20

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(ForestEvent)

std::vector<std::shared_ptr<warped::Event> > Forest::initializeLP() {

    std::vector<std::shared_ptr<warped::Event> > events;

    /* If heat content exceeds ignition threshold, schedule ignition */
    if (state_.heat_content_ >= ignition_threshold_) {
        events.emplace_back( new ForestEvent{lp_name(index_), IGNITION, 0, IGNITION_DELAY} );
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
                unsigned int next_ts = received_event.ts_ + IGNITION_DELAY;
                response_events.emplace_back(
                        new ForestEvent{lp_name(index_), IGNITION, 0, next_ts} );
            }
        } break;

        case RADIATION_TIMER: {

            unsigned int heat_radiated_out = 
                static_cast<unsigned int> (state_.heat_content_ * radiation_fraction_);
            state_.heat_content_ -= heat_radiated_out;

            /* Schedule radiation events for surrounding LPs */
            unsigned int next_ts = received_event.ts_ + RADIATION_DELAY;
            for (unsigned int direction = NORTH; direction < DIRECTION_MAX; direction++) {
                if (!connection_[direction]) continue;
                response_events.emplace_back(
                            new ForestEvent{find_cell( (direction_t)direction ), 
                                    RADIATION, heat_radiated_out/DIRECTION_MAX, next_ts} );
            }

            /* Check if cell has burnt out */
            if (state_.heat_content_ <= burnout_threshold_) {
                state_.burn_status_ = BURNT_OUT;

            } else { /* Else schedule next radiation */
                next_ts = received_event.ts_ + RADIATION_INTERVAL;
                response_events.emplace_back(
                        new ForestEvent{lp_name(index_), RADIATION_TIMER, 0, next_ts} );
            }
        } break;

        case IGNITION: {

            state_.burn_status_= BURNING;

            /* Schedule Peak Event */
            unsigned int peak_time = received_event.ts_ + 
                        ((peak_threshold_ - ignition_threshold_) / heat_rate_);
            response_events.emplace_back( new ForestEvent{lp_name(index_), PEAK, 0, peak_time} );
        } break;

        case PEAK: {

            state_.heat_content_ += peak_threshold_ - ignition_threshold_;

            /* Schedule first Radiation Timer */
            response_events.emplace_back( 
                    new ForestEvent{lp_name(index_), RADIATION_TIMER, 0, received_event.ts_} );
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

bool Forest::neighbor_conn( direction_t direction, unsigned char **combustible_map ) {

    unsigned int new_x = 0, new_y = 0;
    unsigned int current_y = index_ / size_x_;
    unsigned int current_x = index_ % size_x_;

    /* Eliminate the boundary situations */
    /* If LP is in the first row */
    if (!current_y) {

        /* NW, N and NE don't exist for the first row */
        if (direction == NORTH_WEST || direction == NORTH || direction == NORTH_EAST) {
            return false;
        }

        /* SW and W don't exist for LP at beginning of the first row */
        if (!current_x && (direction == SOUTH_WEST || direction == WEST)) {
            return false;
        }

        /* E and SE don't exist for LP at end of the first row */
        if ((current_x == size_x_-1) && (direction == EAST || direction == SOUTH_EAST)) {
            return false;
        }
    }

    /* If LP is in the last row */
    if (current_y == size_y_-1) {

        /* SE, S and SW don't exist for the last row */
        if (direction == SOUTH_EAST || direction == SOUTH || direction == SOUTH_WEST) {
            return false;
        }

        /* W and NW don't exist for LP at beginning of the last row */
        if (!current_x && (direction == WEST || direction == NORTH_WEST)) {
            return false;
        }

        /* NE and E don't exist for LP at end of the last row */
        if ((current_x == size_x_-1) && (direction == NORTH_EAST || direction == EAST)) {
            return false;
        }
    }

    /* If LP is in the first column and not on the first or last rows */
    /* SW, W and NW don't exist for the first column */
    if (!current_x && 
            (direction == SOUTH_WEST || direction == WEST || direction == NORTH_WEST)) {
        return false;
    }

    /* If LP is in the last column and not on the first or last rows */
    /* NE, E and SE don't exist for the first column */
    if ((current_x == size_x_-1) && 
            (direction == NORTH_EAST || direction == EAST || direction == SOUTH_EAST)) {
        return false;
    }

    /* Else LP is not looking for a neighbor outside of the grid limits */
    switch( direction ) {

        case NORTH: {
            new_x = current_x;
            new_y = current_y - 1;
        } break;

        case NORTH_EAST: {
            new_x = current_x + 1;
            new_y = current_y - 1;
        } break;

        case EAST: {
            new_x = current_x + 1;
            new_y = current_y;
        } break;

        case SOUTH_EAST: {
            new_x = current_x + 1;
            new_y = current_y + 1;
        } break;

        case SOUTH: {
            new_x = current_x;
            new_y = current_y + 1;
        } break;

        case SOUTH_WEST: {
            new_x = current_x - 1;
            new_y = current_y + 1;
        } break;

        case WEST: {
            new_x = current_x - 1;
            new_y = current_y;
        } break;

        case NORTH_WEST: {
            new_x = current_x - 1;
            new_y = current_y - 1;
        } break;

        default: {
            std::cerr << "Invalid move direction " << direction << std::endl;
            assert(0);
        }
    }

    /* If no LP exists for that grid position */
    if (!combustible_map[new_x][new_y]) return false;

    return true;
}

int main(int argc, char *argv[]) {

    /* Set the default values for the simulation arguments */
    std::string     vegetation_map      = "map_hawaii.bmp";
    unsigned int    heat_rate           = 15;
    double          radiation_fraction  = 0.05;
    unsigned int    burnout_threshold   = INITIAL_HEAT_CONTENT;
    unsigned int    fire_origin_x       = 500;
    unsigned int    fire_origin_y       = 501;

    /* Read any simulation arguments (if provided) */
    TCLAP::ValueArg<std::string> vegetation_map_arg( "v", "vegetation-map", "Vegetation map",
                                                    false, vegetation_map, "string" );

    TCLAP::ValueArg<unsigned int> heat_rate_arg( "g", "heat-rate", "Rate of fire growth",
                                                    false, heat_rate, "unsigned int" );

    TCLAP::ValueArg<double> radiation_fraction_arg( "r", "radiation-fraction",
                                                    "Fraction of heat radiated out",
                                                    false, radiation_fraction, "double" );

    TCLAP::ValueArg<unsigned int> burnout_threshold_arg( "b", "burnout-threshold",
                                                    "Heat left in a cell after it burns out",
                                                    false, burnout_threshold, "unsigned int" );

    TCLAP::ValueArg<unsigned int> fire_origin_x_arg( "x", "fire-origin-x",
                                                    "X-coordinate for origin of fire",
                                                    false, fire_origin_x, "unsigned int" );

    TCLAP::ValueArg<unsigned int> fire_origin_y_arg( "y", "fire-origin-y",
                                                    "Y-coordinate for origin of fire",
                                                    false, fire_origin_y, "unsigned int" );

    std::vector<TCLAP::Arg*> args = {   &vegetation_map_arg,
                                        &heat_rate_arg,
                                        &radiation_fraction_arg,
                                        &burnout_threshold_arg,
                                        &fire_origin_x_arg,
                                        &fire_origin_y_arg
                                    };

    vegetation_map      = vegetation_map_arg.getValue();
    heat_rate           = heat_rate_arg.getValue();
    radiation_fraction  = radiation_fraction_arg.getValue();
    burnout_threshold   = burnout_threshold_arg.getValue();
    fire_origin_x       = fire_origin_x_arg.getValue();
    fire_origin_y       = fire_origin_y_arg.getValue();

    warped::Simulation forest_sim {"Forest Simulation", argc, argv, args};

    /* Read the vegetation map */
    FILE *fp = fopen(vegetation_map.c_str(), "rb");
    if (!fp) throw "Incorrect name of vegetation map";

    /* Read the 54-byte header */
    unsigned char info[54];
    auto size = fread(info, sizeof(unsigned char), 54, fp);
    if (size != 54) assert(0);

    /* Extract image height and width from header */
    unsigned int width  = 0, height = 0;
    memcpy( &width, &info[18], sizeof(unsigned int) );
    memcpy( &height, &info[22], sizeof(unsigned int) );

    unsigned int row_padded = (width*3 + 3) & (~3);
    unsigned char *data = new unsigned char[row_padded];

    /* Combustible map can have a value [0, 255], higher value means more combustible */
    unsigned char **combustible_map = new unsigned char*[height];

    for (unsigned int i = 0; i < height; i++) {

        combustible_map[i] = new unsigned char[width];
        size = fread(data, sizeof(unsigned char), row_padded, fp);
        if (size != row_padded) assert(0);

        for(unsigned int j = 0; j < width*3; j += 3) {

            /* blue = data[j], green = data[j+1], red = data[j+2] */
            /* Combustion weighted function - red > green >> blue */
            combustible_map[i][j/3] = (data[j] + 8*data[j+1] + 9*data[j+2] ) / 18;

            /* Set combustion index to 0 if combustion index is low */
            /* OR combustion index and blue are both high - indicative of white areas */
            if ( (combustible_map[i][j/3] < 50) ||
                    (combustible_map[i][j/3] > 200  && data[j] > 200) ) {
                combustible_map[i][j/3] = 0;
            }
        }
    }
    fclose(fp);

    for(unsigned int row = 0; row < height/2; ++row){
        for(unsigned int column = 0; column < width; ++column){
            // each column from the first half is swapped
            // with it correspondent from the second half
            unsigned char tmp = combustible_map[row][column];
            combustible_map[row][column] = combustible_map[height - row - 1][column];
            combustible_map[height - row - 1][column] = tmp;
        }
    }
    


    /* Verify the combustion index visually */
    FILE *image = fopen("combustion_index.pgm", "wb");
    fprintf(image, "P5\n %s\n %d\n %d\n %d\n", "# Filtered Vegetation Map", width, height, 255);

    for(unsigned int i = 0; i<height; i++){
        for(unsigned int j = 0; j<width; j++){
            fwrite(&combustible_map[i][j], sizeof(unsigned char), 1, image);
        }
    }

    fclose(image);


    /* Create the LPs */
    std::vector<Forest> lps;
    for (unsigned int i = 0; i < height; i++) {
        for (unsigned int j = 0; j < width; j++) {

            if (!combustible_map[i][j]) continue;

            /* Placeholder equations for threshold calculation */
            unsigned int ignition_threshold = (unsigned int) combustible_map[i][j];
            unsigned int peak_threshold     = ignition_threshold * PEAK_TO_IGN_THRES_RATIO;
 
            /* Impart the initial heat content */
            unsigned int heat_content = INITIAL_HEAT_CONTENT;
            /* Heat content at fire's origin equals ignition point */
            if ( (i == fire_origin_x) && (j == fire_origin_y) ){
                heat_content = ignition_threshold;
            }

            unsigned int index = i*width + j;
            lps.emplace_back(   width,
                                height,
                                combustible_map,
                                ignition_threshold,
                                heat_rate,
                                peak_threshold,
                                radiation_fraction,
                                burnout_threshold,
                                heat_content,
                                index
                            );
        }
    }

    std::vector<warped::LogicalProcess*> lp_pointers;
    for (auto& lp : lps) {
        lp_pointers.push_back(&lp);
    }
    forest_sim.simulate(lp_pointers);

    /* Post-simulation model statistics */
    unsigned int cells_burnt_cnt = 0;
    for(auto& lp: lps){
        if( lp.state_.burn_status_ == BURNT_OUT ) {
            cells_burnt_cnt++;
        }
    }
    std::cout << "Total cells burnt = " << cells_burnt_cnt << std::endl;

    return 0;

}
