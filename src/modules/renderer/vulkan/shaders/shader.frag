#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec4 f_color;

layout (location = 0) out vec4 fragColor;

void main() {
    fragColor = f_color;
}
