#version 330 core

in vec3 FragPos;
in vec3 Normal;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;

void main()
{
    // Store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    // Also store the per-fragment normals into the gbuffer
    gNormal = normalize(Normal);
}
