/// @author Richard Southern

#version 410 core

uniform int width;
uniform int height;
// This is passed on from the vertex shader
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];
uniform mat4 projection;

in vec2 FragmentTexCoord;

layout (location=0) out vec4 FragColor;

void main() {
  vec2 noiseScale = vec2(width/4.0, height/4.0);

  vec3 fragPos   = texture(gPosition, FragmentTexCoord).xyz;
  vec3 normal    = texture(gNormal, FragmentTexCoord).rgb;
  vec3 randomVec = texture(texNoise, FragmentTexCoord * noiseScale).xyz;

  vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
  vec3 bitangent = cross(normal, tangent);
  mat3 TBN       = mat3(tangent, bitangent, normal);

  float occlusion = 0.0;
  float radius = 0.1;
  float kernelSize = 64;
  float bias = 0.025;

  for(int i = 0; i < 64; ++i)
  {
      // get sample position
      vec3 sample1 = TBN * samples[i]; // From tangent to view-space
      sample1 = fragPos + sample1 * radius;

      vec4 offset = vec4(sample1, 1.0);
      offset      = projection * offset;    // from view to clip-space
      offset.xyz /= offset.w;               // perspective divide
      offset.xyz  = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0

      float sampleDepth = texture(gPosition, offset.xy).z;
      float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));

      occlusion += (sampleDepth >= sample1.z + bias ? 1.0 : 0.0) * rangeCheck;
  }


  occlusion = 1.0 - (float(occlusion) / kernelSize);
  FragColor = vec4(vec3(occlusion),1.0f);

}
