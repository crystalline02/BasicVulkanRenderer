# version 460 core

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexcood;

layout(location = 0) out vec4 FragColor;

layout(set = 0, binding = 1) uniform sampler2D image;

void main()
{
    vec3 color = texture(image, fragTexcood).rgb;
    FragColor = vec4(color, 1.f);
}