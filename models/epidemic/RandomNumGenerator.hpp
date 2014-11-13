#ifndef RANDOM_NUM_GENERATOR_HPP
#define RANDOM_NUM_GENERATOR_HPP

#include <stdlib.h>
#include <time.h>

class RandomNumGenerator {
public:

    RandomNumGenerator() = default;

    RandomNumGenerator(unsigned int seed_val) : seed_(seed_val) {}

    unsigned int generateRandNum(unsigned int upper_limit) {

        unsigned int rand_num = (unsigned int) rand_r(&seed_);
        return (upper_limit ? rand_num % upper_limit : rand_num);
    }

private:
    unsigned int seed_;
};

#endif
