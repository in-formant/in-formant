//
// A class for linear transforms of the form ax + b
//
// Copyright (C) 2020 Andreas Gustafsson.  This file is part of
// the Gaborator library source distribution.  See the file LICENSE at
// the top level of the distribution for license information.
//

#ifndef _GABORATOR_LINEAR_TRANSFORM_H
#define _GABORATOR_LINEAR_TRANSFORM_H

namespace gaborator {

struct linear_transform {
    linear_transform(): a(0), b(0) { }
    linear_transform(double a_, double b_): a(a_), b(b_) { }
    linear_transform(const linear_transform &rhs): a(rhs.a), b(rhs.b) { }
    double operator()(double x) const { return a * x + b; }
    linear_transform inverse() const {
        return linear_transform(1.0 / a, -b / a);
    }
    static linear_transform identity() { return linear_transform(1, 0); }
    double a, b;
};

// Composition

static inline linear_transform
operator *(const linear_transform &a, const linear_transform &b) {
    return linear_transform(a.a * b.a, a.a * b.b + a.b);
}

} // namespace

#endif
