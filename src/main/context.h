#ifndef MAIN_CONTEXT_H
#define MAIN_CONTEXT_H

#include "../modules/modules.h"
#include "../analysis/analysis.h"
#include <memory>

namespace Main {

    using namespace Module;

    struct Context {
        std::unique_ptr<Audio::AbstractBase>        audio;
        std::unique_ptr<Target::AbstractBase>       target;
        std::unique_ptr<Renderer::AbstractBase>     renderer;
        
        std::unique_ptr<Freetype::FTInstance>       freetypeInstance;

        std::unique_ptr<Audio::Buffer>              captureBuffer;
        std::unique_ptr<Audio::Queue>               playbackQueue;

        std::shared_ptr<Analysis::PitchSolver>      pitchSolver;
        std::shared_ptr<Analysis::LinpredSolver>    linpredSolver;
        std::shared_ptr<Analysis::FormantSolver>    formantSolver;
    };

}

#endif // MAIN_CONTEXT_H
