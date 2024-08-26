#ifndef INTERVAL_H
#define INTERVAL_H

#include <limits>
#include <algorithm>

class interval {
public:
    static constexpr double infinity = std::numeric_limits<double>::infinity();

    double min, max;

    constexpr interval() noexcept : min(+infinity), max(-infinity) {}

    constexpr interval(double _min, double _max) noexcept : min(_min), max(_max) {}

    constexpr double size() const noexcept {
        return max - min;
    }

    constexpr bool contains(double x) const noexcept {
        return min <= x && x <= max;
    }

    constexpr bool surrounds(double x) const noexcept {
        return min < x && x < max;
    }

    static constexpr interval empty() noexcept {
        return interval(+infinity, -infinity);
    }

    static constexpr interval universe() noexcept {
        return interval(-infinity, +infinity);
    }
};

#endif