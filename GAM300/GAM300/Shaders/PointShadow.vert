/*!***************************************************************************************
\file			PointShadow.vert
\project
\author         Euan Lim

\par			Course: GAM300
\date           05/11/2023

\brief
	Vertex Shader for Point Shadows

All content ? 2023 DigiPen Institute of Technology Singapore. All rights reserved.
******************************************************************************************/

#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 5) in ivec4 boneIds; 
layout (location = 13) in vec4 weights;
layout (location = 6) in mat4 SRT;

//uniform mat4 model;

uniform bool isDefault = false;
uniform mat4 defaultSRT;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];
uniform bool isAnim;

//End

void main()
{
    if(!isDefault)
    {
        gl_Position = SRT * vec4(aPos, 1.0);
    }
    else
    {
        if (isAnim)
        {    
            vec4 totalPosition = vec4(0.0f);
            mat4 boneTransform = mat4(0.0f);
            bool breakOut = false;
            for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
            {
                if(boneIds[i] == -1) 
                    continue;
                if(boneIds[i] >=MAX_BONES) 
                {
                    totalPosition = vec4(aPos, 1.0f);
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
                totalPosition = boneTransform * vec4(aPos, 1.0f);
            }

	        gl_Position = defaultSRT * totalPosition;
        }
        else
	        gl_Position = defaultSRT * vec4(aPos, 1.0f);
    }
}