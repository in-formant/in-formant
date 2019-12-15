#include <functional>
#include "../Analyser.h"

template<typename U, typename T>
static void applyMedianFilter(U & list, int wlen, double& (*accessor)(T&));

inline double & getPitch(double & frm) { return std::ref(frm); }

template<int i>
inline double & getFormant(Formant::Frame & frm) { return std::ref(frm.formant.at(i).frequency); }

void Analyser::applyMedianFilters()
{
    applyMedianFilter(pitchTrack, 2, &getPitch);
    applyMedianFilter(formantTrack, 2, &getFormant<0>);
    applyMedianFilter(formantTrack, 2, &getFormant<1>);
    applyMedianFilter(formantTrack, 2, &getFormant<2>);
    applyMedianFilter(formantTrack, 2, &getFormant<3>);
}

template<typename U, typename T>
void applyMedianFilter(U & list, int wlen, double& (*accessor)(T&))
{
    const int N = list.size();
    
    std::vector<double> window;
    window.reserve(wlen);

    double sum = 0.0;
    double den = 0;

    for (int k = 0; k < wlen; ++k) {
        try {
            double value = accessor(list.at(N - 1 - k));
            if (value == 0) break;

            sum += accessor(list.at(N - 1 - k));
            den++; 
        } catch (std::exception & e) {
            break;
        }
    }

    try {
        accessor(list.at(N - 1)) = (den > 0 ? sum / den : 0);
    } catch (std::exception & e) {
    }
}


