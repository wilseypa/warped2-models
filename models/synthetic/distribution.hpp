#ifndef DISTRIBUTION_HPP
#define DISTRIBUTION_HPP

#include <random> 

class Distribution {
public:
    virtual unsigned int nextTimeDelta (std::default_random_engine rng) = 0;

protected:
    unsigned int distri_return_;
    unsigned int floor_     = 0;
    unsigned int ceiling_   = 0;
};

class Geometric : public Distribution{
public:
    Geometric(std::string params, unsigned int floor_, unsigned int ceiling_) {

        p_ = std::stod(params);
        if (p_ <= 0 || p_ >= 1) {
            std::cerr << "Geometric Distribution: Invalid parameter." << std::endl;
            abort();
        }
    }

    unsigned int nextTimeDelta(std::default_random_engine rng) {

        std::geometric_distribution<int> distribution(p_);
        if (floor_ && !ceiling_){ /* if send time */
            do {
                distri_return_ = (unsigned int)std::max(distribution(rng), 1);
            }while (distri_return_ <= floor_);
        } else if (!floor_ && ceiling_) { // if select lp 
            do {
                distri_return_ = (unsigned int) std::max(distribution(rng), 1);
            } while ( distri_return_  < floor_ || distri_return_ > ceiling_);
        }
        return distri_return_;
    }

private:
    double p_ = 0.0;
};

class Exponential : public Distribution{
public:
    Exponential(std::string params, unsigned int floor_, unsigned int ceiling_) {

        lambda_ = std::stod(params);
        if (!lambda_) {
            std::cerr << "Exponential Distribution: Invalid parameter." << std::endl;
            abort();
        }
    }

    unsigned int nextTimeDelta(std::default_random_engine rng) {

        std::exponential_distribution<double> distribution(lambda_);
        if (floor_ && !ceiling_){ /* if send time */
            do {
                distri_return_ = (unsigned int)std::max(distribution(rng), 1.0);
            }while (distri_return_ <= floor_);
        } else if (!floor_ && ceiling_) { // if select lp 
            do {
                distri_return_ = (unsigned int) std::max(distribution(rng), 1.0);
            } while ( distri_return_  < floor_ || distri_return_ > ceiling_);
        }
        return distri_return_;
    }

private:
    double lambda_ = 0.0;
};

class Binomial : public Distribution {
public:
    Binomial (std::string params, unsigned int floor_, unsigned int ceiling_) {

        std::string delimiter = ",";
        size_t pos = params.find(delimiter);
        t_ = (unsigned int) std::stoul(params.substr(0,pos));
        p_ = std::stod(params.substr(pos+1));
        if (p_ <= 0 || p_ >= 1) {
            std::cerr << "Binomial Distribution: Invalid parameter." << std::endl;
            abort();
        }
    }
    /* binomial distribution returns within range of (0,t), return value + 1 to void returning 0 */
    unsigned int nextTimeDelta(std::default_random_engine rng) {

        std::binomial_distribution<int> distribution(t_, p_);
        if (floor_ && !ceiling_){ /* if send time */
            do {
                distri_return_ = (unsigned int)std::max(distribution(rng), 1);
            }while (distri_return_ <= floor_);
        } else if (!floor_ && ceiling_) { // if select lp 
            do {
                distri_return_ = (unsigned int) std::max(distribution(rng), 1);
            } while ( distri_return_  < floor_ || distri_return_ > ceiling_);
        }
        return distri_return_;
    }

private:
    unsigned int t_ = 0.0;
    double p_ = 0.0;
};

class Cauchy : public Distribution {
public:
    Cauchy (std::string params, unsigned int floor_, unsigned int ceiling_) {

        std::string delimiter = ",";
        size_t pos = params.find(delimiter);
        a_ = std::stod(params.substr(0,pos));
        b_ = std::stod(params.substr(pos+1));
        if (!a_ || !b_) {
            std::cerr << "Cauchy Distribution: Invalid parameter." << std::endl;
            abort();
        }
    }

    unsigned int nextTimeDelta(std::default_random_engine rng) {

        std::cauchy_distribution<double> distribution(a_, b_);
        if (floor_ && !ceiling_){ /* if send time */
            do {
                distri_return_ = (unsigned int)std::max(distribution(rng), 1.0);
            }while (distri_return_ <= floor_);
        } else if (!floor_ && ceiling_) { // if select lp 
            do {
                distri_return_ = (unsigned int) std::max(distribution(rng), 1.0);
            } while ( distri_return_  < floor_ || distri_return_ > ceiling_);
        }
        return distri_return_;
    }

private:
    double a_ = 0.0, b_ = 0.0;
};

class Chi_squared : public Distribution {
public:
    Chi_squared (std::string params, unsigned int floor_, unsigned int ceiling_) {

        n_ = std::stod(params);
        if (!n_) {
            std::cerr << "Chi Squared Distribution: Invalid parameter." << std::endl;
            abort();
        }
    }

    unsigned int nextTimeDelta(std::default_random_engine rng) {

        std::chi_squared_distribution<double> distribution(n_);
        if (floor_ && !ceiling_){ /* if send time */
            do {
                distri_return_ = (unsigned int)std::max(distribution(rng), 1.0);
            }while (distri_return_ <= floor_);
        } else if (!floor_ && ceiling_) { // if select lp 
            do {
                distri_return_ = (unsigned int) std::max(distribution(rng), 1.0);
            } while ( distri_return_  < floor_ || distri_return_ > ceiling_);
        }
        return distri_return_;
    }

private:
    double n_ = 0.0;
};

class Discrete : public Distribution {
public:
    Discrete (std::string params, unsigned int floor_, unsigned int ceiling_) {

        std::string delimiter = ",";
        size_t pos = params.find(delimiter);
        first_ = std::stod(params.substr(0,pos));
        last_ = std::stod(params.substr(pos+1));
        if (!first_ || !last_) {
            std::cerr << "Discrete Distribution: Invalid parameter." << std::endl;
            abort();
        }
    }

    unsigned int nextTimeDelta(std::default_random_engine rng) {

        std::discrete_distribution<int> distribution(first_, last_);
        if (floor_ && !ceiling_){ /* if send time */
            do {
                distri_return_ = (unsigned int)std::max(distribution(rng), 1);
            }while (distri_return_ <= floor_);
        } else if (!floor_ && ceiling_) { // if select lp 
            do {
                distri_return_ = (unsigned int) std::max(distribution(rng), 1);
            } while ( distri_return_  < floor_ || distri_return_ > ceiling_);
        }
        return distri_return_;
    }

private:
    double first_ = 0.0, last_ = 0.0;
};

class Fisher_f : public Distribution {
public:
    Fisher_f (std::string params, unsigned int floor_, unsigned int ceiling_) {

        std::string delimiter = ",";
        size_t pos = params.find(delimiter);
        m_ = std::stod(params.substr(0,pos));
        n_ = std::stod(params.substr(pos+1));
        if (!m_ || !n_) {
            std::cerr << "Fisher F Distribution: Invalid parameter." << std::endl;
            abort();
        }
    }

    unsigned int nextTimeDelta(std::default_random_engine rng) {

        std::fisher_f_distribution<double> distribution (m_, n_);
        if (floor_ && !ceiling_){ /* if send time */
            do {
                distri_return_ = (unsigned int)std::max(distribution(rng), 1.0);
            }while (distri_return_ <= floor_);
        } else if (!floor_ && ceiling_) { // if select lp 
            do {
                distri_return_ = (unsigned int) std::max(distribution(rng), 1.0);
            } while ( distri_return_  < floor_ || distri_return_ > ceiling_);
        }
        return distri_return_;
    }

private:
    double m_ = 0.0, n_ = 0.0;
};

#endif
