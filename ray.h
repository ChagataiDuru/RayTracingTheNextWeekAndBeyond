#ifndef RAY_H
#define RAY_H

#include "rt.h"

class ray {
public:
    ray() {}

    ray(const point3& origin, const vec3& direction) : orig(origin), dir(direction) {}

    const point3& origin() const { return orig; }
    const vec3& direction() const { return dir; }

    point3 rayPos(double t) const {
        return point3(orig + t * dir);
    }

private:
    point3 orig;
    vec3 dir;
};

#endif