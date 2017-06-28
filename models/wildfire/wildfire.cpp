/* Implementation of a Wildfire Simulation */

#include <cassert>
#include <locale>
#include "wildfire.hpp"
#include "ppm.hpp"
#include "tclap/ValueArg.h"

/* Event timer delays */
#define IGNITION_DELAY              1
#define RADIATION_DELAY             1
#define RADIATION_INTERVAL          5

/* Combustion parameters */
#define PEAK_TO_IGN_THRES_RATIO     10
#define INITIAL_HEAT_CONTENT        150
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
            unsigned int directional_heat_radiated_out =
                    (heat_radiated_out > DIRECTION_MAX) ? heat_radiated_out/DIRECTION_MAX : 1;
            state_.heat_content_ -= DIRECTION_MAX * directional_heat_radiated_out;

            /* Schedule radiation events for surrounding LPs */
            unsigned int next_ts = received_event.ts_ + RADIATION_DELAY;
            for (unsigned int direction = NORTH; direction < DIRECTION_MAX; direction++) {
                if (!connection_[direction]) continue;
                response_events.emplace_back(
                            new CellEvent{find_cell( (direction_t)direction ), 
                                    RADIATION, directional_heat_radiated_out, next_ts} );
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

    unsigned int new_row = 0, new_col = 0;
    unsigned int current_row = index_ / num_cols_;
    unsigned int current_col = index_ % num_cols_;

    switch( direction ) {

        case NORTH: {
            new_row = (current_row + num_rows_ - 1) % num_rows_;
            new_col = current_col;
        } break;

        case NORTH_EAST: {
            new_row = (current_row + num_rows_ - 1) % num_rows_;
            new_col = (current_col + 1) % num_cols_;
        } break;

        case EAST: {
            new_row = current_row;
            new_col = (current_col + 1) % num_cols_;
        } break;

        case SOUTH_EAST: {
            new_row = (current_row + 1) % num_rows_;
            new_col = (current_col + 1) % num_cols_;
        } break;

        case SOUTH: {
            new_row = (current_row + 1) % num_rows_;
            new_col = current_col;
        } break;

        case SOUTH_WEST: {
            new_row = (current_row + 1) % num_rows_;
            new_col = (current_col + num_cols_ - 1) % num_cols_;
        } break;

        case WEST: {
            new_row = current_row;
            new_col = (current_col + num_cols_ - 1) % num_cols_;
        } break;

        case NORTH_WEST: {
            new_row = (current_row + num_rows_ - 1) % num_rows_;
            new_col = (current_col + num_cols_ - 1) % num_cols_;
        } break;

        default: {
            std::cerr << "Invalid move direction " << direction << std::endl;
            assert(0);
        }
    }
    return lp_name( new_col + new_row * num_cols_ );
}

bool Cell::neighbor_conn( direction_t direction, unsigned char **combustible_map ) {

    unsigned int new_row = 0, new_col = 0;
    unsigned int current_row = index_ / num_cols_;
    unsigned int current_col = index_ % num_cols_;

    /* Eliminate the boundary situations */
    /* If LP is in the first row */
    if (!current_row) {

        /* NW, N and NE don't exist for the first row */
        if (direction == NORTH_WEST || direction == NORTH || direction == NORTH_EAST) {
            return false;
        }

        /* SW and W don't exist for LP at beginning of the first row */
        if (!current_col && (direction == SOUTH_WEST || direction == WEST)) {
            return false;
        }

        /* E and SE don't exist for LP at end of the first row */
        if ((current_col == num_cols_-1) && (direction == EAST || direction == SOUTH_EAST)) {
            return false;
        }
    }

    /* If LP is in the last row */
    if (current_row == num_rows_-1) {

        /* SE, S and SW don't exist for the last row */
        if (direction == SOUTH_EAST || direction == SOUTH || direction == SOUTH_WEST) {
            return false;
        }

        /* W and NW don't exist for LP at beginning of the last row */
        if (!current_col && (direction == WEST || direction == NORTH_WEST)) {
            return false;
        }

        /* NE and E don't exist for LP at end of the last row */
        if ((current_col == num_cols_-1) && (direction == NORTH_EAST || direction == EAST)) {
            return false;
        }
    }

    /* If LP is in the first column and not on the first or last rows */
    /* SW, W and NW don't exist for the first column */
    if (!current_col && 
            (direction == SOUTH_WEST || direction == WEST || direction == NORTH_WEST)) {
        return false;
    }

    /* If LP is in the last column and not on the first or last rows */
    /* NE, E and SE don't exist for the first column */
    if ((current_col == num_cols_-1) && 
            (direction == NORTH_EAST || direction == EAST || direction == SOUTH_EAST)) {
        return false;
    }

    /* Else LP is not looking for a neighbor outside of the grid limits */
    switch( direction ) {

        case NORTH: {
            new_row = current_row - 1;
            new_col = current_col;
        } break;

        case NORTH_EAST: {
            new_row = current_row - 1;
            new_col = current_col + 1;
        } break;

        case EAST: {
            new_row = current_row;
            new_col = current_col + 1;
        } break;

        case SOUTH_EAST: {
            new_row = current_row + 1;
            new_col = current_col + 1;
        } break;

        case SOUTH: {
            new_row = current_row + 1;
            new_col = current_col;
        } break;

        case SOUTH_WEST: {
            new_row = current_row + 1;
            new_col = current_col - 1;
        } break;

        case WEST: {
            new_row = current_row;
            new_col = current_col - 1;
        } break;

        case NORTH_WEST: {
            new_row = current_row - 1;
            new_col = current_col - 1;
        } break;

        default: {
            std::cerr << "Invalid move direction " << direction << std::endl;
            assert(0);
        }
    }

    /* If no LP exists for that grid position */
    if (!combustible_map[new_row][new_col]) return false;

    return true;
}


int main(int argc, char *argv[]) {

    /* Set the default values for the simulation arguments */
    std::string     vegetation_map      = "test_vegetation_map.ppm";
    unsigned int    heat_rate           = 15;
    double          radiation_fraction  = 0.5;
    unsigned int    burnout_threshold   = INITIAL_HEAT_CONTENT;
    unsigned int    fire_origin_row     = 700;
    unsigned int    fire_origin_col     = 600;

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

    TCLAP::ValueArg<unsigned int> fire_origin_row_arg( "R", "fire-origin-row",
                                                    "row index for origin of fire",
                                                    false, fire_origin_row, "unsigned int" );

    TCLAP::ValueArg<unsigned int> fire_origin_col_arg( "C", "fire-origin-col",
                                                    "column index for origin of fire",
                                                    false, fire_origin_col, "unsigned int" );

    std::vector<TCLAP::Arg*> args = {   &vegetation_map_arg,
                                        &heat_rate_arg,
                                        &radiation_fraction_arg,
                                        &burnout_threshold_arg,
                                        &fire_origin_row_arg,
                                        &fire_origin_col_arg
                                    };

    warped::Simulation wildfire_sim {"Wildfire Simulation", argc, argv, args};
 
    vegetation_map      = vegetation_map_arg.getValue();
    heat_rate           = heat_rate_arg.getValue();
    radiation_fraction  = radiation_fraction_arg.getValue();
    burnout_threshold   = burnout_threshold_arg.getValue();
    fire_origin_row     = fire_origin_row_arg.getValue();
    fire_origin_col     = fire_origin_col_arg.getValue();


    /* Read the vegetation map */
    auto vegetation = new ppm();
    vegetation->read(vegetation_map);
    unsigned int num_rows = vegetation->height;
    unsigned int num_cols = vegetation->width;

    /* Populate the combustion map */
    unsigned char **combustible_map = new unsigned char*[num_rows];
    for (unsigned int i = 0; i < vegetation->size; i++) {
        unsigned int row = i / num_cols;
        unsigned int col = i % num_cols;
        if (!col) {
            combustible_map[row] = new unsigned char[num_cols];
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
                    (vegetation->b[i] > 200)    ) {
            combustible_map[row][col] = 0;

        } else if ( (vegetation->r[i] < 20 ) &&
                    (vegetation->g[i] < 20 ) &&
                    (vegetation->b[i] > 200)    ) {
            combustible_map[row][col] = 0;

        } else {
            combustible_map[row][col] = (abs(12*vegetation->r[i] - 2 * vegetation->g[i])) / 14;
        }
    }
    delete vegetation;

    /* Verify the combustion index visually */
    std::ofstream ofs( "filtered_vegetation_map.pgm",
           std::ios_base::out | std::ios_base::binary | std::ios_base::trunc );
    if (!ofs) assert(0);
    ofs << "P5\n" << num_cols << " " << num_rows << "\n255\n";
    for (unsigned int i = 0; i < num_rows; i++) {
        ofs.write( reinterpret_cast<const char*>(combustible_map[i]), num_cols );
    }
    ofs.close();

    /* Create the LPs */
    std::vector<Cell> lps;
    for (unsigned int i = 0; i < num_rows; i++) {
        for (unsigned int j = 0; j < num_cols; j++) {

            if (!combustible_map[i][j]) continue;

            /* Placeholder equations for threshold calculation */
            unsigned int ignition_threshold = 255 - (unsigned int) combustible_map[i][j];
            unsigned int peak_threshold     = ignition_threshold * PEAK_TO_IGN_THRES_RATIO;

            /* Impart the initial heat content */
            unsigned int heat_content = INITIAL_HEAT_CONTENT;

            /* Heat content at fire's origin equals ignition point */
            /* Origin of fire is a square patch whose center is a configurable parameter */
            unsigned int diff_row = std::abs(i-fire_origin_row);
            unsigned int diff_col = std::abs(j-fire_origin_col);
            if ( (diff_row <= ORIGIN_RADIUS) && (diff_col <= ORIGIN_RADIUS) ) {
                heat_content = ignition_threshold;
            }

            unsigned int index = i*num_cols + j;
            lps.emplace_back(   num_rows,
                                num_cols,
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
    /* Also print the post-wildfire vegetation map */
    auto status_map = new ppm(num_cols, num_rows);
    unsigned int cells_burnt_cnt = 0, cells_burning_cnt = 0;
    for (auto& lp: lps) {
        auto status = lp.state_.burn_status_;
        auto i = lp.index_;
        if (status == BURNT_OUT) {
            cells_burnt_cnt++;
            status_map->r[i] = 255;
            status_map->g[i] = 255;
            status_map->b[i] = 255;

        } else if (status == BURNING) {
            cells_burning_cnt++;
            status_map->r[i] = 255;

        } else { /* status == UNBURNT */
            status_map->r[i] = abs(255 - lp.ignition_threshold_);
            status_map->g[i] = 200;
        }
    }
    status_map->write("post_wildfire_status.ppm");
    delete status_map;

    std::cout << "Wildfire Statistics :" << std::endl;

    std::cout.imbue(std::locale(""));
    std::cout << "\t" << lps.size() << " sq. units of vegetation found in an area of " 
                                    << num_rows * num_cols << " sq. units" << std::endl;

    float cells_burning_frac = static_cast<float>(cells_burning_cnt) / lps.size();
    std::cout << "\tVegetation currently burning \t= "
                                    << cells_burning_frac*100 << "%" << std::endl;

    float cells_burnt_frac = static_cast<float>(cells_burnt_cnt) / lps.size();
    std::cout << "\tVegetation already burnt \t= "
                                    << cells_burnt_frac*100 << "%" << std::endl;

    std::cout << std::endl;

    return 0;
}
