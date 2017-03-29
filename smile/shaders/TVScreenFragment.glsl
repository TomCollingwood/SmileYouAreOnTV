/// @author Richard Southern

#version 410 core


uniform sampler2D screenTexture;
uniform int iGlobalTime;
uniform sampler2D noiseTex;

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
            vec3(1.00, 1.00, 1.00),        // La
            vec3(0.2, 0.2, 0.2),        // Ld
            vec3(0.6, 0.6, 0.6)         // Ls
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
            vec3(1.0, 0.01, 0.01),    // Kd
            vec3(0.2, 0.2, 0.2),    // Ks
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

// end citation

/************************************************************************************/

// code below taken from https://www.shadertoy.com/view/ldjGzV
// begin citation

float noise(vec2 p)
{
  float s = texture(noiseTex,vec2(2.*sin(iGlobalTime),2.*cos(iGlobalTime))*iGlobalTime*8. + p*1.).x;
  s *= s;
  return s;
}

vec2 screenDistort(vec2 uv)
{
  uv -= vec2(.5,.5);
  uv = uv*1.2*(1./1.2+2.*uv.x*uv.x*uv.y*uv.y);
  uv += vec2(.5,.5);
  return uv;
}

float onOff(float a, float b, float c)
{
  return step(c, sin(iGlobalTime + a*cos(iGlobalTime*b)));
}

vec3 getVideo(vec2 uv)
{
  vec2 look = uv;
  float window = 1./(1.+20.*(look.y-mod(iGlobalTime/4.,1.))*(look.y-mod(iGlobalTime/4.,1.)));
  look.x = look.x + sin(look.y*10. + iGlobalTime)/50.*onOff(4.,4.,.3)*(1.+cos(iGlobalTime*80.))*window;
  //float vShift = 0.4*onOff(2.,3.,.9)*(sin(iGlobalTime)*sin(iGlobalTime*20.) +
  //                   (0.5 + 0.1*sin(iGlobalTime*200.)*cos(iGlobalTime)));
  look.y = mod(look.y ,1);//+ vShift, 1.);
  vec3 video = vec3(texture(screenTexture,look));
  return video;
}
//end of citation


vec3 LightIntensity;




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

  vec3 screenFrag = texture(screenTexture,FragmentTexCoord).rgb;

  float Ks = beckmannSpecular(s,v,n,0.9);
  LightIntensity = (
          Light.La * screenFrag );//+
          //Light.Ld * screenFrag * max( dot(s, FragmentNormal), 0.0 ) +
          //Light.Ls * Material.Ks * pow( max( dot(r,v), 0.0 ), 10.0f ));


    //FragColor = vec4(1.0f,0.0f,0.0f,1.0);

  vec2 uv = screenDistort(FragmentTexCoord);

  float vigAmt = 3.+.3*sin(iGlobalTime + 5.*cos(iGlobalTime*5.));
  float vignette = (1.-vigAmt*(uv.y-.5)*(uv.y-.5))*(1.-vigAmt*(uv.x-.5)*(uv.x-.5));

  vec3 video = getVideo(uv);
  video+=noise(FragmentTexCoord*1.5f)/10.0f;
  video*=vignette;
  FragColor = vec4(video,1.0f);

}
