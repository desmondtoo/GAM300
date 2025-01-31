/*!***************************************************************************************
\file			ColourPicking.vert
\project
\author         Euan Lim

\par			Course: GAM300
\date           05/11/2023

\brief
	Vertex Shader for ColourPicking

All content � 2023 DigiPen Institute of Technology Singapore. All rights reserved.
******************************************************************************************/

#version 450 core


//-------------------------
//          COMING IN
//-------------------------

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

//-------------------------
//          GOING OUT
//-------------------------

out vec2 TexCoords;

//-------------------------
//          UNIFORMS
//-------------------------

uniform mat4 SRT;
uniform mat4 projection;
uniform mat4 view;

//End

void main()
{
    TexCoords = aTexCoords;

   gl_Position = projection * view * SRT * vec4(aPos, 1.0);
}