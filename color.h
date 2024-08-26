#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"
#include <algorithm>

using color = vec3;

void write_color(std::ostream& out, const color& pixel_color) {

    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    int rbyte = std::clamp((int)(255.999 * r), 0, 255);
    int gbyte = std::clamp((int)(255.999 * g), 0, 255);
    int bbyte = std::clamp((int)(255.999 * b), 0, 255);

    out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}

#endif