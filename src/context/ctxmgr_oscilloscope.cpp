#include "contextmanager.h"

using namespace Main;

void ContextManager::renderOscilloscope(RenderingContext &rctx)
{
    auto audioNode = nodeIOs["tail"][0]->as<Nodes::IO::AudioTime>();

    const float *data = audioNode->getConstData();
    int   length      = audioNode->getLength();
    float sampleRate  = audioNode->getSampleRate();

    Renderer::GraphRenderData graphRender;
    for (int i = 0; i < length; ++i) {
        graphRender.push_back({
            .x = i / sampleRate,
            .y = data[i] * 20.0f,
        });
    }
    rctx.renderer->renderGraph(graphRender);
}

void ContextManager::eventOscilloscope(RenderingContext &rctx)
{
}

