/// @author Richard Southern

#version 410 core

//uniform sampler2D DiffuseTexure;

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
            vec3(0.1, 0.1, 0.1),        // La
            vec3(0.5, 0.5, 0.5),        // Ld
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
            vec3(0.5, 0.5, 0.5),    // Kd
            vec3(1.0, 1.0, 1.0),    // Ks
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



//https://thebookofshaders.com/edit.php#11/wood.frag
//https://thebookofshaders.com/11/
float noise(vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);
    vec2 u = f*f*(3.0-2.0*f);
    return mix( mix( random( i + vec2(0.0,0.0) ),
                     random( i + vec2(1.0,0.0) ), u.x),
                mix( random( i + vec2(0.0,1.0) ),
                     random( i + vec2(1.0,1.0) ), u.x), u.y);
}

mat2 rotate2d(float angle){
    return mat2(cos(angle),-sin(angle),
                sin(angle),cos(angle));
}

float lines(in vec2 pos, float b){
    float scale = 10.0;
    pos *= scale;
    return smoothstep(0.0,
                    .5+b*.5+ snoise(vec3(pos*2,0.0f)),
                    abs((sin(pos.x*3.1415)+b*2.0))*.5 + snoise(vec3(pos*2.43,0.0f)));
}



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
//


/** From http://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
  */
mat4 rotationMatrix(vec3 axis, float angle)
{
    //axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

/**
  * Rotate a vector vec by using the rotation that transforms from src to tgt.
  */
vec3 rotateVector(vec3 src, vec3 tgt, vec3 vec) {
    float angle = acos(dot(src,tgt));

    // Check for the case when src and tgt are the same vector, in which case
    // the cross product will be ill defined.
    if (angle == 0.0) {
        return vec;
    }
    vec3 axis = normalize(cross(src,tgt));
    mat4 R = rotationMatrix(axis,angle);

    // Rotate the vec by this rotation matrix
    vec4 _norm = R*vec4(vec,1.0);
    return _norm.xyz / _norm.w;
}



const vec2 size = vec2(5.0,0.0);
const vec3 off = vec3(-0.0005,0,0.0005);

vec3 LightIntensity;

void main() {

      vec2 st = FragmentTexCoord.xy;

      vec2 pos = st.yx*vec2(3.,10.);

      float pattern = pos.x;

      // Add noise
      pos = rotate2d( noise(pos) ) * pos;

      // Draw lines
      pattern = lines(pos,.5) + snoise(vec3(st*75,0.0f));


      // modifed from http://stackoverflow.com/questions/5281261/generating-a-normal-map-from-a-height-map
      // CALCULATE NORMAL MAP
      vec2 tmpst = st;
      pos = tmpst.yx*vec2(3.,10.);
      float s11 = 1-lines(rotate2d( noise(pos) ) * pos,.5) + snoise(vec3(tmpst*75,0.0f));

      tmpst = st+off.xy;
      pos = tmpst.yx*vec2(3.,10.);
      float s01 = 1-lines(rotate2d( noise(pos) ) * pos,.5) + snoise(vec3(tmpst*75,0.0f));

      tmpst = st+off.zy;
      pos = tmpst.yx*vec2(3.,10.);
      float s21 = 1-lines(rotate2d( noise(pos) ) * pos,.5) + snoise(vec3(tmpst*75,0.0f));

      tmpst = st+off.yx;
      pos = tmpst.yx*vec2(3.,10.);
      float s10 = 1-lines(rotate2d( noise(pos) ) * pos,.5) + snoise(vec3(tmpst*75,0.0f));

      tmpst = st+off.yz;
      pos = tmpst.yx*vec2(3.,10.);
      float s12 = 1-lines(rotate2d( noise(pos) ) * pos,.5) + snoise(vec3(tmpst*75,0.0f));

      vec3 va = normalize(vec3(size.xy,s21-s01));
      vec3 vb = normalize(vec3(size.yx,s12-s10));
      vec4 bump = vec4( cross(va,vb), 1.0f );

      // Calculate the view vector
      vec3 n = normalize(vec3(FragmentNormal));

      // The source is just up in the Z-direction
      vec3 src = vec3(0.0, 0.0, 1.0);
      // Perturb the normal according to the target
      vec3 np = rotateVector(src, bump.xyz, n);


      // WOOD COLOUR
      vec3 woodback = vec3(55.0f/255.0f,34.0f/255.0f,23.0f/255.0f);
      vec3 woodfront = vec3(84/255,59/255,16/255);

      // Calculate the light vector
      vec3 s = normalize( vec3(Light.Position) - FragmentPosition.xyz );

      // Calculate the direction of camera
      vec3 v = normalize(vec3(-FragmentPosition.xyz));

      // Reflect the light about the surface normal
      vec3 r = reflect( -s, np );

      float power = beckmannSpecular(s,v,np,1.0);

      FragColor =  0.2*pow( max( dot(r,v), 0.0 ), power )+vec4(woodback+0.03*vec3(pattern),1.0); //vec4(power,power,power,1.0);//+ vec4(0.5f,0.5f,0.5f,1.0f) *
}
