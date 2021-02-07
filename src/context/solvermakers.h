#ifndef MAIN_SOLVER_MAKERS_H
#define MAIN_SOLVER_MAKERS_H

#include "../analysis/analysis.h"

namespace Main {
    
    enum class PitchAlgorithm : int64_t {
        Yin,
        MPM,
        RAPT,
    };
    
    Analysis::PitchSolver *makePitchSolver(PitchAlgorithm alg);
    
    enum class LinpredAlgorithm : int64_t {
        Autocorr,
        Covar,
        Burg,
    };

    Analysis::LinpredSolver *makeLinpredSolver(LinpredAlgorithm alg);

    enum class FormantAlgorithm : int64_t {
        Simple,
        Filtered,
        Deep,
    };

    Analysis::FormantSolver *makeFormantSolver(FormantAlgorithm alg);

    enum class InvglotAlgorithm : int64_t {
        IAIF,
        GFM_IAIF,
    };

    Analysis::InvglotSolver *makeInvglotSolver(InvglotAlgorithm alg);

}

#endif // MAIN_SOLVER_MAKERS_H
