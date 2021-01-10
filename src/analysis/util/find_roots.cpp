#include "util.h"
#include "laguerre.h"
#include <iostream>

std::vector<std::complex<double>> Analysis::findRoots(const std::vector<double>& p)
{
    auto roots = Analysis::laguerreSolve(p);
    return roots;
}

