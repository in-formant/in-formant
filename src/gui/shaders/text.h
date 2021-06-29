#ifndef GUI_SHADERS_TEXT_H
#define GUI_SHADERS_TEXT_H

namespace Gui::Shaders {

constexpr const char *textVertex = R"foo(
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

constexpr const char *textFragment = R"foo(
#version 120
varying vec2 TexCoords;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture2D(text, TexCoords).r);
    gl_FragColor = vec4(textColor, 1.0) * sampled;
}  
)foo";

}

#endif // GUI_SHADERS_TEXT_H

