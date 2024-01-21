# version 460 core

vec2 positions[] = {
    vec2(-0.5f, -0.5f), 
    vec2(0.f, 0.5f), 
    vec2(0.5f, -0.5f)};

vec3 colors[] = {
    vec3(1.f, 0.f, 0.f),
    vec3(0.f, 1.f, 0.f),
    vec3(0.f, 0.f, 1.f)
};

layout(location = 0) out vec3 vertex_color;

void main()
{
    gl_Position = vec4(positions[gl_VertexIndex], 0.f, 1.f);
    vertex_color = colors[gl_VertexIndex];
}