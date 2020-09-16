#include "contextmanager.h"

using namespace Main;

void ContextManager::renderFFTSpectrum(RenderingContext &rctx)
{
    Renderer::GraphRenderData graphRender;
    for (const auto& [frequency, intensity] : spectrogramTrack.back()) {
        graphRender.push_back({
            .x = frequency,
            .y = 4.3f * intensity - 9.6f,
        });
    }
    rctx.renderer->renderGraph(graphRender, 3.0f, 1, 1, 1);
}

void ContextManager::eventFFTSpectrum(RenderingContext &rctx)
{
}

