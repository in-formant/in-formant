#version 450 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
layout (location = 0) out vec2 TexCoords;

layout (location = 4) uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
