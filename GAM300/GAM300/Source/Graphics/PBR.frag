#version 450 core

//-------------------------
//          COMING IN
//-------------------------
layout (location = 0) in vec2 TexCoords;
layout (location = 1) in vec3 WorldPos;
layout (location = 2) in vec3 Normal;

layout (location = 3) in vec4 frag_Albedo;
layout (location = 4) in vec3 frag_Metal_Rough_AO_index;
layout (location = 5) in vec3 frag_Metal_Rough_AO_constant;
layout (location = 6) in vec2 frag_texture_index;

//-------------------------
//          GOING OUT
//-------------------------

out vec4 FragColor;

//-------------------------
//          UNIFORMS
//-------------------------

layout (binding = 0) uniform sampler2D myTextureSampler[32];

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 camPos;



//// material parameters
//uniform sampler2D albedoMap;
//uniform sampler2D normalMap;
//uniform sampler2D metallicMap;
//uniform sampler2D roughnessMap;
//uniform sampler2D aoMap;
//
//// lights
//uniform vec3 lightPositions[4];
//uniform vec3 lightColors[4];
//uniform vec3 camPos;



const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal 
// mapping the usual way for performance anyways; I do plan make a note of this 
// technique somewhere later in the normal mapping tutorial.
vec3 getNormalFromMap(int NM_index)
{
    vec3 tangentNormal = texture(myTextureSampler[NM_index], TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------





void main()
{		

    int Tex_index = int(frag_texture_index.x + 0.5f); // .x is texture
    int NM_index = int(frag_texture_index.y + 0.5f);    // .y is normal map

    int Metallic_index = int(frag_Metal_Rough_AO_index.x + 0.5f); // .x is texture
    int Roughness_index = int(frag_Metal_Rough_AO_index.y + 0.5f);    // .y is normal map
    int AO_index = int(frag_Metal_Rough_AO_index.z + 0.5f);    // .y is normal map


    vec3 albedo;
    float metallic;
    float roughness;
    float ao;


    if (Tex_index < 32)
    {
        albedo = pow(texture(myTextureSampler[Tex_index], TexCoords).rgb, vec3(2.2));
    }
    else
    {
        albedo = vec3(frag_Albedo);
    }

    if (Metallic_index < 32)
    {
        metallic  = texture(myTextureSampler[Metallic_index], TexCoords).r;    
    }
    else
    {
        metallic = frag_Metal_Rough_AO_constant.x;
    }
    if (Roughness_index < 32)
    {
        roughness  = texture(myTextureSampler[Roughness_index], TexCoords).r;    
    }
    else
    {
        roughness = frag_Metal_Rough_AO_constant.y;
    }
    if (AO_index < 32)
    {
        ao  = texture(myTextureSampler[AO_index], TexCoords).r;    
    }
    else
    {
        ao = frag_Metal_Rough_AO_constant.z;
    }

    vec3 N = getNormalFromMap(NM_index);
    vec3 V = normalize(camPos - WorldPos);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i) 
    {
        // calculate per-light radiance
//        vec3 L = normalize(lightPositions[i] - WorldPos);
        vec3 L = normalize(lightPos - WorldPos);
        vec3 H = normalize(V + L);
//        float distance = length(lightPositions[i] - WorldPos);
        float distance = length(lightPos - WorldPos);
        float attenuation = 1.0 / (distance * distance);
//        vec3 radiance = lightColors[i] * attenuation;
        vec3 radiance = lightPos * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
           
        vec3 numerator    = NDF * G * F; 
//        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        float denominator = 1.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;
        
        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;	  

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }   
    

    vec3 ambient = vec3(0.03) * albedo * ao;
    
    vec3 color = ambient + Lo;

    // Done in Post Processing
//    // HDR tonemapping
//    color = color / (color + vec3(1.0));
//    // gamma correct
//    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);
}