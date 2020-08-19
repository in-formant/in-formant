#version 300 es

layout (location = 0) in vec3 point;
layout (location = 0) out vec4 f_color;

layout (location = 1) uniform float minFreq;
layout (location = 2) uniform float maxFreq;
layout (location = 3) uniform uint scaleType;

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
    float intensity = point[2];

    gl_Position = vec4(time, frequencyToCoord(frequency), 0.0, 1.0);
    f_color = vec4(intensity, intensity, intensity, 1.0);
}
