/// @author Richard Southern

#version 410 core

uniform sampler2D texFramebuffer;
uniform int width;
uniform int height;
// This is passed on from the vertex shader
in vec2 FragmentTexCoord;

float unity = 1.0f/height;
float unitx = 1.0f/width;

//#define FXAA_ON

// This is no longer a built-in variable
layout (location=0) out vec4 FragColor;

#define FXAA_EDGE_THRESHOLD 1.0f/4.0f
#define FXAA_EDGE_THRESHOLD_MIN 1.0f/16.0f
#define FXAA_SUBPIX 0
#define FXAA_SUBPIX_TRIM 1.0f/3.0f
#define FXAA_SUBPIX_CAP 3.0f/4.0f
#define FXAA_SUBPIX_TRIM_SCALE 1.0f
#define FXAA_SEARCH_STEPS 5
#define FXAA_SEARCH_ACCELERATION 1
#define FXAA_SEARCH_THRESHOLD 1.0f/4.0f

// TIME FOR SOME FXAA BABY

float FxaaLuma(vec3 rgb)
{
 return rgb.y * (0.587/0.299) + rgb.x;
}

vec4 TextureOffset(sampler2D textureinput, vec2 uv, ivec2 offset)
{
  return texture(textureinput,vec2(uv.x+offset.x*unitx,uv.y+offset.y*unity));
}

void main() {
  //http://developer.download.nvidia.com/assets/gamedev/files/sdk/11/FXAA_WhitePaper.pdf
  // FXAAAA

 #ifdef FXAA_ON

  vec3 rgbN = TextureOffset(texFramebuffer,FragmentTexCoord,ivec2(0,-1)).xyz;
  vec3 rgbW = TextureOffset(texFramebuffer,FragmentTexCoord,ivec2(-1,0)).xyz;
  vec3 rgbM = TextureOffset(texFramebuffer,FragmentTexCoord,ivec2(0,0)).xyz;
  vec3 rgbE = TextureOffset(texFramebuffer,FragmentTexCoord,ivec2(1,0)).xyz;
  vec3 rgbS = TextureOffset(texFramebuffer,FragmentTexCoord,ivec2(0,1)).xyz;

  float lumaN = FxaaLuma(rgbN);
  float lumaW = FxaaLuma(rgbW);
  float lumaM = FxaaLuma(rgbM);
  float lumaE = FxaaLuma(rgbE);
  float lumaS = FxaaLuma(rgbS);

  float rangeMin = min(lumaM, min(min(lumaN, lumaW), min(lumaS, lumaE)));
  float rangeMax = max(lumaM, max(max(lumaN, lumaW), max(lumaS, lumaE)));
  float range = rangeMax - rangeMin;

  if(range < max(FXAA_EDGE_THRESHOLD_MIN, rangeMax * FXAA_EDGE_THRESHOLD))
  {
   FragColor = vec4(rgbM,1.0f);
  }
  else
  {
    float lumaL = (lumaN + lumaW + lumaE + lumaS) * 0.25;
    float rangeL = abs(lumaL - lumaM);
    float blendL = max(0.0, (rangeL / range) - FXAA_SUBPIX_TRIM) * FXAA_SUBPIX_TRIM_SCALE;
    blendL = min(FXAA_SUBPIX_CAP, blendL);

    vec3 rgbL = rgbN + rgbW + rgbM + rgbE + rgbS;
    vec3 rgbNW = TextureOffset(texFramebuffer,FragmentTexCoord,ivec2(-1,-1)).xyz;
    vec3 rgbNE = TextureOffset(texFramebuffer,FragmentTexCoord,ivec2(1,-1)).xyz;
    vec3 rgbSW = TextureOffset(texFramebuffer,FragmentTexCoord,ivec2(-1,1)).xyz;
    vec3 rgbSE = TextureOffset(texFramebuffer,FragmentTexCoord,ivec2(1,1)).xyz;
    float lumaNW = FxaaLuma(rgbNW);
    float lumaNE = FxaaLuma(rgbNE);
    float lumaSW = FxaaLuma(rgbSW);
    float lumaSE = FxaaLuma(rgbSE);

    rgbL += (rgbNW + rgbNE + rgbSW + rgbSE);
    rgbL *= vec3(1.0f/9.0f);

    float edgeVert =
     abs((0.25 * lumaNW) + (-0.5 * lumaN) + (0.25 * lumaNE)) +
     abs((0.50 * lumaW ) + (-1.0 * lumaM) + (0.50 * lumaE )) +
     abs((0.25 * lumaSW) + (-0.5 * lumaS) + (0.25 * lumaSE));
    float edgeHorz =
     abs((0.25 * lumaNW) + (-0.5 * lumaW) + (0.25 * lumaSW)) +
     abs((0.50 * lumaN ) + (-1.0 * lumaM) + (0.50 * lumaS )) +
     abs((0.25 * lumaNE) + (-0.5 * lumaE) + (0.25 * lumaSE));
    bool horzSpan = edgeHorz >= edgeVert;

    bool doneN = false;
    bool doneP = false;
    float lumaEndN;
    float lumaEndP;

    for(uint i = 0; i < FXAA_SEARCH_STEPS; i++) {
       #if FXAA_SEARCH_ACCELERATION == 1
       if(!doneN) lumaEndN = FxaaLuma(FxaaTexture(tex, posN.xy).xyz);
       if(!doneP) lumaEndP = FxaaLuma(FxaaTexture(tex, posP.xy).xyz);
       #else
       if(!doneN) lumaEndN = FxaaLuma(
       FxaaTextureGrad(tex, posN.xy, offNP).xyz);
       if(!doneP) lumaEndP = FxaaLuma(
       FxaaTextureGrad(tex, posP.xy, offNP).xyz);
       #endif
       doneN = doneN || (abs(lumaEndN - lumaN) >= gradientN);
       doneP = doneP || (abs(lumaEndP - lumaN) >= gradientN);
       if(doneN && doneP) break;
       if(!doneN) posN -= offNP;
       if(!doneP) posP += offNP;
    }
  }
#else
  FragColor = texture(texFramebuffer,FragmentTexCoord);

#endif
}
