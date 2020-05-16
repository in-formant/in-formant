#include <iostream>
#include "IAIF.h"
#include "../LPC/LPC.h"
#include "../LPC/Frame/LPC_Frame.h"
#include "../Signal/Filter.h"
#include "../Signal/Window.h"

using namespace Eigen;

static ArrayXd conv(const ArrayXd& a, const ArrayXd& b)
{
    const int na = a.size();
    const int nb = b.size();
    const int n = na + nb - 1;

    ArrayXd out(n);

    for (int i = 0; i < n; ++i) {
        const int jmn = (i >= nb - 1) ? i - (nb - 1) : 0;
        const int jmx = (i < na - 1) ? i : na - 1;

        out(i) = 0.0;
        for (int j = jmn; j <= jmx; ++j) {
            out(i) += (a(j) * b(i - j));
        }
    }

    return out;
}

ArrayXd IAIF::processFrame(const ArrayXd& s_gvl, int nv, int ng, double d)
{
    int ns = s_gvl.size();
    int lpf = 2 * nv + 1;
    LPC::Frame lpc;

    ArrayXd one(1);
    one << 1;
   
    // Create window for LPC prediction.
    ArrayXd win = Window::createHamming(ns);

    // Addition of pre-frame.
    ArrayXd x_gvl(ns + lpf);
    x_gvl.head(lpf).setLinSpaced(-s_gvl(0), s_gvl(0));
    x_gvl.tail(ns) = s_gvl;

    // Cancel lip radiation contribution.
    ArrayXd lipFilter_a(2);
    lipFilter_a << 1, -d;

    ArrayXd s_gv(ns);
    ArrayXd x_gv(ns + lpf);
    Filter::apply(one, lipFilter_a, s_gvl, s_gv);
    Filter::apply(one, lipFilter_a, x_gvl, x_gv);

    // Gross glottis estimation.
    lpc.nCoefficients = 1;
    LPC::frame_auto(s_gv * win, lpc);

    ArrayXd ag1 = lpc.a;

    for (int i = 1; i < ng; ++i) {
        ArrayXd x_v1x;
        Filter::apply(lpc.a, x_gv, x_v1x);
        ArrayXd s_v1x = x_v1x.tail(ns);

        LPC::frame_auto(s_v1x * win, lpc);

        ag1 = conv(ag1, lpc.a);
    }

    // Gross vocal tract estimation.
    ArrayXd x_v1;
    Filter::apply(ag1, x_gv, x_v1);
    ArrayXd s_v1 = x_v1.tail(ns);
    
    lpc.nCoefficients = nv;
    LPC::frame_auto(s_v1 * win, lpc);
    ArrayXd av1 = lpc.a;
    
    // Fine glottis estimation.
    ArrayXd x_g1;
    Filter::apply(av1, x_gv, x_g1);
    ArrayXd s_g1 = x_g1.tail(ns);

    lpc.nCoefficients = ng;
    LPC::frame_auto(s_g1 * win, lpc);
    ArrayXd ag = lpc.a;

    // Fine vocal tract estimation.
    ArrayXd x_v;
    Filter::apply(ag, x_gv, x_v);
    ArrayXd s_v = x_v.tail(ns);
    
    lpc.nCoefficients = nv;
    LPC::frame_auto(s_v * win, lpc);
    ArrayXd av = lpc.a;

    ArrayXd gf;
    Filter::apply(av, x_gv, gf);
    return gf.tail(ns);
}
