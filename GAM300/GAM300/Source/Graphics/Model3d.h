/*!***************************************************************************************
\file			Model3d.h
\project
\author         Lian Khai Kiat, Euan Lim, Theophelia Tan

\par			Course: GAM300
\date           28/09/2023

\brief
	This file contains the declaration of 
	1. temporary stucts and classes used for testing
	2. Model class to create a model by abstacting openGL functions

All content � 2023 DigiPen Institute of Technology Singapore. All rights reserved.
******************************************************************************************/
#pragma once

#include "glslshader.h"
#include "../../glfw-3.3.8.bin.WIN64/include/GLFW/glfw3.h"

// Bean: Should not be here and instead be in renderer system
#include "Graphics/BaseCamera.h"

//tempory model
struct GeneralModel {

	// this is a problem
	// coz got different number of instance
	// then the array all different...
	// maybe need sort and align models with the AOS?
	// tmr then think...

	int instance_count = 0; // the number of instance using this geom
	GLuint fbx_VAO;
	GLuint fbx_VBO;
	std::vector<GLuint> tex_VAO;
	std::vector<unsigned int> texure_index; // 
	void addtexure(); // adds all the texture that teh geom needs

};


// need restructure this whole entire thing
// separate instanced stuff
// maybe even seperate from the default cube.. think about this tmr khai kiat
class Model {
public:

	// This 3D Model version
	//void init(AssimpLoader* geom); // make vao vbo

	// display the models created
	void draw();

	// This below are all the things i use to testing with light stuffs
	// initialize cube model
	void cubeinit();

	// initialize cube model for instance rendering
	void instance_cubeinit();

	// display light source
	void lightSource_draw();

	// draw model that is affected by light (temporary)
	void affectedByLight_draw(glm::vec3 lightPos);

	// draw using instance rendering
	void instanceDraw(int entitycount);

	glm::vec3 position;
	
	GLenum prim;
	GLuint vaoid;
	GLuint vboid;
	GLuint texturebuffer{};
	GLuint drawcount;
	GLSLShader shader;

	std::vector<GLuint> FBX_vaoid;
	std::vector<GLuint> FBX_vboid;
	std::vector<GLuint> FBX_drawcount;

	std::vector<GeneralModel> _mGeneral_model;

	// debug AABB
	GLuint vaoidAABB;
	GLSLShader shaderAABB;
	glm::vec3 pntAABB[8];
	std::vector<glm::ivec2> idxAABB{};

	glm::vec3 vertices_min{};
	glm::vec3 vertices_max{};

	// initialize vao for debug lines
	void debugAABB_init(); // vao & shader

	// draw debug lines
	void debugAABB_draw(glm::mat4& SRT);

	//AssimpLoader* _geom;


	//AssimpLoader* _geom; 
};

class SkyBox: public Model
{
public:
	// initialize skybox vao/vbo
	void SkyBoxinit();

	// draw sky box
	void SkyBoxDraw(GLuint skyboxtex, BaseCamera& _camera);
};

class RaycastLine: public Model
{
public:
	// initializer lines
	void lineinit();

	// draw debug lines
	void debugline_draw(glm::mat4& SRT);
};
