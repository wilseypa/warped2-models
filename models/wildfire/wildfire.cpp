/* Implementation of a Wildfire Simulation */

#include <cassert>
#include <locale>
#include "wildfire.hpp"
#include "ppm/ppm.hpp"
#include "tclap/ValueArg.h"
#include <cmath>

/* Combustion parameter */
#define ORIGIN_RADIUS   1

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(CellEvent)

std::vector<std::shared_ptr<warped::Event> > Cell::initializeLP() {

    std::vector<std::shared_ptr<warped::Event> > events;

    /* If heat content exceeds ignition threshold, schedule ignition */
    if (state_.heat_content_ >= ignition_threshold_) {
        events.emplace_back( new CellEvent{lp_name(index_), IGNITION, 0, 1} );
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
                unsigned int ts = received_event.ts_ + 1;
                response_events.emplace_back(
                        new CellEvent{lp_name(index_), IGNITION, 0, ts} );
            }
        } break;

        case RADIATION_TIMER: {

            /* Check if cell is about to burn out */
            if (state_.heat_content_ <= burnout_threshold_ + DIRECTION_MAX*radiation_rate_) {
                state_.burn_status_  = BURNT_OUT;
                state_.heat_content_ = burnout_threshold_;

            } else {
                state_.heat_content_ -= DIRECTION_MAX*radiation_rate_;

                /* Schedule the Radiation Timer */
                unsigned int ts = received_event.ts_ + 1;
                response_events.emplace_back( new CellEvent{lp_name(index_), RADIATION_TIMER, 0, ts} );
            }

            /* Schedule radiation events for surrounding LPs */
            unsigned int ts = received_event.ts_ + 1;
            for (unsigned int direction = NORTH; direction < DIRECTION_MAX; direction++) {
                if (!connection_[direction]) continue;
                response_events.emplace_back(
                            new CellEvent{find_cell( (direction_t)direction ), 
                                    RADIATION, radiation_rate_, ts} );
            }
        } break;

        case IGNITION: {

            state_.burn_status_= BURNING;

            /* Schedule Peak Event */
            unsigned int ts = received_event.ts_ +
                        ((peak_threshold_ - ignition_threshold_) / growth_rate_);
            response_events.emplace_back( new CellEvent{lp_name(index_), PEAK, 0, ts} );
        } break;

        case PEAK: {

            state_.heat_content_ += peak_threshold_ - ignition_threshold_;

            /* Schedule the Radiation Timer */
            unsigned int ts = received_event.ts_ + 1;
            response_events.emplace_back( new CellEvent{lp_name(index_), RADIATION_TIMER, 0, ts} );
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
    std::string  vegetation_map      = "test_map_4.ppm";
    unsigned int radiation_rate      = 100;
    unsigned int ambient_heat        = 20;
    unsigned int peak_threshold      = 1000;
    unsigned int burnout_threshold   = 100;
    unsigned int fire_origin_row     = 15;
    unsigned int fire_origin_col     = 15;

    /* Read any simulation arguments (if provided) */
    TCLAP::ValueArg<std::string> vegetation_map_arg( "m", "vegetation-map", "Vegetation map",
                                                    false, vegetation_map, "string" );

    TCLAP::ValueArg<unsigned int> radiation_rate_arg( "r", "radiation-rate",
                                                    "Heat radiation in a direction per unit time",
                                                    false, radiation_rate, "unsigned int" );

    TCLAP::ValueArg<unsigned int> ambient_heat_arg( "a", "ambient-heat",
                                                    "Ambient heat in a cell",
                                                    false, ambient_heat, "unsigned int" );

    TCLAP::ValueArg<unsigned int> peak_threshold_arg( "p", "peak-threshold",
                                                    "Peak heat content of a cell",
                                                    false, peak_threshold, "unsigned int" );

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
                                        &radiation_rate_arg,
                                        &ambient_heat_arg,
                                        &peak_threshold_arg,
                                        &burnout_threshold_arg,
                                        &fire_origin_row_arg,
                                        &fire_origin_col_arg
                                    };

    warped::Simulation wildfire_sim {"Wildfire Simulation", argc, argv, args};
 
    vegetation_map      = vegetation_map_arg.getValue();
    radiation_rate      = radiation_rate_arg.getValue();
    ambient_heat        = ambient_heat_arg.getValue();
    peak_threshold      = peak_threshold_arg.getValue();
    burnout_threshold   = burnout_threshold_arg.getValue();
    fire_origin_row     = fire_origin_row_arg.getValue();
    fire_origin_col     = fire_origin_col_arg.getValue();

    /* Ensure that peak and burnout threshold entries are valid */
    assert( peak_threshold > burnout_threshold );

    /* Ensure that radiation_rate is >= ambient heat */
    assert( radiation_rate >= ambient_heat );

    /* Read the vegetation map */
    auto vegetation = new ppm();
    vegetation->read(vegetation_map);
    unsigned int num_rows = vegetation->height;
    unsigned int num_cols = vegetation->width;

    /* Ensure that fire origin row and column is valid */
    assert( fire_origin_row < num_rows );
    assert( fire_origin_col < num_cols );

    /* Populate the combustion map */
    unsigned char **combustible_map = new unsigned char*[num_rows];
    for (unsigned int i = 0; i < vegetation->size; i++) {
        unsigned int row = i / num_cols;
        unsigned int col = i % num_cols;
        if (!col) {
            combustible_map[row] = new unsigned char[num_cols];
        }

        /* Note :
            1. Combustibility proportional to Combustion Index(CI)
            2. Highly combustible cells will have high CI (max value = 255)
            3. CI = 0 means non-combustible

            Categorize the rest using RGB to YCbCr conversion functions
            a. black, blue, white, pink do not burn
            b. green is less likely to burn
            c. yellow/orange is likely to burn
            d. red is very likely to burn
         */
        unsigned int Y  =   0.299*(double)vegetation->r[i]
                          + 0.587*(double)vegetation->g[i]
                          + 0.114*(double)vegetation->b[i];

        unsigned int Cb =   128
                          - 0.169*(double)vegetation->r[i]
                          - 0.331*(double)vegetation->g[i]
                          + 0.500*(double)vegetation->b[i];

        unsigned int Cr =   128
                          + 0.500*(double)vegetation->r[i]
                          - 0.419*(double)vegetation->g[i]
                          - 0.081*(double)vegetation->b[i];

        if (Y < 10 || Y > 240 || Cb > 128) {
            combustible_map[row][col] = 0;

        } else {
            combustible_map[row][col] = Cr;
        }
    }
    delete vegetation;

    /*  Verify the combustion index visually
            Black         -> non-vegetation
            White to Grey -> high to low combustibility
     */
    std::ofstream ofs( "filtered_vegetation_map.pgm",
           std::ios_base::out | std::ios_base::binary | std::ios_base::trunc );
    if (!ofs) assert(0);
    ofs << "P5\n" << num_cols << " " << num_rows << "\n255\n";
    for (unsigned int i = 0; i < num_rows; i++) {
        ofs.write( reinterpret_cast<const char*>( combustible_map[i]), num_cols );
    }
    ofs.close();

    /* Create the LPs */
    std::vector<Cell> lps;
    for (unsigned int i = 0; i < num_rows; i++) {
        for (unsigned int j = 0; j < num_cols; j++) {

            if (!combustible_map[i][j]) continue;

            /* Ignition threshold = 500 - CI */
            unsigned int ignition_threshold = 500 - combustible_map[i][j];
            assert( ignition_threshold < peak_threshold );

            /* Note : Initial heat content of a cell equals ambient heat content*/
            unsigned int heat_content = ambient_heat;

            /* Growth Rate */
            unsigned int growth_rate = (combustible_map[i][j]+3) / 3;

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
                                growth_rate,
                                peak_threshold,
                                radiation_rate,
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
    unsigned int lp_id = 0;

    for (unsigned int i = 0; i < status_map->size; i++) {

        /* Post-wildfire status color codes :
             1. non-vegetation  -> black
             2. burnt-out cells -> white
             3. burning cells   -> red
             4. unburnt cells   -> light yellow (highly combustible)
                                -> green (not easily combustible)
         */
        if (!combustible_map[i/num_cols][i%num_cols]) continue;

        auto lp = lps[lp_id++];
        auto status = lp.state_.burn_status_;

        if (status == BURNT_OUT) {
            cells_burnt_cnt++;
            status_map->r[i] = 255;
            status_map->g[i] = 255;
            status_map->b[i] = 255;

        } else if (status == BURNING) {
            cells_burning_cnt++;
            status_map->r[i] = 255;

        } else { /* status == UNBURNT */
            status_map->r[i] = combustible_map[i/num_cols][i%num_cols];
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
