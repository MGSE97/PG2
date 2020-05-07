#version 450 core
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : require

#define PI 3.14159265359
#define LightDistanceDevider 1000.0
#define ShadowBias 0.0001
#define ShadowAmmount 0.25

uniform vec3 light;
uniform vec3 eye;

in vec3 pos;
in vec2 tex;
in vec3 norm;
in mat3 TBN;
flat in int matIdx;
in vec3 position_lcs;
uniform sampler2D shadow_map; // light's shadow map

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
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
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
	float ior = material.rma.b;

	vec3 rma = vec3(material.rma.r, material.rma.g, 1);
	if(material.tex_rma > 0)
		rma = texture(sampler2D(material.tex_rma), tex).rgb;

	float roughness = material.rma.r;
	float metallicness = material.rma.g;

    // Compute shadows
    vec2 shadow_texel_size = 1.0f / textureSize( shadow_map, 0 ); // size of a single texel in tex coords
    const int r = 2; // search radius, try different values
    float shadow = 0.0; // cumulative shadowing coefficient
    for ( int y = -r; y <= r; ++y ) 
    {
        for ( int x = -r; x <= r; ++x ) 
        {
            // convert LCS's range <-1, 1> into TC's range <0, 1>
            vec2 a_tc = (position_lcs.xy + vec2(1.0)) * 0.5;
            a_tc += vec2(x, y) * shadow_texel_size;
            float depth = texture(shadow_map, a_tc).r;
            depth = depth * 2.0 - 1.0;
            shadow += (depth + ShadowBias >= position_lcs.z) ? 1.0 : ShadowAmmount;
        }
    }
    shadow *= (1.0 / ((2*r + 1)*(2*r + 1))); // compute mean shadowing value

    // convert LCS's range <-1, 1> into TC's range <0, 1>
//    vec2 a_tc = (position_lcs.xy + vec2(1.0)) * 0.5;
//    float depth = texture(shadow_map, a_tc).r;
//    // (pseudo)depth was rescaled from <-1, 1> to <0, 1>
//    depth = depth * 2.0 - 1.0; // we need to do the inverse
//    float shadow = (depth + ShadowBias >= position_lcs.z) ? 1.0 : ShadowAmmount; // 0.25f represents the amount of shadowing

	// Prepare vectors
	//vec3 L = normalize(light - pos);
    //vec3 V = normalize(eye - pos);
    vec3 L = normalize(light);
    vec3 V = normalize(eye);
    vec3 H = normalize(L + V);

	// Prepare dotproducts
	float NdH = max(dot(N, H), 0.001);
    float NdL = max(dot(N, L), 0.0);

	// Compute fresnel
	vec3 F0 = mix(vec3(0.04), diffuse, metallicness);
	
    // Calculate light radiance
    float distance    = length(light - pos) / LightDistanceDevider;
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance     = vec3(1) * attenuation;        
        
    // Cook-Torrance brdf
    float NDF = DistributionGGX(N, H, roughness);        
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
        
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallicness;	  
        
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    vec3 specular     = numerator / max(denominator, 0.001);  
            
    // Outgoing radiance Lo        
    vec3 Lo = (kD * diffuse / PI + specular) * radiance * NdL * shadow; 

    vec3 ambient = vec3(0.03) * diffuse;
    vec3 color = ambient + Lo;
	
    // Gamma correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1);
    //FragColor = vec4(dot(N, light - pos), 0, 0, 1);
	//FragColor = vec4(diffuse * max(dot(N, light), 0), 1);
}
