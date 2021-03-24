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

uniform sampler2D text;

void main()
{    
    color = texture(text, TexCoords);
}  
)foo";

}

#endif // GUI_SHADERS_SPEC_H
