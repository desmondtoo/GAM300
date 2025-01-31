/*!***************************************************************************************
\file			DefaultShader.vert
\project
\author         Jake Lian, Euan Lim, Sean Ngo

\par			Course: GAM300
\date           05/11/2023

\brief
	Vertex Shader for PBR - Non Instanced

All content ? 2023 DigiPen Institute of Technology Singapore. All rights reserved.
******************************************************************************************/

#version 450 core

//-------------------------
//          COMING IN
//-------------------------

layout (location = 0) in vec3 aVertexPosition;
layout (location = 1) in vec3 aVertexNormal;
layout (location = 2) in vec3 aVertexTangent;
layout (location = 3) in vec2 aVertexTexCoord; //UVs info
layout (location = 4) in vec4 aVertexColor; // This can throw

layout(location = 5) in ivec4 boneIds; 
layout(location = 13) in vec4 weights;


//-------------------------
//          GOING OUT
//-------------------------

layout (location = 0) out vec2 TexCoords;
layout (location = 1) out vec3 WorldPos;
layout (location = 2) out vec3 Normal;
//layout (location = 3) out vec4 frag_pos_lightspace_D;
//layout (location = 4) out vec4 frag_pos_lightspace_S;
//
//-------------------------
//          UNIFORMS
//-------------------------


uniform mat4 persp_projection;
uniform mat4 View;
uniform mat4 SRT;

uniform mat4 lightSpaceMatrix_Directional;
uniform mat4 lightSpaceMatrix_Spot;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];
uniform bool isAnim;

//End

void main()
{
    TexCoords = aVertexTexCoord;

    vec4 totalPosition = vec4(0.0f);
    mat4 boneTransform = mat4(0.0f);
    bool breakOut = false;
    if (isAnim)
    {    
        for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
        {
            if(boneIds[i] == -1) 
                continue;
            if(boneIds[i] >=MAX_BONES) 
            {
                totalPosition = vec4(aVertexPosition, 1.0f);
                breakOut = true;
                break;
            }
        }

        if(!breakOut)
        {
            boneTransform = finalBonesMatrices[boneIds[0]] * weights[0];
            boneTransform += finalBonesMatrices[boneIds[1]] * weights[1];
            boneTransform += finalBonesMatrices[boneIds[2]] * weights[2];
            boneTransform += finalBonesMatrices[boneIds[3]] * weights[3];
            totalPosition = boneTransform * vec4(aVertexPosition, 1.0f);
        }

	    WorldPos = vec3(SRT * (totalPosition / totalPosition.w));
    }
    else
    {
	    WorldPos = vec3(SRT * vec4(aVertexPosition, 1.0f));
        totalPosition = vec4(aVertexPosition, 1.0f);
        breakOut = true;
    }
        
    Normal = mat3(transpose(inverse(SRT))) * aVertexNormal;
    if(!breakOut)
    {
        vec4 NormalL = boneTransform * vec4(aVertexNormal, 0.0);
//        Normal = mat3(transpose(inverse(SRT))) * vec3(NormalL);
        Normal = (SRT * NormalL).xyz;
    }

	gl_Position = persp_projection * View * SRT * totalPosition;

//    frag_pos_lightspace_D = lightSpaceMatrix_Directional * vec4(WorldPos, 1.0);
//
//    frag_pos_lightspace_S = lightSpaceMatrix_Spot * vec4(WorldPos, 1.0);
//
}