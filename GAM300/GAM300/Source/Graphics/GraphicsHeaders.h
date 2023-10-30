/*!***************************************************************************************
\file			GraphicsHeaders.h
\project
\author         Sean Ngo

\par			Course: GAM300
\date           10/10/2023

\brief
	This file contains the declaration of Graphics System that includes:
	1. 

All content � 2023 DigiPen Institute of Technology Singapore. All rights reserved.
******************************************************************************************/
#ifndef GRAPHICSHEADERS_H
#define GRAPHICSHEADERS_H

#include <glm/gtc/type_ptr.hpp>

#include "Core/SystemInterface.h"
#include "GraphicStructsAndClass.h"
#include "BaseCamera.h"

#include "glslshader.h"
#include <filesystem>

#include "Utilities/GUID.h"

#include "Model3d.h"
//#include "glslshader.h"
#include "GBuffer.h"

#define SHADER ShaderManager::Instance()
#define MYSKYBOX SkyboxManager::Instance()
#define DEBUGDRAW DebugDraw::Instance()
#define LIGHTING Lighting::Instance()
#define RENDERER Renderer::Instance()

class Ray3D;
class RaycastLine;
class SkyBox;

//// Map of all shader field types
//static std::unordered_map<std::string, size_t> shaderFieldTypeMap =
//{
//	{ "float",						GetFieldType::E<float>()},
//	{ "double",						GetFieldType::E<double>()},
//	{ "bool",						GetFieldType::E<bool>()},
//	{ "char",						GetFieldType::E<char>()},
//	{ "short",						GetFieldType::E<short>()},
//	{ "int",						GetFieldType::E<int>()},
//	{ "int64",						GetFieldType::E<int64_t>()},
//	{ "uint16_t",					GetFieldType::E<uint16_t>()},
//	{ "uint32_t",					GetFieldType::E<uint32_t>()},
//	{ "uint32_t",					GetFieldType::E<uint32_t>()},
//	{ "char*",						GetFieldType::E<char*>()},
//	{ "vec2",						GetFieldType::E<Vector2>()},
//	{ "vec3",						GetFieldType::E<Vector3>()},
//	{ "vec4",						GetFieldType::E<Vector4>()}
//};

// Graphics Settings

bool extern RenderShadow;
unsigned int extern bloomCount;
float extern bloomThreshold;
bool extern enableBloom;


// Graphic Functions
void renderQuad(unsigned int& _quadVAO, unsigned int& _quadVBO);
void renderQuadWireMesh(unsigned int& _quadVAO, unsigned int& _quadVBO);
bool bloom(unsigned int amount);

using InstanceContainer = std::unordered_map<GLuint, InstanceProperties>; // <vao, properties>
// Bean: A temp solution to access the shader
// enum SHADERTYPE
// {
// 	HDR,
// 	PBR,
// 	TIR,// Temporary Instance Render
// 	TDR,// Temporary Debug Instance Render
// 	SKYBOX,
// 	BASICLIGHT,
// 	AFFECTEDLIGHT,
// 	SHADOW,
// 	POINTSHADOW,
// 	UI_SCREEN,
// 	UI_WORLD,
// 	BLUR
// };

ENGINE_SYSTEM(ShaderManager)
{
public:
	void Init();
	void Update(float dt);
	void Exit();

	// All shaders will be loaded using this function and passed into shaders container
	void ShaderCompiler(const std::string & _name, const std::string& _vertPath, 
		const std::string& _fragPath, const std::string & _geometryPath = "");

	GLSLShader& GetShader(const SHADERTYPE& _type) { return shaders[static_cast<int>(_type)]; }

	void CreateShaderProperties(const std::string& _frag, const std::string& _vert);
	void ParseShaderFile(const std::string& _filename, bool _frag);

private:
	std::vector<GLSLShader> shaders;
	std::vector<ShaderProperties> shaderProperties;
};

SINGLETON(SkyboxManager)
{
public:
	void Init();

	// Initialize the skybox of the engine
	void CreateSkybox(const std::filesystem::path& _name);

	void Draw(BaseCamera& _camera);

private:
	SkyBox skyBoxModel;
	GLuint skyboxTex;
};

ENGINE_EDITOR_SYSTEM(DebugDraw)
{
public:
	void Init();
	void Update(float dt);
	void Exit();

	void SetupSegment3D();
	
	void Draw();

	void DrawSegment3D(const Segment3D& _segment3D, const glm::vec4& _color);
	void DrawSegment3D(const glm::vec3& _point1, const glm::vec3& _point2, const glm::vec4& _color);

	void DrawRay();

private:
	InstanceContainer* properties;
	std::vector<Ray3D> rayContainer;
	RaycastLine* raycastLine;
	bool enableRay = true;
};

ENGINE_SYSTEM(Lighting)
{
public:
	void Init();
	void Update(float dt);
	void Exit();

	LightProperties& GetLight() { return lightingSource; }

	std::vector<LightProperties>& GetPointLights() { return pointLightSources; }
	std::vector<LightProperties>& GetDirectionLights() { return directionLightSources; }
	std::vector<LightProperties>& GetSpotLights() { return spotLightSources; }

private:
	LightProperties lightingSource;
	std::vector<LightProperties> pointLightSources;
	std::vector<LightProperties> directionLightSources;
	std::vector<LightProperties> spotLightSources;
};

ENGINE_SYSTEM(Renderer)
{
public:
	void Init();
	void Update(float dt);
	void Exit();

	void SetupGrid(const int& _num);

	void Draw(BaseCamera& _camera);

	// Drawing UI onto screenspace
	void UIDraw_2D(BaseCamera& _camera);

	// Drawing UI onto worldspace
	void UIDraw_3D(BaseCamera& _camera);

	// Drawing Screenspace UI onto worldspace
	void UIDraw_2DWorldSpace(BaseCamera & _camera);


	void DrawMeshes(const GLuint& _vaoid, const unsigned int& _instanceCount,
		//const unsigned int& _primCount, GLenum _primType, const LightProperties& _lightSource, SHADERTYPE shaderType);
		const unsigned int& _primCount, GLenum _primType, const LightProperties& _lightSource, BaseCamera & _camera, SHADERTYPE shaderType);
	//glm::vec4 Albe, glm::vec4 Spec, glm::vec4 Diff, glm::vec4 Ambi, float Shin);
	//Materials Mat);

	void DrawGrid(const GLuint & _vaoid, const unsigned int& _instanceCount);

	void DrawDebug(const GLuint & _vaoid, const unsigned int& _instanceCount);

	void DrawDepth(LIGHT_TYPE temporary_test);

	bool Culling();

	void Forward();

	void Deferred();
	
	unsigned int ReturnTextureIdx(InstanceProperties& prop, const GLuint & _id);
	//unsigned int ReturnTextureIdx(const std::string & _meshName, const GLuint & _id);

	InstanceContainer& GetInstanceProperties() { return instanceProperties; }
	std::vector<InstanceContainer>& GetInstanceContainer() { return instanceContainers; }
	std::vector<DefaultRenderProperties>& GetDefaultProperties() { return defaultProperties; }

	float& GetExposure() { return exposure; }

	bool& IsHDR() { return hdr; }

	gBuffer m_gBuffer;
private:
	std::unordered_map<Engine::GUID, InstanceProperties> properties;
	InstanceContainer instanceProperties; // <vao, properties>
	std::vector<InstanceContainer> instanceContainers; // subscript represents shadertype
	//InstanceContainer instanceContainers[size_t(SHADERTYPE::COUNT)]; // subscript represents shadertype
	std::vector<DefaultRenderProperties> defaultProperties;
	float exposure = 1.f;
	bool hdr = true;
};
void renderQuad();
#endif // !GRAPHICSHEADERS_H