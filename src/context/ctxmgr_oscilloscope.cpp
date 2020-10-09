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

    // Find the last zero crossing in glot.
    int glotZcr = glot.size() - 1;

    for (int i = glot.size() - 1; i >= 1; --i) {
        if (glot[i - 1] * glot[i] < 0 && glot[i - 1] > 0) {
            glotZcr = i;
            break;
        }
    }

    // Find the second to last zero crossing in glot.
    int glotZcr2 = glotZcr - 3;
    for (int i = glotZcr2; i >= 1; --i) {
        if (glot[i - 1] * glot[i] < 0 && glot[i - 1] > 0) {
            glotZcr2 = i;
            break;
        }
    }

    int glotPeriod = std::max(glotZcr - glotZcr2, 10);
    
    static float prevGlotLength = glot.size();

    int glotLength = (glotZcr / glotPeriod) * glotPeriod;

    prevGlotLength = 0.08f * glotLength + 0.92f * prevGlotLength;
    glotLength = std::floor(prevGlotLength);

    int soundZcr = (glotZcr * sound.size()) / glot.size();
    int soundZcr2 = (glotZcr2 * sound.size()) / glot.size();
    int soundLength = (glotLength * sound.size()) / glot.size();

    Renderer::GraphRenderData graphRender(soundLength);
    for (int i = 0; i < soundLength; ++i) {
        graphRender[i] = {
            .x = static_cast<float>(i),
            .y = sound[sound.size() - soundLength + i] * 20.0f + 5.0f,
        };
    }
    if (!graphRender.empty()) {
        rctx.renderer->renderGraph(graphRender, 0, soundLength - 1, 3.0f, 1.0f, 1.0f, 1.0f);
    }
    else {
        graphRender.push_back({.x = 0.0f, .y = 5.0f});
        graphRender.push_back({.x = 1.0f, .y = 5.0f});
        rctx.renderer->renderGraph(graphRender, 0, 1, 3.0f, 1.0f, 1.0f, 1.0f);
    }

    graphRender.resize(glotLength);

    for (int i = 0; i < glotLength; ++i) {
        graphRender[i] = {
            .x = static_cast<float>(i),
            .y = glot[glot.size() - glotLength + i] * 2.5f - 5.0f,
        };
    }
    if (!graphRender.empty()) {
        rctx.renderer->renderGraph(graphRender, 0, glotLength - 1, 3.0f, 1.0f, 0.5f, 0.0f);
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

