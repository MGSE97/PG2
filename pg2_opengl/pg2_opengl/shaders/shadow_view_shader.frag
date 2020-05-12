#version 450 core
#define ShadowBias 0.0001
#define ShadowAmmount 0.25
#define ShadowPCF 5

out vec4 FragColor;

in vec3 position_lcs;

uniform sampler2D shadow_map; // light's shadow map

float LinearizeDepth(float depth)
{
    float zNear = 0.8;    // TODO: Replace by the zNear of your perspective projection
    float zFar  = 1000.0; // TODO: Replace by the zFar  of your perspective projection
    return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

void main(void)
{    
    // From Camra
    float c = LinearizeDepth(gl_FragCoord.z);

    // From light
    // Compute shadows
    vec2 shadow_texel_size = 1.0 / textureSize(shadow_map, 0); // size of a single texel in tex coords
    const int r = ShadowPCF; // search radius, try different values
    float shadow = 0.0; // cumulative shadowing coefficient
    for ( int y = -r; y <= r; ++y ) {
        for ( int x = -r; x <= r; ++x ) {
            vec2 a_tc = (position_lcs.xy + vec2(1.0)) * 0.5;
            a_tc += vec2( x, y ) * shadow_texel_size;
            float depth = texture(shadow_map, a_tc).r;
            shadow += depth;
        }
    }
    shadow *= (1.0 / ((2*r + 1)*(2*r + 1))); // compute mean shadowing value
    c = LinearizeDepth(shadow);

    FragColor = vec4(c, c, c, 1.0);

}
