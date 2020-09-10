#ifndef ANALYSIS_LINPRED_H
#define ANALYSIS_LINPRED_H

#include <vector>

namespace Analysis {
    
    class LinpredSolver {
    public:
        virtual std::vector<float> solve(const float *x, int length, int lpcOrder, float *gain) = 0;
    };

    namespace LP {
        class Autocorr : public LinpredSolver {
        public:
            std::vector<float> solve(const float *x, int length, int lpcOrder, float *gain) override;
        private:
            std::vector<double> r, a, rc;
        };

        class Burg : public LinpredSolver {
        public:
            std::vector<float> solve(const float *x, int length, int lpcOrder, float *gain) override;
        private:
            std::vector<double> b1, b2, aa;
        };
    }

}

#endif // ANALYSIS_LINPRED_H
