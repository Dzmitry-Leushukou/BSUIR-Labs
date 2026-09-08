#pragma once

#include <cmath>


unsigned int threadsCount(unsigned int operationsCount) {
    return 1 + (log10(operationsCount) / 2.0);
}