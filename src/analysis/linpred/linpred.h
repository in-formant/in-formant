#ifndef ANALYSIS_LINPRED_H
#define ANALYSIS_LINPRED_H

#include <vector>

namespace Analysis {
    
    class LinpredSolver {
    public:
        virtual ~LinpredSolver() {}
        virtual std::vector<double> solve(const double *x, int length, int lpcOrder, double *gain) = 0;
    };

    namespace LP {
        class Autocorr : public LinpredSolver {
        public:
            std::vector<double> solve(const double *x, int length, int lpcOrder, double *gain) override;
        private:
            std::vector<double> r, a, rc;
        };

        class Burg : public LinpredSolver {
        public:
            std::vector<double> solve(const double *x, int length, int lpcOrder, double *gain) override;
        private:
            std::vector<double> b1, b2, aa;
        };
    }

}

#endif // ANALYSIS_LINPRED_H
