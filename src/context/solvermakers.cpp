#include "solvermakers.h"
#include <stdexcept>

using namespace Main;

Analysis::PitchSolver *Main::makePitchSolver(PitchAlgorithm alg)
{
    switch (alg) {
    case PitchAlgorithm::Yin:
        return new Analysis::Pitch::Yin(0.15);
    case PitchAlgorithm::MPM:
        return new Analysis::Pitch::MPM;
    case PitchAlgorithm::RAPT:
        return new Analysis::Pitch::RAPT;
    default:
        throw std::runtime_error("ContextManager] Unknown pitch estimation algorithm.");
    }
}

Analysis::LinpredSolver *Main::makeLinpredSolver(LinpredAlgorithm alg)
{
    switch (alg) {
    case LinpredAlgorithm::Autocorr:
        return new Analysis::LP::Autocorr;
    case LinpredAlgorithm::Covar:
        return new Analysis::LP::Covar;
    case LinpredAlgorithm::Burg:
        return new Analysis::LP::Burg;
    default:
        throw std::runtime_error("ContextManager] Unknown linear prediction algorithm.");
    }
}

Analysis::FormantSolver *Main::makeFormantSolver(FormantAlgorithm alg)
{
    switch (alg) {
    case FormantAlgorithm::Simple:
        return new Analysis::Formant::SimpleLP;
    case FormantAlgorithm::Filtered:
        return new Analysis::Formant::FilteredLP;
#ifdef ENABLE_TORCH
    case FormantAlgorithm::Deep:
        return new Analysis::Formant::DeepFormants;
#else
#endif
    default:
        throw std::runtime_error("ContextManager] Unknown formant estimation algorithm.");
    }
}

Analysis::InvglotSolver *Main::makeInvglotSolver(InvglotAlgorithm alg)
{
    switch (alg) {
    case InvglotAlgorithm::IAIF:
        return new Analysis::Invglot::IAIF(0.99);
    case InvglotAlgorithm::GFM_IAIF:
        return new Analysis::Invglot::GFM_IAIF(0.99);
    default:
        throw std::runtime_error("ContextManager] Unknown glottal inverse filtering algorithm.");
    }
}
