#include "util.h"
#include "laguerre.h"
#include <iostream>

rpm::vector<std::complex<double>> Analysis::findRoots(const rpm::vector<double>& p)
{
    auto roots = Analysis::laguerreSolve(p);
    return roots;
}

