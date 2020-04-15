#include "../Signal/Filter.h"
#include "../Signal/Window.h"
#include "LPC.h"
#include "Frame/LPC_Frame.h"

using namespace Eigen;

ArrayXd LPC::residual(const ArrayXd & x, int L, int shift, int order)
{
    const int len(x.size());
    int start, stop;
   
    ArrayXd win = Window::createHanning(L);
  
    start = 0;
    stop = start + L - 1;

    ArrayXd res(len);
    res.setZero();

    ArrayXd segment, A, inv;

    LPC::Frame frame;
    frame.nCoefficients = order;

    while (stop < len) {
        segment = x(seq(start, stop)) * win;
        
        LPC::frame_auto(segment, frame);
        Filter::apply(frame.a, segment, inv);
        inv *= std::sqrt(segment.square().sum() / inv.square().sum());

        res(seq(start, stop)) += inv;

        start += shift;
        stop += shift;
    }

    res /= res.abs().maxCoeff();

    return std::move(res);

}
