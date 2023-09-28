/**************************************************************************************/
/*!
//    \file			TextureManager.h
//    \author(s) 	Euphrasia Theophelia Tan Ee Mun
//
//    \date   	    15th September 2023
//    \brief		This file contains the declarations of functions that are used to 
//  				create textures from a dds file and retrieve textures from a GUID
//					and a container for the textures.
//
//    \Percentage   Theophelia 100%
//
//    Copyright (C) 2022 DigiPen Institute of Technology.
//    Reproduction or disclosure of this file or its contents without the
//    prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /**************************************************************************************/

#pragma once

#include "../Core/SystemInterface.h"
#include "../gli-master/gli/gli.hpp"
#include "glslshader.h"

#define TextureManager Texture_Manager::Instance()

SINGLETON(Texture_Manager)
{
public:
	void Init();
	void Update(float dt);
	void Exit();

	void AddTexture(char const* Filename, std::string GUID);
	GLuint GetTexture(std::string GUID);
	GLuint CreateTexture(char const* Filename);
	GLuint CreateSkyboxTexture(char const* Filename);

private:

	std::unordered_map<std::string, std::pair<char const*, GLuint>> mTextureContainer; // GUID, <file name, GLuint>
	
};

