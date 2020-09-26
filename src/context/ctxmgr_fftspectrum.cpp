#include "contextmanager.h"
#include <iostream>

using namespace Main;

void ContextManager::renderFFTSpectrum(RenderingContext &rctx)
{
    int iframe = 
        useFrameCursor ? std::min(std::max<int>(std::round(specMX * spectrogramCount), 0), spectrogramCount - 1)
                       : spectrogramCount - 1;

    auto spec = spectrogramTrack[iframe];
    auto lpSpec = lpSpecTrack[iframe];

    Renderer::GraphRenderData graphRender;
    for (const auto& [frequency, intensity] : spec) {
        graphRender.push_back({
            .x = frequency,
            .y = 4.6f * log10f(1.0f + intensity) - 9.6f,
        });
    }
    if (graphRender.empty()) {
        graphRender.push_back({.x = static_cast<float>(viewMinFrequency), .y = -9.6f});
        graphRender.push_back({.x = static_cast<float>(viewMaxFrequency), .y = -9.6f});
    }
    rctx.renderer->renderGraph(graphRender, viewMinFrequency, viewMaxFrequency, 3.0f, 0.9f, 0.9f, 0.9f);

    graphRender.clear();
    for (const auto& [frequency, intensity] : lpSpec) {
        graphRender.push_back({
            .x = frequency,
            .y = 6.0f * intensity - 9.6f,
        });
    }
    if (graphRender.empty()) {
        graphRender.push_back({.x = static_cast<float>(viewMinFrequency), .y = -9.6f});
        graphRender.push_back({.x = static_cast<float>(viewMaxFrequency), .y = -9.6f});
    }
    rctx.renderer->renderGraph(graphRender, viewMinFrequency, viewMaxFrequency, 2.0f, 1.0f, 0.5f, 0.0f);
}

void ContextManager::eventFFTSpectrum(RenderingContext &rctx)
{
}

