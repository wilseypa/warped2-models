#ifndef DISTRIBUTION_HPP
#define DISTRIBUTION_HPP

#include <random>
//#include <string>

class Distribution {
public:
    virtual unsigned int nextTimeDelta (std::default_random_engine rng) = 0;
};

class Geometric : public Distribution {
public:
    Geometric(std::string params) {

        p_ = std::stod(params);
        if (p_ <= 0 || p_ >= 1) {
            std::cerr << "Geometric Distribution: Invalid parameter." << std::endl;
            abort();
        }
    }

    unsigned int nextTimeDelta (std::default_random_engine rng) {
        std::geometric_distribution<int> distribution(p_);
        return (unsigned int)std::max(distribution(rng), 1);
    }

private:
    double p_ = 0.0;
};


#endif
