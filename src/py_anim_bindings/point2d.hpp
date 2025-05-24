#pragma once

#include <cmath>

namespace anim {

// Legacy Point2D class to maintain backward compatibility
struct Point2D {
    double time;   // The time (x-coordinate) of the point
    double value;  // The value (y-coordinate) of the point

    Point2D(double time = 0.0, double value = 0.0)
        : time(time), value(value) {}
};

} // namespace anim