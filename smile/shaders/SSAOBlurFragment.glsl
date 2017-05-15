/// @author Tom Collingwood & Joey de Vries of learnopengl.com

#version 410 core
in vec2 TexCoords;

// Code taken from and edited from https://learnopengl.com/#!Advanced-Lighting/SSAO

uniform sampler2D ssaoInput;

layout (location=0) out vec4 FragColor;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x)
    {
        for (int y = -2; y < 2; ++y)
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssaoInput, TexCoords + offset).r;
        }
    }
   FragColor = vec4(result) / (4.0 * 4.0);

    //FragColor = vec4(texture(ssaoInput, TexCoords).xyz, 1.0f);
}
