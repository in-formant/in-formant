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
    rctx.renderer->renderGraph(graphRender, 3.0f, 0.9f, 0.9f, 0.9f);

    auto lpSpec = nodeIOs["linpred"][1]->as<Nodes::IO::AudioSpec>();

    graphRender.clear();
    for (int i = 0; i < lpSpec->getLength(); ++i) {
        graphRender.push_back({
            .x = i,
            .y = 4.3f * lpSpec->getConstData()[i] - 9.6f,
        });
    }
    rctx.renderer->renderGraph(graphRender, 2.0f, 1.0f, 0.5f, 0.0f);
}

void ContextManager::eventFFTSpectrum(RenderingContext &rctx)
{
}

