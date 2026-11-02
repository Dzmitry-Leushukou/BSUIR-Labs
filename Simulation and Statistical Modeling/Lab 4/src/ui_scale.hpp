#pragma once

#include <cmath>

namespace ui_scale {
inline float factor = 1.0f;

inline int px(int value) {
    return static_cast<int>(std::lround(value * factor));
}
}
