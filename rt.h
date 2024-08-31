#ifndef RT_H
#define RT_H

#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
// C++ Std Usings

using std::make_shared;
using std::shared_ptr;

// Constants

const double infinity = std::numeric_limits<double>::infinity();
constexpr double pi = 3.1415926535897932385;
constexpr double degrees_to_radians_factor = pi / 180.0;

// Utility Functions

inline double degrees_to_radians(double degrees) {
    return degrees * degrees_to_radians_factor;
}

class RandomGenerator {
public:

    RandomGenerator(const RandomGenerator&) = delete;
    RandomGenerator& operator=(const RandomGenerator&) = delete;

    static RandomGenerator& instance() {
        static RandomGenerator instance;
        return instance;
    }

    double random_double() {
        return dist(rng);
    }

    double random_double(double min, double max) {
        std::uniform_real_distribution<double> dist(min, max);
        return dist(rng);
    }

    int random_int(int min, int max) {
        // Returns a random integer in [min,max].
        return int(random_double(min, max + 1));
    }

private:

    RandomGenerator() : rng(std::random_device{}()), dist(0.0, 1.0) {}

    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;
};
// Common Headers

#include "color.h"
#include "interval.h"
#include "ray.h"
#include "vec3.h"

#endif
