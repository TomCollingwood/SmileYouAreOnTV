#version 430

// A texture sampler to store the colour information
uniform sampler2D ColourTexture;

// A texture sampler to store the normal information
uniform sampler2D NormalTexture;

// The output colour which will be output to the framebuffer
layout (location=0) out vec4 fragColour;

// Structure for holding light parameters
struct LightInfo {
    vec4 Position; // Light position in eye coords.
    vec3 La; // Ambient light intensity
    vec3 Ld; // Diffuse light intensity
    vec3 Ls; // Specular light intensity
};

// We'll have a single light in the scene with some default values
uniform LightInfo Light = LightInfo(
            vec4(2.0, 2.0, 10.0, 1.0),   // position
            vec3(0.2, 0.2, 0.2),        // La
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
            vec3(0.1, 0.1, 0.1),    // Ka
            vec3(1.0, 0.3, 0.3),    // Kd
            vec3(0.0, 1.0, 0.0),    // Ks
            10.0);                  // Shininess

// Attributes passed on from the vertex shader
smooth in vec3 FragmentPosition;
smooth in vec3 FragmentNormal;
smooth in vec2 FragmentTexCoord;

// below was purloined from http://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}


void main() {  
    // Calculate the normal (this is the expensive bit in Phong)
    vec3 n = normalize( FragmentNormal );

    // ***********************************************************
    // What you'll need to do here is rotate n based on the normal
    // provided in the normal map. After this, the lighting
    // calculations will work below.
    // ***********************************************************

    vec3 texNormal = texture(NormalTexture,1-FragmentTexCoord*10).rgb;

    texNormal=2*texNormal;
    texNormal=texNormal-vec3(1,1,1);
    texNormal = normalize(texNormal);

    float angle = acos(dot(texNormal,vec3(0,0,1)));
    vec3 _cross = cross(texNormal,vec3(0,0,1));

    mat4 rotatMat = rotationMatrix(_cross,angle);
    vec4 tn =rotatMat* vec4(n[0],n[1],n[2],1);
    n = vec3(tn[0],tn[1],tn[2]);
    n=normalize(n);

    // arcos thing for theta
    // cross product to n and p
    // rotate from there

    // find rotation matrix to multiply the normal with
    // rotation from perturbed to normal




    // Calculate the view vector
    vec3 v = normalize(vec3(-FragmentPosition));

    // Calculate the light vector
    vec3 s = normalize( vec3(Light.Position) - FragmentPosition );

    // Reflect the light about the surface normal
    vec3 r = reflect( -s, n );

    // Compute the light from the ambient, diffuse and specular components (phong model)
    vec3 lightColor = (
            Light.La * Material.Ka +
            Light.Ld * Material.Kd * max( dot(s, n), 0.0 ) +
            Light.Ls * Material.Ks * pow( max( dot(r,v), 0.0 ), Material.Shininess ));

    // This is the call that retrieves the colour from the texture map using the
    // givne texture coordinates.
    vec3 texColor = texture(ColourTexture,1- FragmentTexCoord*10).rgb;

    // Use the following shader for the correct value
    fragColour = vec4(texColor,1.0);
}  



