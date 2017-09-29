#ifndef DISTRIBUTION_HPP
#define DISTRIBUTION_HPP

#include <random>

#define FLOOR 1

class Distribution {
public:
    virtual unsigned int nextTimeDelta (std::default_random_engine rng) = 0;

protected:
    unsigned int ceiling_ = 0;
};

class Geometric : public Distribution{
public:
    Geometric(std::string params) {

        size_t pos = params.find(",");
        p_ = std::stod(params.substr(0, pos));
        if (p_ <= 0 || p_ >= 1) {
            std::cerr << "Geometric Distribution: Invalid parameter." << std::endl;
            abort();
        }
        ceiling_ = (unsigned int) std::stoul(params.substr(pos+1));
    }

    unsigned int nextTimeDelta(std::default_random_engine rng) {

        std::geometric_distribution<int> distribution(p_);
        unsigned int val = (unsigned int)distribution(rng);
        return ((ceiling_) ? val % ceiling_ : val) + FLOOR;
    }

private:
    double p_ = 0.0;
};

class Exponential : public Distribution {
public:
    Exponential(std::string params) {

        size_t pos = params.find(",");
        lambda_  = std::stod(params.substr(0, pos));
        if (lambda_ <= 0) {
            std::cerr << "Exponential Distribution: Invalid parameter." << std::endl;
            abort();
        }
        ceiling_ = (unsigned int) std::stoul(params.substr(pos+1));
    }

    unsigned int nextTimeDelta(std::default_random_engine rng) {

        std::exponential_distribution<double> distribution(lambda_);
        unsigned int val = (unsigned int)distribution(rng);
        return ((ceiling_) ? val % ceiling_ : val) + FLOOR;
    }

private:
    double lambda_ = 0.0;
};

class Binomial : public Distribution {
public:
    Binomial (std::string params) {

        size_t pos = params.find(",");
        p_ = std::stod(params.substr(0, pos));
        if (p_ < 0 || p_ > 1) {
            std::cerr << "Binomial Distribution: Invalid parameter p." << std::endl;
            abort();
        }
        ceiling_ = (unsigned int) std::stoul(params.substr(pos+1));
        if (ceiling_ <= FLOOR) {
            std::cerr << "Binomial Distribution: Invalid ceiling value." << std::endl;
            abort();
        }
    }

    unsigned int nextTimeDelta(std::default_random_engine rng) {

        std::binomial_distribution<int> distribution(ceiling_-FLOOR, p_);
        unsigned int val = (unsigned int)distribution(rng);
        return val + FLOOR;
    }

private:
    double p_ = 0.0;
};


#if 0
class Cauchy : public Distribution {
public:
    Cauchy (std::string params, unsigned int ceiling_) {

        ceiling = ceiling_;
        std::string delimiter = ",";
        size_t pos = params.find(delimiter);
        a_ = std::stod(params.substr(0,pos));
        b_ = std::stod(params.substr(pos+1));
        if (a_ < 0  || b_ < 0) {
            std::cerr << "Cauchy Distribution: Invalid parameter." << std::endl;
            abort();
        }
    }

    unsigned int nextTimeDelta(std::default_random_engine rng) {

        std::cauchy_distribution<double> distribution(a_, b_);
        unsigned int val = (unsigned int) distribution(rng);
        return (ceiling_) ? val % ceiling_ : (val + FLOOR);
    }

private:
    double a_ = 0.0, b_ = 0.0;
    unsigned int ceiling = 0;
};

class Chi_squared : public Distribution {
public:
    Chi_squared (std::string params, unsigned int ceiling_) { 

        ceiling = ceiling_;
        n_ = std::stod(params);
        if (n_ <= 0) {
            std::cerr << "Chi Squared Distribution: Invalid parameter." << std::endl;
            abort();
        }
    }

    unsigned int nextTimeDelta(std::default_random_engine rng) {

        std::chi_squared_distribution<double> distribution(n_);
        unsigned int val = (unsigned int) distribution(rng);
        return (ceiling_) ? val % ceiling_ : (val + FLOOR);
    }

private:
    double n_ = 0.0;
    unsigned int ceiling = 0;
};

class Discrete : public Distribution {
public:
    Discrete (std::string params, unsigned int ceiling_) {

        ceiling = ceiling_;
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
        unsigned int val = (unsigned int) distribution(rng);
        return (ceiling_) ? val % ceiling_ : (val + FLOOR);
    }

private:
    double first_ = 0.0, last_ = 0.0;
    unsigned int ceiling = 0;
};

class Fisher_f : public Distribution {
public:
    Fisher_f (std::string params, unsigned int ceiling_) {

        ceiling = ceiling_;
        std::string delimiter = ",";
        size_t pos = params.find(delimiter);
        m_ = std::stod(params.substr(0,pos));
        n_ = std::stod(params.substr(pos+1));
        if (m_ <= 0 || n_ <= 0) {
            std::cerr << "Fisher F Distribution: Invalid parameter." << std::endl;
            abort();
        }
    }

    unsigned int nextTimeDelta(std::default_random_engine rng) {

        std::fisher_f_distribution<double> distribution (m_, n_);
        unsigned int val = (unsigned int) distribution(rng);
        return (ceiling_) ? val % ceiling_ : (val + FLOOR);
    }

private:
    double m_ = 0.0, n_ = 0.0;
    unsigned int ceiling = 0;
};

class Negative_binomial : public Distribution {
public:
    Negative_binomial (std::string params, unsigned int ceiling_) {

        ceiling = ceiling_;
        std::string delimiter = ",";
        size_t pos = params.find(delimiter);
        k_ = (unsigned int) std::stoul(params.substr(0,pos));
        p_ = std::stod(params.substr(pos+1));
        if (p_ < 0 || p_ > 1 || k_ <= 0) {
            std::cerr << "Negative Binomial Distribution: Invalid parameter." << std::endl;
            abort();
        }
    }

    unsigned int nextTimeDelta(std::default_random_engine rng) {

        std::negative_binomial_distribution<int> distribution(k_, p_);
        unsigned int val = (unsigned int) distribution(rng);
        return (ceiling_) ? val % ceiling_ : (val + FLOOR);
    }

private:
    unsigned int k_ = 0, ceiling = 0;
    double p_ = 0.0;
};

class Normal : public Distribution {
public:
    Normal (std::string params, unsigned int ceiling_) {

        ceiling = ceiling_;
        std::string delimiter = ",";
        size_t pos = params.find(delimiter);
        mean_ = std::stod(params.substr(0, pos));
        stddev_ = std::stod(params.substr(pos+1));
        if(mean_ < 0 || stddev_ <= 0) {
            std::cerr << "Normal Distribution: Invalid parameter" << std::endl;
            abort();
        }
    }

    unsigned int nextTimeDelta(std::default_random_engine rng) {

        std::normal_distribution<double> distribution(mean_, stddev_);
        unsigned int val = (unsigned int) distribution(rng);
        return (ceiling_) ? val % ceiling_ : (val + FLOOR);
    }

private:
    double mean_ = 0.0, stddev_ = 0.0;
    unsigned int ceiling = 0;
};


class Uniform_int : public Distribution {
public:
    Uniform_int (std::string params, unsigned int ceiling_) {

        ceiling = ceiling_;
        std::string delimiter = ",";
        size_t pos = params.find(delimiter);
        a_ = (unsigned int)std::stoul(params.substr(0, pos));
        b_ = (unsigned int)std::stoul(params.substr(pos+1));
        if(a_ <= 0 || a_ >= b_) {
            std::cerr << "Normal Distribution: Invalid parameter" << std::endl;
            abort();
        }
    }

    unsigned int nextTimeDelta(std::default_random_engine rng) {

        std::uniform_int_distribution<int> distribution(a_, b_);
        unsigned int val = (unsigned int) distribution(rng);
        return (ceiling_) ? val % ceiling_ : (val + FLOOR);
    }

private:
    unsigned int a_ = 0, b_ = 0, ceiling = 0;
};

        
class Poisson : public Distribution {
public:
    Poisson (std::string params, unsigned int ceiling_) {

        ceiling = ceiling_;
        mean_ = std::stod(params);
        if(mean_ <= 0) {
            std::cerr << "Normal Distribution: Invalid parameter" << std::endl;
            abort();
        }
    }

    unsigned int nextTimeDelta(std::default_random_engine rng) {

        std::poisson_distribution<int> distribution(mean_);
        unsigned int val = (unsigned int) distribution(rng);
        return (ceiling_) ? val % ceiling_ : (val + FLOOR);
    }

private:
    double mean_ = 0.0;
    unsigned int ceiling = 0;
};

class Lognormal : public Distribution {
public:
    Lognormal (std::string params, unsigned int ceiling_) {

        ceiling = ceiling_;
        std::string delimiter = ",";
        size_t pos = params.find(delimiter);
        mean_ = std::stod(params.substr(0, pos));
        stddev_ = std::stod(params.substr(pos+1));
        if(mean_ < 0|| stddev_ <= 0) {
            std::cerr << "Normal Distribution: Invalid parameter" << std::endl;
            abort();
        }
    }

    unsigned int nextTimeDelta(std::default_random_engine rng) {

        std::lognormal_distribution<double> distribution(mean_, stddev_);
        unsigned int val = (unsigned int) distribution(rng);
        return (ceiling_) ? val % ceiling_ : (val + FLOOR);
    }

private:
    double mean_ = 0.0, stddev_ = 0.0;
    unsigned int ceiling = 0;
};

#endif


#endif
