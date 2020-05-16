#include "../Signal/Filter.h"
#include "../Signal/Window.h"
#include "LPC.h"
#include "Frame/LPC_Frame.h"

using namespace Eigen;

ArrayXd LPC::residual(const ArrayXd & x, int L, int shift, int order)
{
    const int len(x.size());
    int start;
   
    ArrayXd win = Window::createHanning(L);
  
    start = 0;

    ArrayXd res(len);
    res.setZero();

    ArrayXd segment, A, inv;

    LPC::Frame frame;
    frame.nCoefficients = order;

    while (start + L < len) {
        segment = x.segment(start, L) * win;
        
        LPC::frame_auto(segment, frame);
        Filter::apply(frame.a, segment, inv);
        inv *= std::sqrt(segment.square().sum() / inv.square().sum());

        res.segment(start, L) += inv;

        start += shift;
    }

    res /= res.abs().maxCoeff();

    return res;

}
