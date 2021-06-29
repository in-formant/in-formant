#ifndef GUI_SHADERS_SPEC_H
#define GUI_SHADERS_SPEC_H

namespace Gui::Shaders {

constexpr const char *specVertex = R"foo(
#version 120

attribute vec4 vertex; // <vec2 pos, vec2 tex>
varying vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
} 
)foo";

constexpr const char *specFragment = R"foo(
#version 120

varying vec2 TexCoords;

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
    if (frequencyScale == 1)
        return log(f) * INV_LOG_2;
    else if (frequencyScale == 2)
        return 2595.0 * log(1.0 + f / 700.0) * INV_LOG_10;
    else if (frequencyScale == 3)
        return ERB_A * log(1.0 + 0.00437 * f) * INV_LOG_10;
    else
        return f;
}

float reverse(float v) {
    if (frequencyScale == 1)
        return pow(2.0, v);
    else if (frequencyScale == 2)
        return 700.0 * (pow(10.0, v / 2595.0) - 1.0);
    else if (frequencyScale == 3)
        return (pow(10.0, v / ERB_A) - 1.0) / 0.00437;
    else
        return v;
}

void main()
{
    // TexCoords: [0,1] => [0,fftFrequency]
    // FragCoord: [0,1] => [0,maxFrequency]

    int chunkIndex = int(floor(mod(TexCoords.x, 1.0) * 2048.0));

    float txl_x = (2.0 * float(chunkIndex) + 1.0) / (2.0 * 2048.0);
    float txl_y1 = (2.0 * 4096.0 + 1.0) / (2.0 * 4098.0);
    float txl_y2 = (2.0 * 4097.0 + 1.0) / (2.0 * 4098.0);

    int nfft = int(texture2D(tex, vec2(txl_x, txl_y1)));
    float sampleRate = float(texture2D(tex, vec2(txl_x, txl_y2)));

    float ty = TexCoords.y;

    float freq = reverse(transform(minFrequency) + ty * (transform(maxFrequency) - transform(minFrequency)));

    ty = freq / (sampleRate / 2.0);
    
    ty *= (float(nfft) / 4096.0);
   
    if (ty < 0.0 || ty >= 4096.0 / 4098.0) {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else {
        float amplitude = float(texture2D(tex, vec2(TexCoords.x, ty)));

        float adjusted = sqrt(amplitude / pow(10.0, maxGain / 20.0)) * 7.0;
        int index = int(clamp(floor(adjusted * 255.0), 0.0, 255.0));

        gl_FragColor = vec4(colorMap[index], 1.0);
    }
}  
)foo";

}

#endif // GUI_SHADERS_SPEC_H
