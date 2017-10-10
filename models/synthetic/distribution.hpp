#ifndef DISTRIBUTION_HPP
#define DISTRIBUTION_HPP

#include <random>

#define FLOOR 1

class Distribution {
public:
    virtual unsigned int nextRandNum (std::default_random_engine rng) = 0;

protected:
    unsigned int ceiling_ = 0;
};

class Geometric : public Distribution{
public:
    Geometric(std::string params) {

        size_t pos = params.find(",");
        p_ = std::stod(params.substr(0, pos));
        if (p_ < 0 || p_ > 1) {
            std::cerr << "Geometric Distribution: Invalid parameter." << std::endl;
            abort();
        }
        ceiling_ = (unsigned int) std::stoul(params.substr(pos+1));
    }

    unsigned int nextRandNum(std::default_random_engine rng) {

        /* this distribution will return 1, as if only run once */
        std::geometric_distribution<int> distribution(p_);
        auto val = (unsigned int)distribution(rng);
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
        /* as lanbda higher than 1 the higher change get 0 or all 0 */
        if (lambda_ <= 0 || lambda_ >= 1) {
            std::cerr << "Exponential Distribution: Invalid parameter." << std::endl;
            abort();
        }
        ceiling_ = (unsigned int) std::stoul(params.substr(pos+1));
    }

    unsigned int nextRandNum(std::default_random_engine rng) {

        std::exponential_distribution<double> distribution(lambda_);
        unsigned int val = (unsigned int)distribution(rng);
        return ((ceiling_) ? val % ceiling_ : val) + FLOOR;
    }

private:
    double lambda_ = 0.0;
};


/* randomise is pending on p, the percentage of the ceiling is often seleted */
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

    unsigned int nextRandNum(std::default_random_engine rng) {

        std::binomial_distribution<int> distribution(ceiling_-FLOOR, p_);
        unsigned int val = (unsigned int)distribution(rng);
        return val + FLOOR;
    }

private:
    double p_ = 0.0;
};


class Cauchy : public Distribution {
public:
    Cauchy (std::string params) {

        size_t pos = params.find(",");
        a_ = std::stod(params.substr(0,pos));
        std::string temp = params.substr(pos+1);
        pos = temp.find(",");
        b_ = std::stod(temp.substr(0,pos));
        if (a_ < 0  || b_ < 0) {
            std::cerr << "Cauchy Distribution: Invalid parameter." << std::endl;
            abort();
        }
        ceiling_ = std::stoul(temp.substr(pos+1));
    }

    unsigned int nextRandNum(std::default_random_engine rng) {

        std::cauchy_distribution<double> distribution(a_, b_);
        unsigned int val = (unsigned int) distribution(rng);
        return ((ceiling_) ? val % ceiling_ : val) + FLOOR;
    }

private:
    double a_ = 0.0, b_ = 0.0;
};


class Chi_squared : public Distribution {
public:
    Chi_squared (std::string params) {

        size_t pos = params.find(",");
        n_ = std::stod(params.substr(0,pos));
        if (n_ <= 0) {
            std::cerr << "Chi Squared Distribution: Invalid parameter." << std::endl;
            abort();
        }
        ceiling_ = std::stoul(params.substr(pos+1));
    }

    unsigned int nextRandNum(std::default_random_engine rng) {

        std::chi_squared_distribution<double> distribution(n_);
        unsigned int val = (unsigned int) distribution(rng);
        return ((ceiling_) ? val % ceiling_ : val) + FLOOR;
    }

private:
    double n_ = 0.0;
};


class Discrete : public Distribution {
public:
    Discrete (std::string params) {

        size_t pos = params.find(",");
        last_ = std::stod(params.substr(0,pos));
        std::string temp = params.substr(pos+1);
        pos = temp.find(",");
        first_ = std::stod(temp.substr(0,pos));
        /* if the first >= last, higher chance to get 0 */
        if (first_ <= 0 || last_ <= 0 || first_ <= last_) {
            std::cerr << "Discrete Distribution: Invalid parameter." << std::endl;
            abort();
        }
        ceiling_ = std::stoul(temp.substr(pos+1));
    }

    unsigned int nextRandNum(std::default_random_engine rng) {

        std::discrete_distribution<int> distribution(first_, last_);
        unsigned int val = (unsigned int) distribution(rng);
        return ((ceiling_) ? val % ceiling_ : val) + FLOOR;
    }

private:
    double first_ = 0.0, last_ = 0.0;
};


class Fisher_f : public Distribution {
public:
    Fisher_f (std::string params) {

        size_t pos = params.find(",");
        m_ = std::stod(params.substr(0,pos));
        std::string temp = params.substr(pos+1);
        n_ = std::stod(temp.substr(0,pos));
        if (m_ <= 0 || n_ <= 0) {
            std::cerr << "Fisher F Distribution: Invalid parameter." << std::endl;
            abort();
        }
        ceiling_ = std::stoul(temp.substr(pos+1));
    }

    unsigned int nextRandNum(std::default_random_engine rng) {

        std::fisher_f_distribution<double> distribution (m_, n_);
        unsigned int val = (unsigned int) distribution(rng);
        return ((ceiling_) ? val % ceiling_ : val) + FLOOR;
    }

private:
    double m_ = 0.0, n_ = 0.0;
};


class Negative_binomial : public Distribution {
public:
    Negative_binomial (std::string params) {

        size_t pos = params.find(",");
        k_ = std::stoul(params.substr(0,pos));
        std::string temp = params.substr(pos+1);
        pos = temp.find(",");
        p_ = std::stod(temp.substr(0,pos));
        if (p_ < 0 || p_ > 1 || k_ <= 0) {
            std::cerr << "Negative Binomial Distribution: Invalid parameter." << std::endl;
            abort();
        }
        ceiling_ = std::stoul(temp.substr(pos+1));
    }

    unsigned int nextRandNum(std::default_random_engine rng) {

        std::negative_binomial_distribution<int> distribution(k_, p_);
        unsigned int val = (unsigned int) distribution(rng);
        return ((ceiling_) ? val % ceiling_ : val) + FLOOR;
    }

private:
    unsigned int k_ = 0;
    double p_ = 0.0;
};


class Normal : public Distribution {
public:
    Normal (std::string params) {

        size_t pos = params.find(",");
        mean_ = std::stod(params.substr(0, pos));
        std::string temp = params.substr(pos+1);
        pos = temp.find(",");
        stddev_ = std::stod(temp.substr(0,pos));
        if(mean_ < 0 || stddev_ <= 0) {
            std::cerr << "Normal Distribution: Invalid parameter" << std::endl;
            abort();
        }
        ceiling_ = std::stoul(temp.substr(pos));
    }

    unsigned int nextRandNum(std::default_random_engine rng) {

        std::normal_distribution<double> distribution(mean_, stddev_);
        unsigned int val = (unsigned int) distribution(rng);
        return ((ceiling_) ? val % ceiling_ : val) + FLOOR;
    }

private:
    double mean_ = 0.0, stddev_ = 0.0;
};


/* uniform_int retruns the number in range of [a,ceiling] */
class Uniform_int : public Distribution {
public:
    Uniform_int (std::string params) {

        size_t pos = params.find(",");
        a_ = std::stoul(params.substr(0, pos));
        ceiling_ = std::stoul(params.substr(pos+1));
        if(a_ >= ceiling_) {
            std::cerr << "Normal Distribution: Invalid parameter" << std::endl;
            abort();
        }
    }

    unsigned int nextRandNum(std::default_random_engine rng) {

        std::uniform_int_distribution<int> distribution(a_, ceiling_);
        unsigned int val = (unsigned int) distribution(rng);
        return ((ceiling_) ? val % ceiling_ : val) + FLOOR;
    }

private:
    unsigned int a_ = 0;
};

        
class Poisson : public Distribution {
public:
    Poisson (std::string params) {

        size_t pos = params.find(",");
        mean_ = std::stod(params.substr(0,pos));
        if(mean_ <= 3) { //if mena lower than 3 higher chance return 0 or 1
            std::cerr << "Normal Distribution: Invalid parameter" << std::endl;
            abort();
        }
        ceiling_ = std::stoul(params.substr(pos+1));
    }

    unsigned int nextRandNum(std::default_random_engine rng) {

        std::poisson_distribution<int> distribution(mean_);
        unsigned int val = (unsigned int) distribution(rng);
        return ((ceiling_) ? val % ceiling_ : val) + FLOOR;
    }

private:
    double mean_ = 0.0;
};


class Lognormal : public Distribution {
public:
    Lognormal (std::string params) {

        std::string delimiter = ",";
        size_t pos = params.find(delimiter);
        mean_ = std::stod(params.substr(0, pos));
        std::string temp = params.substr(pos+1);
        pos = temp.find(",");
        stddev_ = std::stod(temp.substr(0,pos));
        if(mean_ <= 1 || stddev_ < 1) {
            std::cerr << "Normal Distribution: Invalid parameter" << std::endl;
            abort();
        }
        ceiling_ = std::stoul(temp.substr(pos+1));
    }

    unsigned int nextRandNum(std::default_random_engine rng) {

        std::lognormal_distribution<double> distribution(mean_, stddev_);
        unsigned int val = (unsigned int) distribution(rng);
        return ((ceiling_) ? val % ceiling_ : val) + FLOOR;
    }

private:
    double mean_ = 0.0, stddev_ = 0.0;
};



#endif
