#include "contextmanager.h"
#include <iostream>

using namespace Main;

void ContextManager::renderOscilloscope(RenderingContext &rctx)
{
    auto audioNode = nodeIOs["tail"][0]->as<Nodes::IO::AudioTime>();

    const float *data = audioNode->getConstData();
    int   length      = audioNode->getLength();
    float sampleRate  = audioNode->getSampleRate();

    Renderer::GraphRenderData graphRender(length);
    for (int i = 0; i < length; ++i) {
        graphRender[i] = {
            .x = i / sampleRate,
            .y = data[i] * 20.0f + 5.0f,
        };
    }
    rctx.renderer->renderGraph(graphRender, 3.0f, 1, 1, 1);

    auto glotAudioNode = nodeIOs["invglot"][0]->as<Nodes::IO::AudioTime>();

    data       = glotAudioNode->getConstData();
    length     = glotAudioNode->getLength();
    sampleRate = glotAudioNode->getSampleRate();
    graphRender.resize(length);

    for (int i = 0; i < length; ++i) {
        graphRender[i] = {
            .x = i / sampleRate,
            .y = data[i] * 2.5f - 5.0f,
        };
    }
    rctx.renderer->renderGraph(graphRender, 3.0f, 1, 1, 1);
}

void ContextManager::eventOscilloscope(RenderingContext &rctx)
{
}

