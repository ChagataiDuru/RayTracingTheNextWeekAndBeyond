#ifndef AABB_H
#define AABB_H

#include "rt.h"


class aabb {
public:
    std::array<interval, 3> intervals;

    aabb() = default;

    aabb(const interval& x, const interval& y, const interval& z) noexcept
        : intervals{ x, y, z } {}

    aabb(const point3& a, const point3& b) noexcept {
        for (size_t i = 0; i < 3; ++i) 
            intervals[i] = interval(std::min(a[i], b[i]), std::max(a[i], b[i]));
    }

    aabb(const aabb& box0, const aabb& box1) {
        for (size_t i = 0; i < 3; ++i) 
			intervals[i] = interval(std::min(box0.intervals[i].min, box1.intervals[i].min),std::max(box0.intervals[i].max, box1.intervals[i].max));
    }

    constexpr const interval& axis_interval(size_t n) const noexcept {
        return intervals[n];
    }

    bool hit(const ray& r, interval ray_t) const noexcept {
        const auto& ray_orig = r.origin();
        const auto& ray_dir = r.direction();

        for (size_t axis = 0; axis < 3; ++axis) {
            const auto& ax = axis_interval(axis);
            const auto adinv = 1.0 / ray_dir[axis];

            auto [t0, t1] = std::minmax((ax.min - ray_orig[axis]) * adinv,
                (ax.max - ray_orig[axis]) * adinv);

            ray_t.min = std::max(t0, ray_t.min);
            ray_t.max = std::min(t1, ray_t.max);

            if (ray_t.max <= ray_t.min)
                return false;
        }
        return true;
    }

    int longest_axis() const
    {
		auto longest = 0;
        for (size_t i = 1; i < 3; ++i) {
			if (intervals[i].length() > intervals[longest].length())
				longest = i;
		}
		return longest;
	}

    static const aabb empty, universe;
};

const aabb aabb::empty = aabb(interval(0, 0), interval(0, 0), interval(0, 0));
const aabb aabb::universe = aabb(interval(-infinity, infinity), interval(-infinity, infinity), interval(-infinity, infinity));

#endif