#include "util.h"
#include "laguerre.h"
#include <iostream>

std::vector<std::complex<float>> Analysis::findRoots(const std::vector<float>& p)
{
    return Analysis::laguerreSolve(p);
}

