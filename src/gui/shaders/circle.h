#ifndef GUI_SHADERS_CIRCLE_H
#define GUI_SHADERS_CIRCLE_H

namespace Gui::Shaders {

constexpr const char *circleVertex = R"foo(
#version 120
attribute vec4 vertex;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
} 
)foo";

constexpr const char *circleFragment = R"foo(
#version 120

uniform vec2 resolution;
uniform vec2 center;
uniform float radius;
uniform vec3 fillColor;

vec4 circle(vec2 uv, vec2 pos, float rad, vec3 color)
{
    float d = length(pos - uv) - rad;
    float t = clamp(d, 0.0, 1.0);
    return vec4(color, 1.0 - t);
}

void main()
{
    gl_FragColor = circle(gl_FragCoord.xy, center, radius, fillColor);
}  
)foo";

}

#endif // GUI_SHADERS_CIRCLE_H

