#include "util.h"

#include <algorithm>
#include <cmath>

static void diff(std::vector<double> in, std::vector<double>& out)
{
    out = std::vector<double>(in.size()-1);

    for(int i=1; i<in.size(); ++i)
        out[i-1] = in[i] - in[i-1];
}

static void vectorProduct(std::vector<double> a, std::vector<double> b, std::vector<double>& out)
{
    out = std::vector<double>(a.size());

    for(int i=0; i<a.size(); ++i)
        out[i] = a[i] * b[i];
}

static void findIndicesLessThan(std::vector<double> in, double threshold, std::vector<int>& indices)
{
    for(int i=0; i<in.size(); ++i)
        if(in[i]<threshold)
            indices.push_back(i+1);
}

static void selectElements(std::vector<double> in, std::vector<int> indices, std::vector<double>& out)
{
    for(int i=0; i<indices.size(); ++i)
        out.push_back(in[indices[i]]);
}

static void selectElements(std::vector<int> in, std::vector<int> indices, std::vector<int>& out)
{
    for(int i=0; i<indices.size(); ++i)
        out.push_back(in[indices[i]]);
}

static void signVector(std::vector<double> in, std::vector<int>& out, int sign)
{
    out = std::vector<int>(in.size());

    for(int i=0; i<in.size(); ++i)
    {
        if(sign*in[i]>0)
            out[i]=1;
        else if(sign*in[i]<0)
            out[i]=-1;
        else
            out[i]=0;
    }
}

std::vector<int> Analysis::findPeaks(const double *data, int length, int sign)
{
    std::vector<double> x0(data, std::next(data, length));
    std::vector<int> peakInds;

    int minIdx = distance(x0.begin(), min_element(x0.begin(), x0.end()));
    int maxIdx = distance(x0.begin(), max_element(x0.begin(), x0.end()));

    double sel = (x0[maxIdx]-x0[minIdx])/4.0;

    int len0 = x0.size();

    std::vector<double> dx;
    diff(x0, dx);
    replace(dx.begin(), dx.end(), 0.0f, -2.2204e-16f);
    std::vector<double> dx0(dx.begin(), dx.end()-1);
    std::vector<double> dx1(dx.begin()+1, dx.end());
    std::vector<double> dx2;

    vectorProduct(dx0, dx1, dx2);

    std::vector<int> ind;
    findIndicesLessThan(dx2, 0, ind); // Find where the derivative changes sign
    
    std::vector<double> x;

    std::vector<int> indAux(ind.begin(), ind.end());
    selectElements(x0, indAux, x);
    x.insert(x.begin(), x0[0]);
    x.insert(x.end(), x0[x0.size()-1]);;


    ind.insert(ind.begin(), 0);
    ind.insert(ind.end(), len0);

    int minMagIdx = distance(x.begin(), min_element(x.begin(), x.end()));
    double minMag = x[minMagIdx];
    double leftMin = minMag;
    int len = x.size();

    if(len>2)
    {
        double tempMag = minMag;
        bool foundPeak = false;
        int ii;

        // Deal with first point a little differently since tacked it on
        // Calculate the sign of the derivative since we tacked the first
        //  point on it does not neccessarily alternate like the rest.
        std::vector<double> xSub0(x.begin(), x.begin()+3);//tener cuidado substd::vector
        std::vector<double> xDiff;//tener cuidado substd::vector
        diff(xSub0, xDiff);

        std::vector<int> signDx;
        signVector(xDiff, signDx, sign);

        if (signDx[0] <= 0) // The first point is larger or equal to the second
        {
            if (signDx[0] == signDx[1]) // Want alternating signs
            {
                x.erase(x.begin()+1);
                ind.erase(ind.begin()+1);
                len = len-1;
            }
        }
        else // First point is smaller than the second
        {
            if (signDx[0] == signDx[1]) // Want alternating signs
            {
                x.erase(x.begin());
                ind.erase(ind.begin());
                len = len-1;
            }
        }

        if ( x[0] >= x[1] )
            ii = 0;
        else
            ii = 1;

        double maxPeaks = ceil((double)len/2.0);
        std::vector<int> peakLoc(maxPeaks,0);
        std::vector<double> peakMag(maxPeaks,0.0);
        int cInd = 1;
        int tempLoc;
    
        while(ii < len)
        {
            ii = ii+1;//This is a peak
            //Reset peak finding if we had a peak and the next peak is bigger
            //than the last or the left min was small enough to reset.
            if(foundPeak)
            {
                tempMag = minMag;
                foundPeak = false;
            }
        
            //Found new peak that was lager than temp mag and selectivity larger
            //than the minimum to its left.
        
            if( x[ii-1] > tempMag && x[ii-1] > leftMin + sel )
            {
                tempLoc = ii-1;
                tempMag = x[ii-1];
            }

            //Make sure we don't iterate past the length of our std::vector
            if(ii == len)
                break; //We assign the last point differently out of the loop

            ii = ii+1; // Move onto the valley
            
            //Come down at least sel from peak
            if(!foundPeak && tempMag > sel + x[ii-1])
            {                
                foundPeak = true; //We have found a peak
                leftMin = x[ii-1];
                peakLoc[cInd-1] = tempLoc; // Add peak to index
                peakMag[cInd-1] = tempMag;
                cInd = cInd+1;
            }
            else if(x[ii-1] < leftMin) // New left minima
                leftMin = x[ii-1];
            
        }

        // Check end point
        if ( x[x.size()-1] > tempMag && x[x.size()-1] > leftMin + sel )
        {
            peakLoc[cInd-1] = len-1;
            peakMag[cInd-1] = x[x.size()-1];
            cInd = cInd + 1;
        }
        else if( !foundPeak && tempMag > minMag )// Check if we still need to add the last point
        {
            peakLoc[cInd-1] = tempLoc;
            peakMag[cInd-1] = tempMag;
            cInd = cInd + 1;
        }

        //Create output
        if( cInd > 0 )
        {            
            std::vector<int> peakLocTmp(peakLoc.begin(), peakLoc.begin()+cInd-1);
            selectElements(ind, peakLocTmp, peakInds);
            //peakMags = std::vector<double>(peakLoc.begin(), peakLoc.begin()+cInd-1);
        }
    }

    return peakInds;
}
