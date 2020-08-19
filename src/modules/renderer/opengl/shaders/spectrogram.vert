#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 point;
layout (location = 0) out vec4 f_color;

layout (location = 1) uniform float minFreq;
layout (location = 2) uniform float maxFreq;
layout (location = 3) uniform float minGain;
layout (location = 4) uniform float maxGain;
layout (location = 5) uniform uint scaleType;

float mel(float f) {
    const float invlog10 = 0.434294481903251827651128;
    return 2595.0 * log(1.0 + f / 700.0) * invlog10;
}

float linearToCoord(float x, float min, float max) {
    return 2.0 * (x - min) / (max - min) - 1.0;
}

float frequencyToCoordLinear(float f) {
    return linearToCoord(f, minFreq, maxFreq);
}

float frequencyToCoordLog(float f) {
    return linearToCoord(log(f), log(minFreq), log(maxFreq));
}

float frequencyToCoordMel(float f) {
    return linearToCoord(mel(f), mel(minFreq), mel(maxFreq));
}

float frequencyToCoord(float f) {
    if (scaleType == 0u) return frequencyToCoordLinear(f);
    if (scaleType == 1u) return frequencyToCoordLog(f);
    if (scaleType == 2u) return frequencyToCoordMel(f);
}

void main() {
    float time = point[0];
    float frequency = point[1];
    float gain = point[2];

    gl_Position = vec4(time, frequencyToCoord(frequency), 0.0, 1.0);

    float a = clamp((gain - minGain) / (maxGain - minGain), 0.0, 1.0);

    float r = 5.0 * (a - 0.2);
    float g = 5.0 * (a - 0.6);
    float b;
    if (a > 0.8)      b = 5.0 * (a - 0.8);
    else if (a > 0.4) b = 5.0 * (0.6 - a);
    else              b = 5.0 * a;

    r = clamp(r, 0.0, 0.98);
    g = clamp(g, 0.0, 0.98);
    b = clamp(b, 0.0, 0.98);

    f_color = vec4(r, g, b, 1.0);
}
