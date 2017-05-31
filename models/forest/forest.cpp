//Implementation of a Forest Fire Simulation

#include <cassert>
#include <random>
#include "forest.hpp"
#include "tclap/ValueArg.h"

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(ForestEvent)

std::vector<std::shared_ptr<warped::Event> > Forest::initializeLP() {

    std::vector<std::shared_ptr<warped::Event> > events;

    for (unsigned int i = 0; i < this->index_; i++){ //For all of the cells in the forest
        if(this->state.heat_content_ >= this->ignition_threshold){ //If heat content > ignition threshold
            events.emplace_back(new ForestEvent {this->name_, IGNITION,ts_} // Then start an ignition event
        }
    }
    return events;
}


inline std::string Forest::lp_name(const unsigned int lp_index){

    return std::string("Forest_") + std::to_string(lp_index);
}

std::vector<std::shared_ptr<warped::Event> > Forest::receiveEvent(const warped::Event& event) {

    std::vector<std::shared_ptr<warped::Event> > response_events;
    auto received_event = static_cast<const ForestEvent&>(event);

    switch (received_event.type_) {

        case RADIATION: {
            if(this->state_.burn_status_==BURNT_OUT){
            break
            }
            this->state_.heat_content_=this->state_.heat_content_ + recieved_event.heat_content;
            // if there is enough heat and the vegtation is unburnt Schedule ignition 
            if(this->state_.heat_content_ >= this->ignition_threshold && 
                                                            this->state_burn_status == UNBURNT){
            unsigned int ignition_time = recieved_event.ts+1;
            response_events.emplace_back(new ForestEvent {this->name, IGNITION, ignition_time });
            }
            break;
        }

        case RADIATION_TIMER: {
            unsigned int radiation_heat=this->state_.heat_content_ /100 * 5
            this->state_.heat_content_ /100 * 95;
            // Schedule Radiation events for each of the eight surrounding LPs

            /*begin for loop*/
            unsigned int radiation_time = received_event.ts_ + 1;
            response_events.emplace_back(new ForestEvent { name, RADIATION,
                                                                            radiation_time });
            /*end for loop*/
            if(this->state_.heat_content_ <= this->burnout_threshold){
            this->state_.burn_status_ = BURNT_OUT
            }
            else{
            unsigned int radiation_timer = recieved_event.ts + 5;
            response_events.emplace_back(new ForestEvent {this->name_, RADIATION_TIMER,
                                                                           radiation_timer });
            }
            break;
        }
        case IGNITION: {
            this->state_.burn_status_=GROWTH;
            // Schedule Peak Event
            unsigned int peak_time = received_event.ts + ((this->peak_threshold-this->ignition_threshold)/this->heat_rate);
            response_events.emplace_back(new ForestEvent {this->name_, PEAK, peak_time });
            break;
        }
        case PEAK: {
            this->state_.burn_status_=DECAY;
            this->state_.heat_content_=this->state_.heat_content_ + (this->peak_threshold - this->ignition_threshold);
            // Schedule first Radiation Timer
            unsigned int radiation_timer = recieved_event.ts + 5;
            response_events.emplace_back(new ForestEvent {this->name_, RADIATION_TIMER, 
                                                                          radiation_timer });
            break;
        }
    }
    return response_events;
}


unsigned char *read_bmp( std::string img_name, unsigned int heat_rate,
                            unsigned int radiation_percent, unsigned int burnout_threshold
                                        unsigned int burn_start_x, unsigned int burn_start_y){

    FILE *fp = fopen(img_name.c_str(), "rb");
    if(!fp) throw "Argument Exception";

    // Read the 54-byte header
    unsigned char info[54];
    fread(info, sizeof(unsigned char), 54, fp);

    // Extract image height and width from header
    width  = *(unsigned int *)&info[18];
    height = *(unsigned int *)&info[22];

    std::cout << "Width  : " << width    << std::endl;
    std::cout << "Height : " << height   << std::endl;

    unsigned int row_padded = (width*3 + 3) & (~3);
    unsigned char *data = new unsigned char[row_padded];



    for( unsigned int i = 0; i < height; i++ ) {
        fread(data, sizeof(unsigned char), row_padded, fp);
        for(unsigned int j = 0; j < width*3; j += 3) {
            //std::cout   << "B: "<< (int)data[j] 
            //            << " G: " << (int)data[j+1]
            //            << " R: " << (int)data[j+2]
            //            << std::endl;
            unsigned int index_num = i*j; 
            //Placeholder equations for threshold calculation
            unsigned int ignition_threshold = (int)data[j] + (int)data[j+1] + (int)data[j+2];
            unsigned int peak_threshold = ((int)data[j] + (int)data[j+1] + (int)data[j+2]) * 2;
            

            std::string name = Forest::lp_name(index_num)
            lps.emplace_back(name, width, height,ignition_threshold, heat_rate, 
                                     peak_threshold, burnout_threshold, index_num);
 
            /* If the LP being created is the start of the fire then give it intial heat content */
            if(i == burn_start_x && j == burn_start_y){
                this->state.heat_content_ = 400
            }
        }
   }
   fclose(fp);
   return data;
}

std::string Forest::compute_spread(direction_t direction) {

    unsigned int new_x = 0, new_y = 0;
    unsigned int current_y = index_ / size_x_;
    unsigned int current_x = index_ % size_x_;

    switch (direction) {
 
       case NORTH: {
           new_x = current_x;
           new_y = (current_y + 1) % size_y_;
       } break;

       case NORTH_EAST: {
           new_x = (current_x + 1) % size_x_;
           new_y = (current_y + 1) % size_y_;
       } break;
                                                                
       case EAST: {
           new_x = (current_x + 1) % size_x_;
           new_y = current_y;
       } break;
                                                                                
       case SOUTH_EAST: {
           new_x = (current_x + 1) % size_x_;
           new_y = (current_y + size_y_ - 1) % size_y_;
       } break;

       case SOUTH: {
           new_x = current_x;
           new_y = (current_y + size_y_ - 1) % size_y_;
       } break;

       case SOUTH_WEST: {
           new_x = (current_x + size_x_ - 1) % size_x_;
           new_y = (current_y + size_y_ - 1) % size_y_;
       } break;

       case WEST: {
           new_x = (current_x + size_x_ - 1) % size_x_;
           new_y = current_y; 
       } break;

       case NORTH_WEST: {
           new_x = (current_x + size_x_ - 1) % size_x_;
           new_y = (current_y + 1) % size_y_;
       } break;
       
       default: {
           std::cerr << "Invalid move direction " << direction << std::endl;
           assert(0);
       }
       }

       return lp_name(new_x + new_y * size_x_);
}


int main(int argc, char *argv[],){
   
    std::string config_filename = "map_hawaii.bmp";
    unsigned int heat_rate = 100;
    unsigned int radiation_percent = 5;
    unsigned int burnout_threshold = 50;
    unsigned int burn_start_x = 500;
    unsigned int burn_start_y = 501;

    
    TCLAP::ValueArg<std::string> config_arg("m", "map", "Forest model vegetation config", 
                                                                false, config_filename, "string");
    TCLAP::ValueArg<unsigned int> heat_rate_arg("h", "heat-rate", "Speed of growth of the fire",
                                                                false, heat_rate, "unsigned int");
    TCLAP::ValueArg<unsigned int> radition_percent_arg("r", "radiation-percent", 
             "Percent of Heat released every timstamp", false, radiation_percent, "unsigned int");
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
                                        &radiation_percent_arg, &burnout_threshold_arg,
                                                    &burn_start_x_arg, &burn_start_y_arg};
   
    config_filename = config_arg.getValue();
    heat_rate = heat_rate_arg.getValue();
    radiation_percent = radiation_percent_arg.getValue();
    burnout_threshold = burnout_threshold.getValue();
    burn_start_x = burn_start_x.getValue();
    burn_start_y = burn_start_y.getValue();

    warped::Simulation forest_sim {"Forest Simulation", argc, argv, args};

    std::vector<Forest> lps;
   
    (void) read_bmp(config_filename, heat_rate, radiation_percent, burnout_threshold, 
                                                                burn_start_x, burn_start_y);

    std::vector<warped::LogicalProcess*> lp_pointers;
    for (auto& lp : lps) {
        lp_pointers.push_back(&lp);
    }

    forest_sim.simulate(lp_pointers);

    

    
    return 0;

}
