#version 450 core

// IN
layout (location = 0) in vec4 vColor;
layout (location = 1) in vec3 FragmentPos;
layout (location = 2) in vec3 Normal;

layout (location = 4) in vec2 Tex_Coord;


layout (location = 10) in vec4 frag_albedo;
layout (location = 11) in vec4 frag_specular;
layout (location = 12) in vec4 frag_diffuse;
layout (location = 13) in vec4 frag_ambient;
layout (location = 14) in float frag_shininess;

layout (location = 5) in vec3 TangentLightPos;
layout (location = 15) in vec3 TangentViewPos;
layout (location = 16) in vec3 TangentFragPos;



//    vec3 TangentLightPos;
//    vec3 TangentViewPos;
//    vec3 TangentFragPos;

//layout (location = 15) in VS_OUT {
//    vec3 TangentLightPos;
//    vec3 TangentViewPos;
//    vec3 TangentFragPos;
//} fs_in;

// OUT
out vec4 FragColor;


//uniform vec3 lightColor;
//uniform vec3 lightPos;
//uniform vec3 camPos;


layout(binding=0) uniform sampler2D myTextureSampler;

layout(binding=1) uniform sampler2D normalSampler;



void main()
{
    float gamma = 2.2;

    

    // COLOR CODE

//    if(lightColor == vec3(0.f,0.f,0.f))
//    {
//        FragColor = vec4(vColor); // set all 4 vector values to 1.0
//    }
//
//
//    // Initial   
//    //    FragColor = vec4(lightColor * objectColor, 1.0);
//
//    // ambient lighting
//    // float ambientStrength = 0.1;
//    vec3 ambience = vec3(frag_ambient) * lightColor;
//
////    vec3 result = ambient * objectColor;
////    FragColor = vec4(result, 1.0);
////
//    // diffuse 
//    vec3 norm = normalize(Normal);
//    vec3 lightDir = normalize(lightPos - FragmentPos);
//    float diff = max(dot(norm, lightDir), 0.0);
//    vec3 diffusion = lightColor * (diff * vec3(frag_diffuse));
//
////    vec3 result = (ambient + diffuse) * objectColor;
////    FragColor = vec4(result, 1.0);
////
//    // specular
//    vec3 viewDir = normalize(camPos - FragmentPos);
//    vec3 reflectDir = reflect(-lightDir, norm);  
//    float spec = pow(max(dot(viewDir, reflectDir), 0.0), frag_shininess); // look at basic lighting for the 32
//    vec3 speculation = lightColor * (vec3(frag_specular) * spec) ;  
//        
//    vec3 result = (ambience + diffusion + speculation) * vec3(vColor);
//    result = (ambience + diffusion + speculation) * vec3(frag_albedo);
//    FragColor = vec4(result, 1.0);


// NORMAL MAPPING
// 
     // obtain normal from normal map in range [0,1]
    vec3 normal = texture(normalSampler, Tex_Coord).rgb;
    
    // transform normal vector to range [-1,1]
    normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space
   
//    vec3 normal = normalize(Normal);


    // get diffuse color

    vec3 color = pow(texture(myTextureSampler, Tex_Coord).rgb, vec3(gamma));
//    vec3 color = texture(myTextureSampler, Tex_Coord).rgb;
    // ambient
    vec3 ambient = 0.1 * color;
    // diffuse
    vec3 lightDir = normalize(-TangentLightPos + TangentFragPos);
//    vec3 lightDir = normalize(TangentLightPos - TangentFragPos);
//    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular
    vec3 viewDir = normalize(TangentViewPos - TangentFragPos);
//    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2) * spec;
    FragColor = vec4(ambient + diffuse + specular, 1.0);





// FragColor = texture(myTextureSampler,Tex_Coord);





// Gamma Correction -> Last Step
    FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));

}