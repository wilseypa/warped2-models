#ifndef DISTRIBUTION_HPP
#define DISTRIBUTION_HPP

#include <random>   

class Distribution {
public:
    virtual unsigned int nextTimeDelta (std::shared_ptr<std::default_random_engine> rng) = 0;
};

class Geometric : public Distribution{
public:
    Geometric(std::string param) {

        p_ = std::stod(param);
        if (p_ <= 0 || p_ >= 1) {
            std::cerr << "Geometric Distribution: Invalid parameter." << std::endl;
            abort();
        }
    }

    unsigned int nextTimeDelta(std::shared_ptr<std::default_random_engine> rng) {
        std::geometric_distribution<int> distribution(p_);
        return std::abs(distribution(rng));
    }

private:
    double p_ = 0.0;
};

class Exponential : public Distribution{
public:
    unsigned int nextTimeDelta() {return 0;}
};

#endif
