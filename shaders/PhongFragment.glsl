/// @author Richard Southern

#version 410 core

uniform sampler2D anasTex;
// This is passed on from the vertex shader
//in vec3 LightIntensity;
in vec3 FragmentPosition;
in vec3 FragmentNormal;
in vec2 FragmentAnasCoord;
in mat4 _MV;

// This is no longer a built-in variable
layout (location=0) out vec4 FragColor;

/************************************************************************************/

// Structure for holding light parameters
struct LightInfo {
    vec4 Position; // Light position in eye coords.
    vec3 La; // Ambient light intensity
    vec3 Ld; // Diffuse light intensity
    vec3 Ls; // Specular light intensity
};

// We'll have a single light in the scene with some default values
uniform LightInfo Light = LightInfo(
            vec4(-2.0, -2.0, 10.0, 1.0),   // position
            vec3(0.9, 0.9, 0.9),        // La
            vec3(1.0, 1.0, 1.0),        // Ld
            vec3(1.0, 1.0, 1.0)         // Ls
            );

// The material properties of our object
struct MaterialInfo {
    vec3 Ka; // Ambient reflectivity
    vec3 Kd; // Diffuse reflectivity
    vec3 Ks; // Specular reflectivity
    float Shininess; // Specular shininess factor
};

// The object has a material
uniform MaterialInfo Material = MaterialInfo(
            vec3(0.6, 0.6, 0.6),    // Ka
            vec3(1.0, 1.0, 1.0),    // Kd
            vec3(1.0, 1.0, 1.0),    // Ks
            3.0                    // Shininess
            );

/************************************************************************************/
vec3 LightIntensity;




void main() {
    // Transform your input normal
    vec3 n = normalize( FragmentNormal );

    // Calculate the light vector
    vec3 s = normalize( vec3(Light.Position) - FragmentPosition.xyz );

    // Calculate the vertex position
    vec3 v = normalize(vec3(-FragmentPosition.xyz));

    // Reflect the light about the surface normal
    vec3 r = reflect( -s, n );


    // V = v
    // R = r
    // L = s
    // N = FragmentNormal



//              LightIntensity = (
//                      Light.La *  Material.Ka +
//                      Light.Ld *  Material.Kd * max( dot(s, FragmentNormal), 0.0 ) +
//                      Light.Ls *  Material.Ks * pow( max( dot(r,v), 0.0 ), Material.Shininess ));



    vec4 tangeant = vec4(texture(anasTex, FragmentAnasCoord).rgb,1.0f);
    mat4 ourMat;
    ourMat[0] = vec4(s,1.0f);
    ourMat[1] = vec4(v,1.0f);
    ourMat[2] = vec4(0.0f);
    ourMat[3] = vec4(0.0f,0.0f,0.0f,1.0f);
    vec4 whatWeNeed = 0.5f * ourMat * tangeant;

    float LT = 2.0f*whatWeNeed[0] - 1.0f;
    float VT = 2.0f*whatWeNeed[1] - 1.0f;

    float Idiffuse = sqrt(1-VT*VT);
    float Ispec = Idiffuse*sqrt(1-VT*VT) - LT*VT;

    float ourp = dot(-FragmentNormal,s);


    LightIntensity = (
            Light.La *  Material.Ka +
            Light.Ld *  Material.Kd * Idiffuse +
            Light.Ls *  Material.Ks * Ispec);







    //FragColor = vec4(texture(anasTex, FragmentAnasCoord).rgb,1.0f);
    FragColor = vec4(LightIntensity,1.0);
    //FragColor = vec4(1.0f);

}
