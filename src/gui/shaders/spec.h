#ifndef GUI_SHADERS_SPEC_H
#define GUI_SHADERS_SPEC_H

namespace Gui::Shaders {

constexpr const char *specVertex = R"foo(
#version 130

in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
} 
)foo";

constexpr const char *specFragment = R"foo(
#version 130

out vec4 color;
in vec2 TexCoords;

uniform sampler2D tex;

uniform vec3 colorMap[256];

uniform int frequencyScale; // Linear, Log, Mel, ERB
uniform float minFrequency;
uniform float maxFrequency;

uniform float maxGain;

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

    int chunkIndex = int(floor(mod(TexCoords.x, 1) * 2048));

    int nfft = int(texelFetch(tex, ivec2(chunkIndex, 4096), 0));
    float sampleRate = float(texelFetch(tex, ivec2(chunkIndex, 4097), 0));

    float ty = TexCoords.y;

    float freq = reverse(transform(minFrequency) + ty * (transform(maxFrequency) - transform(minFrequency)));

    ty = freq / (sampleRate / 2.0);
    
    ty *= (float(nfft) / 4096.0);
   
    if (ty < 0 || ty > 1) {
        color = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else {
        float amplitude = float(texture(tex, vec2(TexCoords.x, ty)));

        float adjusted = sqrt(amplitude / pow(10, maxGain / 20)) * 7;
        int index = (int) clamp(floor(adjusted * 255), 0, 255);

        color = vec4(colorMap[index], 1.0);
    }
}  
)foo";

}

#endif // GUI_SHADERS_SPEC_H
