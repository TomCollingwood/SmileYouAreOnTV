/// @author Tom Collingwood

#version 410 core

uniform mat4 MV;
uniform mat4 MVP;
uniform mat3 N;


// The vertex position attribute
layout (location=0) in vec3 VertexPosition;

// The vertex normal attribute
layout (location=2) in vec3 VertexNormal;

smooth out vec3 FragPos;
smooth out vec3 Normal;


void main()
{
    gl_Position = MVP * vec4(VertexPosition, 1.0);
    FragPos = vec3(MV * vec4(VertexPosition,1.0));
    Normal = N * VertexNormal;
}
