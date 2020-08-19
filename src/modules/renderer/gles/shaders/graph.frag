#version 300 es

layout (location = 0) in vec4 f_color;

layout (location = 0) out vec4 fragColor;

void main() {
    fragColor = f_color;
}
