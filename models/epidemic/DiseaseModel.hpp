#ifndef DISEASE_MODEL_HPP
#define DISEASE_MODEL_HPP

#include <cmath>
#include "Person.hpp"
#include "RandomNumGenerator.hpp"

#define PROB_MULTIPLIER 100

using namespace std;

class DiseaseModel {
public:

    DiseaseModel() = default;

    DiseaseModel(float transmissibility, 
                    unsigned int latent_dwell_interval, 
                    unsigned int incubating_dwell_interval, 
                    unsigned int infectious_dwell_interval, 
                    unsigned int asympt_dwell_interval, 
                    float latent_infectivity, float incubating_infectivity, 
                    float infectious_infectivity, float asympt_infectivity, 
                    float prob_ulu, float prob_ulv, float prob_urv, 
                    float prob_uiv, float prob_uiu, unsigned int seed) 
            : transmissibility_(transmissibility), 
                latent_dwell_interval_(latent_dwell_interval), 
                incubating_dwell_interval_(incubating_dwell_interval), 
                infectious_dwell_interval_(infectious_dwell_interval), 
                asympt_dwell_interval_(asympt_dwell_interval), 
                latent_infectivity_(latentInfectivity), 
                incubating_infectivity_(incubating_infectivity), 
                infectious_infectivity_(infectious_infectivity), 
                asympt_infectivity_(asympt_infectivity), 
                prob_ulu_(prob_ulu), prob_ulv_(prob_ulv), 
                prob_urv_(prob_urv), prob_uiv_(prob_uiv), 
                prob_uiu_(prob_uiu) {

        rand_num_gen_ = make_unique<RandomNumGenerator>(seed);
    }

    void reaction(std::map<unsigned int, std::shared_ptr<Person>> population, 
                                                    unsigned int current_time) {

        unsigned int uninfected_num = 0, latent_num = 0, incubating_num = 0, 
                            infectious_num = 0, asympt_num = 0, recovered_num = 0;
        std::vector<std::shared_ptr<Person>> uninfected_population;

        for (auto entry : population) {
            ptts(entry.second, current_time);
            if ((entry.second)->infection_state_ == infection_state_t::UNINFECTED) {
                uninfected_population.push_back(entry.second);
                uninfected_num++;
            } else if ((entry.second)->infection_state_ == infection_state_t::LATENT) {
                latent_num++;
            } else if ((entry.second)->infection_state_ == infection_state_t::INCUBATING) {
                incubating_num++;
            } else if ((entry.second)->infection_state_ == infection_state_t::INFECTIOUS) {
                infectious_num++;
            } else if ((entry.second)->infection_state_ == infection_state_t::ASYMPT) {
                asympt_num++;
            } else {
                recovered_num++;
            }
        }

        if (uninfected_population.size()) {
            for (auto person : uninfected_population) {
                double susceptibility = person->susceptibility_;
                double suscep_cross_trans = (double) (susceptibility * transmissibility_);
                double prob_latent = 1.0, prob_incubating = 1.0, 
                            prob_infectious = 1.0, prob_asympt = 1.0, disease_prob = 1.0;
                if (latent_num) {
                    prob_latent -= (double) (suscep_cross_trans * latent_infectivity_);
                    prob_latent = pow(prob_latent, (double) latent_num);
                }
                if (incubating_num) {
                    prob_incubating -= (double) (suscep_cross_trans * incubating_infectivity_);
                    prob_incubating = pow(prob_incubating, (double) incubating_num);
                }
                if (infectious_num) {
                    prob_infectious -= (double) (suscep_cross_trans * infectious_infectivity_);
                    prob_infectious = pow(prob_infectious, (double) infectious_num);
                }
                if (asympt_num) {
                    prob_asympt -= (double) (suscep_cross_trans * asympt_infectivity_);
                    prob_asympt = pow(prob_asympt, (double) asympt_num);
                }

                double prod_prob = prob_latent * prob_incubating 
                                        * prob_infectious * prob_asympt;
                disease_prob -= 
                    pow(prod_prob, (double) (current_time - person->loc_arrival_timestamp_));

                unsigned int disease_num = (unsigned int) disease_prob * PROB_MULTIPLIER;
                if (disease_num > rand_num_gen_->generateRandNum(PROB_MULTIPLIER)) {
                    unsigned int rand_num = rand_num_gen_->generateRandNum(PROB_MULTIPLIER);

                    if (person->vaccination_status_) {
                        unsigned int ulv_num = (unsigned int) (prob_ulv_ * PROB_MULTIPLIER);
                        unsigned int urv_plus_ulv_num = 
                            (unsigned int) ((prob_urv_ + prob_ulv_) * PROB_MULTIPLIER);
                        if (ulv_num > rand_num) {
                            person->infection_state_ = infection_state_t::LATENT;
                        } else if (urv_plus_ulv_num > rand_num) {
                            person->infection_state_ = infection_state_t::RECOVERED;
                        } else {
                            person->infection_state_ = infection_state_t::INCUBATING;
                        }
                    } else {
                        unsigned int ulu_num = (unsigned int) (prob_ulu_ * PROB_MULTIPLIER);
                        if (ulu_num > rand_num) {
                            person->infection_state_ = infection_state_t::LATENT;
                        } else {
                            person->infection_state_ = infection_state_t::INCUBATING;
                        }
                    }
                }
            }
        }
    }

private:

    void ptts(std::shared_ptr<Person> person, unsigned int current_time) {

        if (person->infection_state_ == infection_state_t::LATENT) {
            if ((current_time - person->prev_state_change_timestamp_) 
                                            >= latent_dwell_interval_) {
                person->infection_state_ = infection_state_t::INFECTIOUS;
                person->prev_state_change_timestamp_ += latent_dwell_interval_;
            }
        }
        if (person->infection_state_ == infection_state_t::INCUBATING) {
            if ((current_time - person->prev_state_change_timestamp_) 
                                        >= incubating_dwell_interval_) {
                person->infection_state_ = infection_state_t::ASYMPT;
                person->prev_state_change_timestamp_ += incubating_dwell_interval_;
            }
        }
        if (person->infection_state_ == infection_state_t::INFECTIOUS) {
            if ((current_time - person->prev_state_change_timestamp_) 
                                        >= infectious_dwell_interval_) {
                person->infection_state_ = infection_state_t::RECOVERED;
                person->prev_state_change_timestamp_ += infectious_dwell_interval_;
            }
        }
        if (person->infection_state_ == infection_state_t::ASYMPT) {
            if ((current_time - person->prev_state_change_timestamp_) 
                                            >= asympt_dwell_interval_) {
                person->infectionState = infection_state_t::RECOVERED;
                person->prev_state_change_timestamp_ += asympt_dwell_interval_;
            }
        }
        if ((person->infection_state_ == infection_state_t::UNINFECTED) || 
                (person->infection_state_ == infection_state_t::RECOVERED)) {
            person->prev_state_change_timestamp_ = 0;
        }
    }

    float transmissibility_;
    unsigned int latent_dwell_interval_;
    unsigned int incubating_dwell_interval_;
    unsigned int infectious_dwell_interval_;
    unsigned int asympt_dwell_interval_;
    float latent_infectivity_;
    float incubating_infectivity_;
    float infectious_infectivity_;
    float asympt_infectivity_;
    float prob_ulu_;
    float prob_ulv_;
    float prob_urv_;
    float prob_uiv_;
    float prob_uiu_;
    std::unique_ptr<RandomNumGenerator> rand_num_gen_;
};

#endif
