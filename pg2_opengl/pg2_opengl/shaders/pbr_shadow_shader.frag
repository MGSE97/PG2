#version 450 core
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : require

#define PI 3.14159265359

#define LightDistanceDevider 200.0
#define ShadowBias 0.0001
#define ShadowAmmount 0.1
#define ShadowPCF 5

#define AOSamples 64
#define AOBias 0.1
#define AORadius 1
#define AOPCF 5

uniform vec3 light;
uniform vec3 eye;
uniform mat4 pv;

in vec3 pos;
in vec2 tex;
in vec3 norm;
in mat3 TBN;
flat in int matIdx;

in vec3 position_lcs;
uniform uint64_t shadow_map; // light's shadow map

uniform bool ssao_enabled; // ssao active
uniform uint64_t ssao_map; // ssao map
uniform uint64_t ssao_noise; // ssao noise
uniform vec3 ssao_samples[AOSamples];   // ssao samples

uniform uint64_t brdf_map;
uniform uint64_t ir_map;
uniform uint64_t pref_env_map;
uniform int pref_env_map_lvl;

out vec4 FragColor;

layout (bindless_sampler) uniform;

struct Material {
	vec3 diffuse; // (1,1,1) or albedo
	uint64_t tex_diffuse; // albedo texture
	
	vec3 rma; // (1,1,1) or (roughness, metalness, 1)
	uint64_t tex_rma; // rma texture
	
	vec3 normal; // (1,1,1) or (0,0,1)
	uint64_t tex_normal; // bump texture
};

layout (std430, binding = 0) readonly buffer Materials {
	Material materials[];
};

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a    = roughness*roughness;
    float a2   = a*a;
    float NdH  = max(dot(N, H), 0.0);
    float NdH2 = NdH*NdH;
	
    float nom  = a2;
    float denom = (NdH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return nom / denom;
}

float GeometrySchlickGGX(float NdV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdV;
    float denom = NdV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdV = max(dot(N, V), 0.0);
    float NdL = max(dot(N, L), 0.0);
    float ggx1  = GeometrySchlickGGX(NdV, roughness);
    float ggx2  = GeometrySchlickGGX(NdL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}  

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}   

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

vec2 BRDFIntMap(float NdV, float alfa)
{
    if(brdf_map == 0)
        return vec2(1, 0);
    return texture(sampler2D(brdf_map), vec2(NdV, alfa)).rg;
}

vec3 PrefEnvMap(vec3 I, float alfa)
{
    if(pref_env_map == 0)
        return vec3(1);
    return textureLod(sampler2D(pref_env_map), SampleSphericalMap(I), alfa * pref_env_map_lvl).rgb;
}

vec3 IrradianceMap(vec3 N)
{
    if(ir_map == 0)
        return vec3(1);
    return texture(sampler2D(ir_map), SampleSphericalMap(N)).rgb;
}

float LinearizeDepth(float depth)
{
    float zNear = 0.8;    // TODO: Replace by the zNear of your perspective projection
    float zFar  = 1000.0; // TODO: Replace by the zFar  of your perspective projection
    return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

// ToDo
float ComputeOclusion(vec3 position, vec3 normal)
{
    const vec2 noiseScale = vec2(800.0/4.0, 600.0/4.0); 
    vec3 randomVec = texture(sampler2D(ssao_noise), tex*noiseScale).xyz;  
    vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 s_TBN       = mat3(tangent, bitangent, normal);  

    vec2 ssao_texel_size = 1.0 / textureSize(sampler2D(ssao_map), 0 );
    float occlusion = 0.0;
    vec2 radius = AORadius * ssao_texel_size;
    for(int i = 0; i < AOSamples; ++i)
    {
        // get sample position
        vec3 ssao_sample = s_TBN * ssao_samples[i]; // From tangent to view-space
        ssao_sample = gl_FragCoord.xyz + ssao_sample * vec3(radius, AORadius);
        float result = 0;
        for (float x = -AOPCF; x < AOPCF; ++x) 
        {
            for (float y = -AOPCF; y < AOPCF; ++y) 
            {

               vec4 offset = vec4(ssao_sample, 1.0);
                offset      = pv * offset;    // from view to clip-space
                offset.xyz /= offset.w;               // perspective divide
                offset.xyz  = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0  
                offset.xy += vec2(x, y) * radius;

                float sampleDepth = texture(sampler2D(ssao_map), offset.xy).r; 
                float rangeCheck = smoothstep(0.0, 1.0, AORadius / abs(gl_FragCoord.z - sampleDepth));
                result += ((sampleDepth >= ssao_sample.z + AOBias) ? 1.0 : 0.0) * rangeCheck;  
            }
        }
        occlusion += result / (2 * AOPCF * AOPCF);
    }  
    occlusion = 1.0 - (occlusion / (AOSamples));
    //occlusion = LinearizeDepth(texture(ssao_map, (TBN*gl_FragCoord.xyz).xy).r);
    return occlusion;
}

float ComputeShadows()
{
    vec2 shadow_texel_size = 1.0 / textureSize(sampler2D(shadow_map), 0 ); // size of a single texel in tex coords
    const int r = ShadowPCF; // search radius, try different values
    float shadow = 0.0; // cumulative shadowing coefficient
    for ( int y = -r; y <= r; ++y ) 
    {
        for ( int x = -r; x <= r; ++x ) 
        {
            // convert LCS's range <-1, 1> into TC's range <0, 1>
            vec2 a_tc = (position_lcs.xy + vec2(1.0)) * 0.5;
            a_tc += vec2(x, y) * shadow_texel_size;
            float depth = texture(sampler2D(shadow_map), a_tc).r;
            depth = depth * 2.0 - 1.0;
            shadow += (depth + ShadowBias >= position_lcs.z) ? 1.0 : ShadowAmmount;
        }
    }
    shadow *= (1.0 / ((2*r + 1)*(2*r + 1))); // compute mean shadowing value
    return shadow;
}

void main( void )
{
	// Get texture and material data
	Material material = materials[matIdx];

	vec3 diffuse;
	if(material.tex_diffuse > 0)
		diffuse = texture(sampler2D(material.tex_diffuse), tex).rgb;
	else
		diffuse = material.diffuse.rgb;

	// Resolve bump map
	vec3 N = norm;
	if(material.tex_normal > 0)
		N = normalize(TBN * normalize(texture(sampler2D(material.tex_normal), tex).rgb * 2.0 - 1.0));    
    // Fix oposite normal
    if(dot(N, eye) < 0)
        N = -N;

	// Resolve material params
	float ior = max(material.rma.b, 1.0002926);

	vec3 rma = vec3(material.rma.r, material.rma.g, 1);
	if(material.tex_rma > 0)
		rma = texture(sampler2D(material.tex_rma), tex).rgb;

	float roughness = rma.r;
	float metallicness = rma.g;
    float ao = rma.b;

    // Compute shadows
    float shadow = ComputeShadows();

    // Compute ambient oclusion
    if(material.tex_rma <= 0 && ssao_enabled)
        ao = ComputeOclusion(pos, N);

	// Prepare vectors
	vec3 L = normalize(light - pos);
    vec3 V = normalize(eye - pos);
    vec3 H = normalize(L + V);
    vec3 O = reflect(L, N);
    vec3 I = reflect(-V, N);

	// Prepare dotproducts
	float NdH = max(dot(N, H), 0.001);
    float HdV = max(dot(H, V), 0.0);
	float NdV = max(dot(N, V), 0.001);
    float NdL = max(dot(N, L), 0.0);
    float NdO = max(dot(N, O), 0.0);

	// Compute fresnel
	//vec3 F0 = mix(vec3(0.04), diffuse, metallicness);
    vec3 F0 = vec3((1 - ior) / (1 + ior));
    F0 = F0 * F0;
    vec3 F  = fresnelSchlick(HdV, F0);   
	
    // Reflectance equation
    // Calculate light radiance
    float distance    = length(light - pos) / LightDistanceDevider;
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance     = vec3(1) * attenuation;        
        
    // Cook-Torrance brdf
    float D = DistributionGGX(N, H, roughness);        
    float G   = GeometrySmith(N, V, L, roughness);          
        
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallicness;	  
        
    vec3 numerator    = D * G * F;
    //float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    float denominator = 4.0 * NdV * NdL;
    vec3 specular     = numerator / max(denominator, 0.001);  
            
    // Outgoing radiance Lo        
    vec3 Lo = (kD * diffuse / PI + specular) * radiance * NdL; 

    // IBL Lighting
    vec3 LD = diffuse * IrradianceMap(N);
    vec3 LS = PrefEnvMap(I, roughness);
    vec2 sb = BRDFIntMap(NdV, roughness);

//    kS = fresnelSchlick(NdV, F0);
//    kD = vec3(1.0) - kS;
//    kD *= 1.0 - metallicness;	  
        
    vec3 ambient = (kD * LD + (kS*sb.x + sb.y)*LS) * ao; 
    vec3 color = ambient + Lo;
    color *= shadow;
	//color = vec3(shadow,0,0)*LD;
    //color = vec3(LinearizeDepth(texture(shadow_map, (position_lcs.xy+vec2(1))*0.5).r));
    //color = vec3(LinearizeDepth(texture(sampler2D(shadow_map), gl_FragCoord.xy/800.0).r));
    //color = vec3(texture(sampler2D(ir_map), gl_FragCoord.xy/800.0).rgb);
    //color = vec3(LinearizeDepth(gl_FragCoord.z));

    // Gamma correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1);
}
