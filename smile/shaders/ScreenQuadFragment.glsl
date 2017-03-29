/// @author Richard Southern

#version 410 core

uniform sampler2D texFramebuffer;
// This is passed on from the vertex shader
in vec2 FragmentTexCoord;

// This is no longer a built-in variable
layout (location=0) out vec4 FragColor;

void main() {
  FragColor = texture(texFramebuffer,FragmentTexCoord);
}
