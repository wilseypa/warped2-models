#ifndef PERSON_HPP
#define PERSON_HPP

enum infection_state_t {

    UNINFECTED,
    LATENT,
    INCUBATING,
    INFECTIOUS,
    ASYMPT,
    RECOVERED
};

class Person {
public:

    Person() = default;

    Person(unsigned int pid, double susceptibility, bool vacc_status, 
                infection_state_t infection_state, unsigned int arrival_timestamp, 
                unsigned int prev_state_change_timestamp)
        : pid_(pid), susceptibility_(susceptibility), vaccination_status_(vacc_status),
            infection_state_(infection_state), loc_arrival_timestamp_(arrival_timestamp),
            prev_state_change_timestamp_(prev_state_change_timestamp) {}

    unsigned int pid_;
    double susceptibility_;
    bool vaccination_status_;
    infection_state_t infection_state_;
    unsigned int loc_arrival_timestamp_;
    unsigned int prev_state_change_timestamp_;
};

#endif
