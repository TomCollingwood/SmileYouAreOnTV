/// @author Richard Southern

#version 410 core


uniform sampler2D screenTexture;
uniform sampler2D noiseTexture;
uniform int iGlobalTime;

uniform int width;
uniform int height;

uniform float _xscale;
uniform float _yscale;
uniform float _brightness;
uniform int _tvon;
uniform int _camera;
uniform bool _vignetteon;
uniform bool _noiseon;

// This is passed on from the vertex shader
//in vec3 LightIntensity;
in vec3 FragmentPosition;
in vec3 FragmentNormal;
in vec2 FragmentTexCoord;
in mat4 _MV;

// This is no longer a built-in variable
layout (location=0) out vec4 FragColor;

float kernel[9];
vec2 offsets[9];
vec3 sampleTex[9];

vec3 col;




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


/************************************************************************************/



//https://thebookofshaders.com/edit.php#11/wood.frag
//https://thebookofshaders.com/11/
float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))
                * 43758.5453123);
}
float noise(vec2 st) {
    vec2 newst = vec2(st.x*sin(iGlobalTime)+st.x,cos(iGlobalTime)*iGlobalTime*8.+st.y);
    vec2 i = floor(newst);
    vec2 f = fract(newst);
    vec2 u = f*f*(3.0-2.0*f);
    return mix( mix( random( i + vec2(0.0,0.0) ),
                     random( i + vec2(1.0,0.0) ), u.x),
                mix( random( i + vec2(0.0,1.0) ),
                     random( i + vec2(1.0,1.0) ), u.x), u.y);
}


// code below edited from https://www.shadertoy.com/view/ldjGzV
// begin citation

vec2 screenDistort(vec2 uv)
{

  uv -= vec2(.5,.5);
  uv = uv*1.2*(1./1.2+2.*uv.x*uv.x*uv.y*uv.y);
  uv += vec2(.5,.5);

  float fwidth = width;
  float fheight = height;

  float ratio = fwidth/fheight;
  float fourbythree = 4.0f/3.0f;

  float diff = ratio-fourbythree;
  if(_camera==1)
  {
  if(diff>0)
  {
    uv.y*=fourbythree/ratio;
    uv.y+=(1.0f-ratio/fourbythree)/2.0f;
  }
  else
  {
    uv.x*=fourbythree/ratio;
    uv.x+=(1.0f-fourbythree/ratio)/2.0f;
  }
  }




  return uv;
}

float onOff(float a, float b, float c)
{
  return step(c, sin(iGlobalTime + a*cos(iGlobalTime*b)));
}

vec3 getVideo(vec2 uv,float xscale, float yscale)
{
  vec2 look = uv;
  float window = 1./(1.+20.*(look.y-mod(iGlobalTime/4.,1.))*(look.y-mod(iGlobalTime/4.,1.)));
  look.x = look.x + sin(look.y*10. + iGlobalTime)/50.*onOff(4.,4.,.3)*(1.+cos(iGlobalTime*80.))*window;
  //float vShift = 0.4*onOff(2.,3.,.9)*(sin(iGlobalTime)*sin(iGlobalTime*20.) +
  //                   (0.5 + 0.1*sin(iGlobalTime*200.)*cos(iGlobalTime)));
  look.y = mod(look.y ,1);//+ vShift, 1.);
  vec3 video ;//= vec3(texture(screenTexture,look));


  // below adapted from https://learnopengl.com/#!Advanced-OpenGL/Framebuffers Accesed 17/02
  float fwidth = width;
  float fheight = height;
  float offsetx = 1.0f/fwidth;
  float offsety = 1.0f/fheight;


  offsets = vec2[](
    vec2(-offsetx, offsety),  // top-left
    vec2(0.0f,    offsety),  // top-center
    vec2(offsetx,  offsety),  // top-right
    vec2(-offsetx, 0.0f),    // center-left
    vec2(0.0f,    0.0f),    // center-center
    vec2(offsetx,  0.0f),    // center-right
    vec2(-offsetx, -offsety), // bottom-left
    vec2(0.0f,    -offsety), // bottom-center
    vec2(offsetx,  -offsety)  // bottom-right
    );
    kernel = float[](
    0.0625, 0.125, 0.0625,
    0.125,  0.25,  0.125,
    0.0625, 0.125, 0.0625
    );

  look.x*=xscale;
  look.y*=yscale;

  look.x-=1.0f*(xscale-1.0f)/2;
  look.y-=1.0f*(yscale-1.0f)/2;

  //look.x=clamp(uv.x,0.0f,1.0f);
  //look.y=clamp(uv.y,0.0f,1.0f);

// if clamped
  vec3 col = vec3(0.0);
  // please forgive me shader overlords
  if(look.x<1.0f && look.x>0.0f && look.y<1.0f && look.y>0.0f)
  {

    for(int i = 0; i < 9; i++)
    {
    sampleTex[i].r = vec3(texture(screenTexture, vec2(look.x + offsets[i].x + 0.005f,look.y+ offsets[i].y))).r;
    sampleTex[i].g = vec3(texture(screenTexture, vec2(look.x + offsets[i].x,look.y+ offsets[i].y))).g;
    sampleTex[i].b = vec3(texture(screenTexture, vec2(look.x + offsets[i].x - 0.005f,look.y+ offsets[i].y))).b;
    col += sampleTex[i] * kernel[i];
    }
  }
    video = col;




//  video[0]= texture(screenTexture,vec2(look.x+0.005f,look.y)).r;
//  video[1]= texture(screenTexture,vec2(look.x,look.y)).g;
//  video[2]= texture(screenTexture,vec2(look.x-0.005f,look.y)).b;
  return video;
}
//end of citation


vec3 LightIntensity;


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
///end


void main() {
  //-----------------------------------------------------------------------------
  // Transform your input normal
  vec3 n = normalize( FragmentNormal );

  // Calculate the light vector
  vec3 s = normalize( vec3(Light.Position) - FragmentPosition.xyz );

  // Calculate the direction of camera
  vec3 v = normalize(vec3(-FragmentPosition.xyz));

  // Reflect the light about the surface normal
  vec3 r = reflect( -s, n );
  //-----------------------------------------------------------------------------

  vec2 uv = screenDistort(FragmentTexCoord);

  float vigAmt = 4.+.3*sin(iGlobalTime + 5.*cos(iGlobalTime*5.));
  float vignette = (1.-vigAmt*(uv.y-.5)*(uv.y-.5))*(1.-vigAmt*(uv.x-.5)*(uv.x-.5));

  vec3 video = getVideo(uv,_xscale,_yscale)*_brightness;

  if(_noiseon) video+=noise(FragmentTexCoord*100.0f)/10.0f;

  if(_vignetteon) video*=vignette;

//  float speedcol = 3.0f;
//  float amount = 0.1f;
//  video[0]+=mix(-amount,amount,(1.0f+sin(iGlobalTime/speedcol)/2.0f));
//  video[1]+=mix(-amount,amount,(1.0f+cos(iGlobalTime/speedcol)/2.0f));
//  video[2]+=mix(-amount,amount,(1.0f+sin(iGlobalTime/speedcol + 180))/2.0f);


  float power = 0.001f* beckmannSpecular(s,v,n,0.03);

  FragColor = vec4(_tvon*video,1.0f)+vec4(power,power,power,1.0f);



}
