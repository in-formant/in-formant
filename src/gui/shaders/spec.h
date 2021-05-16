#ifndef GUI_SHADERS_SPEC_H
#define GUI_SHADERS_SPEC_H

namespace Gui::Shaders {

constexpr const char *specVertex = R"foo(
#version 330
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
} 
)foo";

constexpr const char *specFragment = R"foo(
#version 330
in vec2 TexCoords;
out vec4 color;

uniform sampler2D tex;

uniform vec3 colorMap[256];

uniform float fftFrequency;

uniform int frequencyScale; // Linear, Log, Mel, ERB
uniform float minFrequency;
uniform float maxFrequency;

#define INV_LOG_2   1.44269504089
#define INV_LOG_10  0.4342944819
#define ERB_A       21.33228113095401739888262

float transform(float f) {
    if (frequencyScale == 0)
        return f;
    else if (frequencyScale == 1)
        return log(f) * INV_LOG_2;
    else if (frequencyScale == 2)
        return 2595 * log(1 + f / 700) * INV_LOG_10;
    else if (frequencyScale == 3)
        return ERB_A * log(1 + 0.00437 * f) * INV_LOG_10;
}

float reverse(float v) {
    if (frequencyScale == 0)
        return v;
    else if (frequencyScale == 1)
        return pow(2, v);
    else if (frequencyScale == 2)
        return 700 * (pow(10, v / 2595) - 1);
    else if (frequencyScale == 3)
        return (pow(10, v / ERB_A) - 1) / 0.00437;
}

void main()
{
    // TexCoords: [0,1] => [0,fftFrequency]
    // FragCoord: [0,1] => [0,maxFrequency]

    float ty = 1 - TexCoords.y;

    float freq = reverse(transform(minFrequency) + ty * transform(maxFrequency));

    ty = freq / fftFrequency;
   
    if (ty < 0 || ty > 1) {
        color = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else { 
        float amplitude = float(texture(tex, vec2(TexCoords.x, ty)));

        color = vec4(vec3(1.0, 0.2, 0.0) * sqrt(amplitude) * 15, 1.0);
    }
}  
)foo";

}

#endif // GUI_SHADERS_SPEC_H
