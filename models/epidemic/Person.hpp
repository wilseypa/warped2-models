#ifndef PERSON_HPP
#define PERSON_HPP

#define INVALID_PID 0

class Person {

public:
    Person(unsigned int pid, double susceptibility, std::string vaccination_status, 
            std::string infection_state, int arrival_time_at_loc, 
            int last_state_change_time) :
        pid_(pid), susceptibility_(susceptibility), vaccination_status_(vaccination_status),
        infection_state_(infection_state), arrival_time_at_loc_(arrival_time_at_loc),
        last_state_change_time_(last_state_change_time) {}

    /* PID */
    unsigned int pid_;

    /* Susceptibility */
    double susceptibility_;

    /* Vaccination status */
    std::string vaccination_status_;

    /* Infection state */
    std::string infection_state_;

    /* Time when the person arrived at the location */
    int arrival_time_at_loc_;

    /* Time when the last infection state change occured */
    int last_state_change_time_;
};

#endif
