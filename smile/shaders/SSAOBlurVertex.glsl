
/// @author Richard Southern

#version 410 core

// The modelview and projection matrices are no longer given in OpenGL 4.2


/* NOTES
        -uniform is the same for every one of million verticies / parallel processes
        -attributes "in" are different for each vertex / parallel process
*/

// The vertex position attribute
layout (location=0) in vec3 VertexPosition;

// The vertex normal attribute
layout (location=2) in vec3 VertexNormal;

// The vertex color attribute
layout (location=1) in vec2 VertexTexCoord;

smooth out vec2 TexCoords;

/************************************************************************************/
void main() {
    // Set the position of the current vertex
    gl_Position =vec4(VertexPosition, 1.0);
    TexCoords = VertexTexCoord;
}
