#include "contextmanager.h"
#include <iostream>

using namespace Main;

void ContextManager::renderOscilloscope(RenderingContext &rctx)
{
    int iframe = 
        useFrameCursor ? std::min(std::max<int>(std::round(specMX * spectrogramCount), 0), spectrogramCount - 1)
                       : spectrogramCount - 1;

    auto& sound = soundTrack[iframe];
    auto& glot = glotTrack[iframe];
    int length;

    length = sound.size();

    Renderer::GraphRenderData graphRender(length);
    for (int i = 0; i < length; ++i) {
        graphRender[i] = {
            .x = static_cast<float>(i),
            .y = sound[i] * 20.0f + 5.0f,
        };
    }
    if (!graphRender.empty()) {
        rctx.renderer->renderGraph(graphRender, 0, length - 1, 3.0f, 1.0f, 1.0f, 1.0f);
    }
    else {
        graphRender.push_back({.x = 0.0f, .y = 5.0f});
        graphRender.push_back({.x = 1.0f, .y = 5.0f});
        rctx.renderer->renderGraph(graphRender, 0, 1, 3.0f, 1.0f, 1.0f, 1.0f);
    }

    length = glot.size(); 
    graphRender.resize(length);

    for (int i = 0; i < length; ++i) {
        graphRender[i] = {
            .x = static_cast<float>(i),
            .y = glot[i] * 2.5f - 5.0f,
        };
    }
    if (!graphRender.empty()) {
        rctx.renderer->renderGraph(graphRender, 0, length - 1, 3.0f, 1.0f, 0.5f, 0.0f);
    } 
    else {
        graphRender.push_back({.x = 0.0f, .y = -5.0f});
        graphRender.push_back({.x = 1.0f, .y = -5.0f});
        rctx.renderer->renderGraph(graphRender, 0, 1, 3.0f, 1.0f, 0.5f, 0.0f);
    }
}

void ContextManager::eventOscilloscope(RenderingContext &rctx)
{
}

