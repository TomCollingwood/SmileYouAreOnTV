/// @author Richard Southern

#version 410 core

#define M_PI 3.14159265358979323846

// This is passed on from the vertex shader
//in vec3 LightIntensity;
in vec3 FragmentPosition;
in vec3 FragmentNormal;
in vec2 FragmentTexCoord;
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
            vec4(0.0, 0.0, 10.0, 1.0),   // position
            vec3(0.05, 0.05, 0.05),        // La
            vec3(0.2, 0.2, 0.2),        // Ld
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
            vec3(0.01, 0.01, 0.01),    // Ka
            vec3(0.06, 0.06, 0.06),    // Kd
            vec3(0.3, 0.3, 0.3),    // Ks
            3.0                    // Shininess
            );

// below is taken from https://github.com/stackgl/glsl-specular-beckmann
// begin citation

float beckmannDistribution(float x, float roughness) {
  float NdotH = max(x, 0.0001);
  float cos2Alpha = NdotH * NdotH;
  float tan2Alpha = (cos2Alpha - 1.0) / cos2Alpha;
  float roughness2 = roughness * roughness;
  float denom = 3.141592653589793 * roughness2 * cos2Alpha * cos2Alpha;
  return exp(tan2Alpha / roughness2) / denom;
}

float beckmannSpecular(
  vec3 lightDirection,
  vec3 viewDirection,
  vec3 surfaceNormal,
  float roughness) {
  return beckmannDistribution(dot(surfaceNormal, normalize(lightDirection + viewDirection)), roughness);
}

/************************************************************************************/
vec3 LightIntensity;

//https://thebookofshaders.com/edit.php#11/wood.frag
//https://thebookofshaders.com/11/
float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))
                * 43758.5453123);
}

// Value noise by Inigo Quilez - iq/2013
// https://www.shadertoy.com/view/lsf3WH

vec3 random3(vec3 c) {
    float j = 4096.0*sin(dot(c,vec3(17.0, 59.4, 15.0)));
    vec3 r;
    r.z = fract(512.0*j);
    j *= .125;
    r.x = fract(512.0*j);
    j *= .125;
    r.y = fract(512.0*j);
    return r-0.5;
}

const float F3 =  0.3333333;
const float G3 =  0.1666667;
float snoise(vec3 p) {

    vec3 s = floor(p + dot(p, vec3(F3)));
    vec3 x = p - s + dot(s, vec3(G3));

    vec3 e = step(vec3(0.0), x - x.yzx);
    vec3 i1 = e*(1.0 - e.zxy);
    vec3 i2 = 1.0 - e.zxy*(1.0 - e);

    vec3 x1 = x - i1 + G3;
    vec3 x2 = x - i2 + 2.0*G3;
    vec3 x3 = x - 1.0 + 3.0*G3;

    vec4 w, d;

    w.x = dot(x, x);
    w.y = dot(x1, x1);
    w.z = dot(x2, x2);
    w.w = dot(x3, x3);

    w = max(0.6 - w, 0.0);

    d.x = dot(random3(s), x);
    d.y = dot(random3(s + i1), x1);
    d.z = dot(random3(s + i2), x2);
    d.w = dot(random3(s + 1.0), x3);

    w *= w;
    w *= w;
    d *= w;

    return dot(d, vec4(52.0));
}

// end


void main() {
  // Transform your input normal
  vec3 n = normalize( FragmentNormal );

  // Calculate the light vector
  vec3 s = normalize( vec3(Light.Position) - FragmentPosition.xyz );

  // Calculate the direction of camera
  vec3 v = normalize(vec3(-FragmentPosition.xyz));

  // Reflect the light about the surface normal
  vec3 r = reflect( -s, n );

  // Compute the light from the ambient, diffuse and specular components
  // PHONG

  float power = beckmannSpecular(s,v,n,0.4);
//  LightIntensity = (
//          Light.La * Material.Ka +
//          Light.Ld * Material.Kd * max( dot(s, FragmentNormal), 0.0 ) +
//          Light.Ls * Material.Ks * pow( max( dot(r,v), 0.0 ), KsPow ));



    //FragColor = vec4(1.0f,0.0f,0.0f,1.0);
  FragColor =0.07f*vec4(power,power,power,1.0)*(1+0.2*snoise(vec3(FragmentTexCoord*600,0.0f)));//+ noise(FragmentTexCoord) ;

}
