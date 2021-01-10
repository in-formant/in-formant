#include "linpred.h"

using namespace Analysis::LP;

std::vector<double> Autocorr::solve(const double *data, int length, int lpcOrder, double *pGain)
{
    const int n = length;
    const int m = lpcOrder;

    std::vector<double> aut(m + 1);
    std::vector<double> lpc(m);    
    double error;
    double epsilon;
    int i, j;

    j=m+1;
    while(j--){
        double d=0; /* double needed for accumulator depth */
        for(i=j;i<n;i++)
            d+=(double)data[i]*data[i-j];
        aut[j]=d;
    }

    /* Generate lpc coefficients from autocorr values */

    /* set our noise floor to about -100dB */
    error=aut[0] * (1. + 1e-10);
    epsilon=1e-9*aut[0]+1e-10;

    for(i=0;i<m;i++){
        double r= -aut[i+1];

        if(error<epsilon){
            std::fill(std::next(lpc.begin(), i), lpc.end(), 0.0);
            goto done;
        }

        /* Sum up this iteration's reflection coefficient; note that in
           Vorbis we don't save it.  If anyone wants to recycle this code
           and needs reflection coefficients, save the results of 'r' from
           each iteration. */

        for(j=0;j<i;j++)
            r-=lpc[j]*aut[i-j];
        r/=error;

        /* Update LPC coefficients and total error */

        lpc[i]=r;
        for(j=0;j<i/2;j++){
            double tmp=lpc[j];

            lpc[j]+=r*lpc[i-1-j];
            lpc[i-1-j]+=r*tmp;
        }
        
        if(i&1)lpc[j]+=lpc[j]*r;

        error*=1.-r*r;
    }

done:

    /* slightly damp the filter */
    {
        double g = .99;
        double damp = g;
        for(j=0;j<m;j++){
            lpc[j]*=damp;
            damp*=g;
        }
        *pGain = damp;
    }

    return lpc;
}
