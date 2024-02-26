# version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

layout(location = 0) out vec3 vertexNormal;
layout(location = 1) out vec2 vertexTexcoord;

layout(set = 0, binding = 0) uniform Matrices
{
    mat4 model;
    mat4 view;
    mat4 projection;
} matrices;

void main()
{
    gl_Position = matrices.projection * matrices.view * matrices.model * vec4(aPos, 1.f);
    vertexNormal = aNormal;
    vertexTexcoord = aTexCoord;
}