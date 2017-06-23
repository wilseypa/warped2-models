/* Implementation of a Wildfire Simulation */

#include <cassert>
#include "wildfire.hpp"
#include "ppm.hpp"
#include "tclap/ValueArg.h"

/* Event timer delays */
#define IGNITION_DELAY              1
#define RADIATION_DELAY             1
#define RADIATION_INTERVAL          5

/* Combustion parameters */
#define PEAK_TO_IGN_THRES_RATIO     3
#define INITIAL_HEAT_CONTENT        20
#define ORIGIN_RADIUS               1

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(CellEvent)

std::vector<std::shared_ptr<warped::Event> > Cell::initializeLP() {

    std::vector<std::shared_ptr<warped::Event> > events;

    /* If heat content exceeds ignition threshold, schedule ignition */
    if (state_.heat_content_ >= ignition_threshold_) {
        events.emplace_back( new CellEvent{lp_name(index_), IGNITION, 0, IGNITION_DELAY} );
    }
    return events;
}

inline std::string Cell::lp_name(const unsigned int lp_index){

    return std::string("Cell_") + std::to_string(lp_index);
}

std::vector<std::shared_ptr<warped::Event> > Cell::receiveEvent(const warped::Event& event) {

    std::vector<std::shared_ptr<warped::Event> > response_events;
    auto received_event = static_cast<const CellEvent&>(event);

    switch (received_event.type_) {

        case RADIATION: {

            if (this->state_.burn_status_ == BURNT_OUT) break;

            state_.heat_content_ += received_event.heat_content_;

            /* If heat exceeds ignition threshold and vegtation is unburnt, schedule ignition */
            if ( (state_.burn_status_ == UNBURNT) && 
                    (state_.heat_content_ >= ignition_threshold_) ) {
                unsigned int next_ts = received_event.ts_ + IGNITION_DELAY;
                response_events.emplace_back(
                        new CellEvent{lp_name(index_), IGNITION, 0, next_ts} );
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
                            new CellEvent{find_cell( (direction_t)direction ), 
                                    RADIATION, heat_radiated_out/DIRECTION_MAX, next_ts} );
            }

            /* Check if cell has burnt out */
            if (state_.heat_content_ <= burnout_threshold_) {
                state_.burn_status_ = BURNT_OUT;

            } else { /* Else schedule next radiation */
                next_ts = received_event.ts_ + RADIATION_INTERVAL;
                response_events.emplace_back(
                        new CellEvent{lp_name(index_), RADIATION_TIMER, 0, next_ts} );
            }
        } break;

        case IGNITION: {

            state_.burn_status_= BURNING;

            /* Schedule Peak Event */
            unsigned int peak_time = received_event.ts_ + 
                        ((peak_threshold_ - ignition_threshold_) / heat_rate_);
            response_events.emplace_back( new CellEvent{lp_name(index_), PEAK, 0, peak_time} );
        } break;

        case PEAK: {

            state_.heat_content_ += peak_threshold_ - ignition_threshold_;

            /* Schedule first Radiation Timer */
            response_events.emplace_back( 
                    new CellEvent{lp_name(index_), RADIATION_TIMER, 0, received_event.ts_} );
        } break;
    }
    return response_events;
}

std::string Cell::find_cell( direction_t direction ) {

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

bool Cell::neighbor_conn( direction_t direction, unsigned char **combustible_map ) {

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
    std::string     vegetation_map      = "test_vegetation_map.ppm";
    unsigned int    heat_rate           = 15;
    double          radiation_fraction  = 0.05;
    unsigned int    burnout_threshold   = INITIAL_HEAT_CONTENT;
    unsigned int    fire_origin_x       = 260;
    unsigned int    fire_origin_y       = 260;

    /* Read any simulation arguments (if provided) */
    TCLAP::ValueArg<std::string> vegetation_map_arg( "m", "vegetation-map", "Vegetation map",
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

    warped::Simulation wildfire_sim {"Wildfire Simulation", argc, argv, args};

    /* Read the vegetation map */
    auto vegetation = new ppm();
    vegetation->read(vegetation_map);
    unsigned int size_y = vegetation->width;
    unsigned int size_x = vegetation->height;

    /* Populate the combustion map */
    unsigned char **combustible_map = new unsigned char*[size_x];
    for (unsigned int i = 0; i < vegetation->size; i++) {
        unsigned int row = i / size_y;
        unsigned int col = i % size_y;
        if (!col) {
            combustible_map[row] = new unsigned char[size_y];
        }

        /*  Filter unwanted pixels by setting combustion index to 0 if :
                1. pixel is black
                2. pixel is white
                3. pixel indiates rocks i.e. highly reddish
                4. pixel indicates other non-vegetative features suchas pink for houses
                5. pixel indicates water i.e. highly blue
            Else,
                Use <Red,Green> weighted function to calculate combustion index
         */
        if (        (vegetation->r[i] < 20 ) &&
                    (vegetation->g[i] < 20 ) &&
                    (vegetation->b[i] < 20 )    ) {
            combustible_map[row][col] = 0;
        
        } else if ( (vegetation->r[i] > 200) &&
                    (vegetation->g[i] > 200) &&
                    (vegetation->b[i] > 200)    ) {
            combustible_map[row][col] = 0;

        } else if ( (vegetation->r[i] > 200) &&
                    (vegetation->g[i] < 20 ) &&
                    (vegetation->b[i] < 20 )    ) {
            combustible_map[row][col] = 0;

        } else if ( (vegetation->r[i] > 200) &&
                    (vegetation->g[i] < 20 ) &&
                    (vegetation->b[i] > 200)    ) {
            combustible_map[row][col] = 0;

        } else if ( (vegetation->r[i] < 20 ) &&
                    (vegetation->g[i] < 20 ) &&
                    (vegetation->b[i] > 200)    ) {
            combustible_map[row][col] = 0;

        } else {
            combustible_map[row][col] = (9*vegetation->r[i] + 7*vegetation->g[i]) / 16;
        }
    }
    delete vegetation;

    /* Verify the combustion index visually */
    std::ofstream ofs( "filtered_vegetation_map.pgm",
           std::ios_base::out | std::ios_base::binary | std::ios_base::trunc );
    if (!ofs) assert(0);
    ofs << "P5\n" << size_y << " " << size_x << "\n255\n";
    for (unsigned int i = 0; i < size_x; i++) {
        ofs.write( reinterpret_cast<const char*>(combustible_map[i]), size_y );
    }
    ofs.close();

    /* Create the LPs */
    std::vector<Cell> lps;
    for (unsigned int i = 0; i < size_x; i++) {
        for (unsigned int j = 0; j < size_y; j++) {

            if (!combustible_map[i][j]) continue;

            /* Placeholder equations for threshold calculation */
            unsigned int ignition_threshold = (unsigned int) combustible_map[i][j];
            unsigned int peak_threshold     = ignition_threshold * PEAK_TO_IGN_THRES_RATIO;
 
            /* Impart the initial heat content */
            unsigned int heat_content = INITIAL_HEAT_CONTENT;

            /* Heat content at fire's origin equals ignition point */
            /* Origin of fire is a square patch whose center is a configurable parameter */
            unsigned int diff_x = std::abs(i-fire_origin_x);
            unsigned int diff_y = std::abs(j-fire_origin_y);
            if ( (diff_x <= ORIGIN_RADIUS) && (diff_y <= ORIGIN_RADIUS) ) {
                heat_content = ignition_threshold;
            }

            unsigned int index = i*size_y + j;
            lps.emplace_back(   size_x,
                                size_y,
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
    wildfire_sim.simulate(lp_pointers);

    /* Post-simulation model statistics */
    unsigned int cells_burnt_cnt = 0;
    for (auto& lp: lps) {
        if (lp.state_.burn_status_ == BURNT_OUT) {
            cells_burnt_cnt++;
        }
    }
    std::cout << "Total cells burnt = " << cells_burnt_cnt << std::endl;

    return 0;
}
