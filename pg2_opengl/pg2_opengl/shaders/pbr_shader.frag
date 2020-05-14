#version 450 core
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : require

#define PI 3.14159265359
#define LightDistanceDevider 500.0

uniform vec3 light;
uniform vec3 light_color;
uniform vec3 eye;

in vec3 pos;
in vec2 tex;
in vec3 norm;
in mat3 TBN;
flat in int matIdx;

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

vec3 Fresnel(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
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

	// Prepare vectors
    vec3 L = normalize(light - pos);
    vec3 V = normalize(eye - pos);
    vec3 H = normalize(L + V);
    vec3 I = reflect(-V, N);

	// Prepare dotproducts
	float NdH = max(dot(N, H), 0.001);
    float HdV = max(dot(H, V), 0.0);
	float NdV = max(dot(N, V), 0.001);
    float NdL = max(dot(N, L), 0.0);

	// Compute fresnel
    vec3 F0 = vec3((1 - ior) / (1 + ior));
    F0 = F0 * F0;
    vec3 F  = Fresnel(HdV, F0);   
	
    // Reflectance equation
    // Calculate light radiance
    float distance    = length(light - pos) / LightDistanceDevider;
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance     = light_color * attenuation;        
        
    // Cook-Torrance brdf
    float D = DistributionGGX(N, H, roughness);        
    float G   = GeometrySmith(N, V, L, roughness);          
        
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallicness;	  
        
    vec3 numerator    = D * G * F;
    float denominator = 4.0 * NdV * NdL;
    vec3 specular     = numerator / max(denominator, 0.001);  
            
    // Outgoing radiance Lo        
    vec3 Lo = (kD * diffuse / PI + specular) * radiance * NdL; 

    // IBL Lighting
    vec3 LD = diffuse * IrradianceMap(N);
    vec3 LS = PrefEnvMap(I, roughness);
    vec2 sb = BRDFIntMap(NdV, roughness);
            
    vec3 ambient = (kD * LD + (kS*sb.x + sb.y)*LS) * ao; 
    vec3 color = ambient + Lo;
	
    // Gamma correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2)); 
    
    FragColor = vec4(color, 1);
}
