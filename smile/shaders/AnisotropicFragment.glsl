/// @author Richard Southern

#version 410 core

// This is passed on from the vertex shader
//in vec3 LightIntensity;
in vec3 FragmentPosition;
in vec3 FragmentNormal;
in vec2 FragmentTexCoord;
in vec3 FragmentWorldSpace;
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


/************************************************************************************/
vec3 LightIntensity;

void main() {

//  V = v
//  R = r
//  L = s
//  N = FragmentNormal

    vec3 n = normalize( FragmentNormal );
    vec3 Li = normalize( vec3(Light.Position) - FragmentPosition.xyz );
    vec3 Vi = normalize( - FragmentPosition.xyz );

    //vec4 tangeant = vec4(texture(AnasTexure, FragmentTexCoord).rgb,1.0f);
    vec2 ourTang = FragmentTexCoord*2 -1;
    vec4 tangeant = vec4(ourTang.x,ourTang.y,0.0f,1.0f);
    tangeant = vec4(normalize(tangeant.rgb),1.0f);

    float stretch = 50.0f;
    vec3 stretchedWorldSpace = vec3(FragmentWorldSpace.x*(1+tangeant.y*stretch),FragmentWorldSpace.y*(1+tangeant.x*stretch),FragmentWorldSpace.z*(1+stretch));
    vec3 KsNOISE = vec3(snoise(stretchedWorldSpace*5));

    vec3 Ks = vec3(0.6f) + 0.3f*KsNOISE;//vec3(texture(SpecTexure,FragmentTexCoord).rgb);
    //vec3 Kd = vec3(snoise(FragmentWorldSpace));//vec3(texture(DiffuseTexure,FragmentTexCoord).rgb);
    vec3 Kd = vec3(0.6f) + 0.1*KsNOISE;

    // vec4 tangeant  = vec3(MV * vec4(texture(AnasTexure, FragmentTexCoord).rgb,1.0));
    mat4 ourMat;
    ourMat[0] = vec4(Li,1.0f);
    ourMat[1] = vec4(Vi,1.0f);
    ourMat[2] = vec4(0.0f,0.0f,0.0f,0.0f);
    ourMat[3] = vec4(0.0f,0.0f,0.0f,2.0f);
    vec4 whatWeNeed = 0.5f * transpose(ourMat) * tangeant;

    float LT = 2.0f*whatWeNeed[0] - 1.0f;
    float VT = 2.0f*whatWeNeed[1] - 1.0f;

    float LN = sqrt(1-LT*LT);
    float VR = LN*sqrt(1-VT*VT) - LT*VT;

    LightIntensity = (
            Light.La *  Kd +
             max(dot(n,Li),0.0f)*
            (Light.Ld *  Kd * max(LN,0.0f)+
            Light.Ls *  Ks * pow(max(VR,0.0f),100.0f)));


    FragColor = vec4(LightIntensity,1.0);
}
