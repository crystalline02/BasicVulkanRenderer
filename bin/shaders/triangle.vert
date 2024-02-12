# version 460 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoord;

layout(location = 0) out vec3 vertex_color;
layout(location = 1) out vec2 texture_coord;

layout(set = 0, binding = 0) uniform Matrices
{
    mat4 model;
    mat4 view;
    mat4 projection;
} matrices;

void main()
{
    gl_Position = matrices.projection * matrices.view * matrices.model * vec4(aPos, 0.f, 1.f);
    vertex_color = aColor;
    texture_coord = aTexCoord;
}