#include "Precompiled.h"
#include "GraphicsSystem.h"

#include "LapSupGraphics/Compiler.h"
#include "LapSupGraphics/Mesh.h"
#include "Model3d.h"
#include "Editor_Camera.h"
#include "../Core/FramerateController.h"

#include "Editor/Editor.h"
#include "Editor/EditorHeaders.h"
#include "Scene/SceneManager.h"
#include "Core/EventsManager.h"

#include "MeshManager.h"

//Temporary
//Model testmodel;

//Model LightSource;

Model AffectedByLight;
bool haveTexture = false;
Model Line;

std::map<std::string, InstanceProperties> properties;

bool SwappingColorSpace = false;
//Editor_Camera testCam;

//Editor_Camera E_Camera;
std::vector<Ray3D> Ray_Container;


// Naive Solution for now

std::vector <Materials> temp_MaterialContainer;

trans_mats SRT_Buffers[50];
GLSLShader temp_instance_shader;
LightProperties Lighting_Source;
//bool isThereLight = false;

//void InstanceSetup(GLuint vaoid);
//void InstancePropertySetup(InstanceProperties& prop);

void GraphicsSystem::Init()
{
	std::vector<std::pair<GLenum, std::string>> shdr_files;
	// Vertex Shader
	shdr_files.emplace_back(std::make_pair(
		GL_VERTEX_SHADER,
		"GAM300/Source/Graphics/InstancedRender.vert"));

	// Fragment Shader
	shdr_files.emplace_back(std::make_pair(
		GL_FRAGMENT_SHADER,
		"GAM300/Source/Graphics/InstancedRender.frag"));

	std::cout << "TEMP Instanced Render SHADER\n";
	temp_instance_shader.CompileLinkValidate(shdr_files);
	std::cout << "\n\n";

	// if linking failed
	if (GL_FALSE == temp_instance_shader.IsLinked()) {
		std::stringstream sstr;
		sstr << "Unable to compile/link/validate shader programs\n";
		sstr << temp_instance_shader.GetLog() << "\n";
		std::cout << sstr.str();
		std::exit(EXIT_FAILURE);
	}

	/*Scene& currentScene = SceneManager::Instance().GetCurrentScene();
	unsigned int textureCount = 0;
	bool skip = false;
	for (MeshRenderer& renderer : currentScene.GetComponentsArray<MeshRenderer>()) {
		Entity& entity = currentScene.GetEntity(renderer);
		std::string& textureName = currentScene.GetComponent<Texture>(entity).filepath;
		unsigned int texID = TextureManager.GetTexture(AssetManager::Instance().GetAssetGUID(textureName));
		for (int j = 0; j <= textureCount; ++j) {
			if (properties[renderer.MeshName].texture[j] == texID) {
				skip = true;
			}
		}
		if (skip) {
			skip = false;
			continue;
		}
		if (textureCount >= 31) {
			PRINT("TOO MANY TEXTURE IN THIS GEOM!!");
			break;
		}
		properties[renderer.MeshName].texture[textureCount++] = texID;
	}*/

	//std::cout << "-- Graphics Init -- " << std::endl;

	//INIT GRAPHICS HERE
	
	glEnable(GL_EXT_texture_sRGB); // Unsure if this is required	

	// Euan RayCasting Testing
	Line.lineinit();
	
	// Setting up Positions
	//testmodel.position = glm::vec3(0.f, 0.f, -800.f);
	//LightSource.position = glm::vec3(0.f, 0.f, -300.f);
	//testmodel.position = glm::vec3(0.f, 0.f, -800.f);
	//LightSource.position = glm::vec3(0.f, 0.f, -300.f);
	AffectedByLight.position = glm::vec3(0.f, 0.f, -500.f);

	int index = 0;

	EditorCam.Init();
}

void GraphicsSystem::Update(float dt)
{
	//std::cout << "-- Graphics Update -- " << std::endl;
	Scene& currentScene = SceneManager::Instance().GetCurrentScene();

	currentScene.singleComponentsArrays.GetArray<Transform>();
	
	Ray3D temp;
	bool checkForSelection = Raycasting(temp);
	
	float intersected = FLT_MAX;
	float temp_intersect;

	// Temporary Material thing
	//temp_MaterialContainer[3].Albedo = glm::vec4{ 1.f,1.f,1.f,1.f };
	temp_MaterialContainer[3].Diffuse = glm::vec4{ 1.0f, 0.5f, 0.31f,1.f };
	temp_MaterialContainer[3].Specular = glm::vec4{ 0.5f, 0.5f, 0.5f,1.f };
	temp_MaterialContainer[3].Ambient = glm::vec4{ 1.0f, 0.5f, 0.31f,1.f };
	temp_MaterialContainer[3].Shininess = 32.f;


	temp_MaterialContainer[3].Albedo.r = static_cast<float>(sin(glfwGetTime() * 2.0));
	temp_MaterialContainer[3].Albedo.g = static_cast<float>(sin(glfwGetTime() * 0.7));
	temp_MaterialContainer[3].Albedo.b = static_cast<float>(sin(glfwGetTime() * 1.3));


	// Temporary Light stuff
	bool haveLight = false;
	for (LightSource& lightSource : currentScene.GetComponentsArray<LightSource>())
	{
		haveLight = true;
		Entity& entity{ currentScene.GetEntity(lightSource) };
		Transform& transform = currentScene.GetComponent<Transform>(entity);

		Lighting_Source.lightpos = transform.translation;
		Lighting_Source.lightColor = lightSource.lightingColor;
	}
	if (!haveLight)
	{
		Lighting_Source.lightColor = glm::vec3(0.f, 0.f, 0.f);
	}

	// Update Loop
	int i = 0;


	for (MeshRenderer& renderer : currentScene.GetComponentsArray<MeshRenderer>())
	{
		Mesh* t_Mesh = MeshManager.DereferencingMesh(renderer.MeshName);
		
		if (t_Mesh == nullptr)
		{
			continue;
		}

		int index = t_Mesh->index;
		
		Entity& entity = currentScene.GetEntity(renderer);
		Transform& transform = currentScene.GetComponent<Transform>(entity);
		/*std::string& textureName = currentScene.GetComponent<Texture>(entity).filepath;
		unsigned int texID = TextureManager.GetTexture(AssetManager::Instance().GetAssetGUID(textureName));
		for (int j = 0; j <= properties[renderer.MeshName].textureCount; ++j) {
			if (properties[renderer.MeshName].texture[j] == texID) {
				haveTexture = true;
			}
		}
		if (!haveTexture) {
			if (properties[renderer.MeshName].textureCount < 32) {
				properties[renderer.MeshName].texture[properties[renderer.MeshName].textureCount++] = texID;
			}
		}
		haveTexture = false;*/
		properties[renderer.MeshName].entitySRT[properties[renderer.MeshName].iter++] = transform.GetWorldMatrix();
		char maxcount = 32;
		// newstring
		for (char namecount = 0; namecount < maxcount; ++namecount) {
			std::string newName = renderer.MeshName;
			
			newName += ('1' + namecount);
			
			if (properties.find(newName) == properties.end()) {
				break;
			}

			/*for (int j = 0; j <= properties[newName].textureCount; ++j) {
				if (properties[newName].texture[j] == texID) {
					haveTexture = true;
				}
			}
			if (!haveTexture) {
				if (properties[newName].textureCount < 32) {
					properties[newName].texture[properties[newName].textureCount++] = texID;
				}
			}*/

			properties[newName].entitySRT[properties[newName].iter++] = transform.GetWorldMatrix();
		}
		++i;

		// I am putting it here temporarily, maybe this should move to some editor area :MOUSE PICKING
		if (checkForSelection)
		{
			//glm::mat4 translation_mat(
			//	glm::vec4(1.f, 0.f, 0.f, 0.f),
			//	glm::vec4(0.f, 1.f, 0.f, 0.f),
			//	glm::vec4(0.f, 0.f, 1.f, 0.f),
			//	glm::vec4(transform.translation, 1.f)
			//);
			//glm::mat4 rotation_mat = glm::toMat4(glm::quat(transform.rotation));

			glm::mat4 transMatrix = transform.GetWorldMatrix();

			//glm::mat4 noscale = translation_mat * rotation_mat;

			glm::vec3 translation;
			glm::quat rot;
			glm::vec3 skew;
			glm::vec4 perspective;
			glm::vec3 scale;
			glm::decompose(transMatrix, scale, rot, translation, skew, perspective);

			glm::vec3 mins = scale * glm::vec3(-1.f, -1.f, -1.f);
			glm::vec3 maxs = scale * glm::vec3(1.f, 1.f, 1.f);
			glm::mat4 rotMat = glm::toMat4(rot);

			if (testRayOBB(temp.origin, temp.direction, mins, maxs,
				glm::translate(glm::mat4(1.0f), translation) * rotMat, temp_intersect))
			{
				if (temp_intersect < intersected)
				{
					//EDITOR.SetSelectedEntity(&entity);
					currentScene.GetHandle<Entity>(entity);
					SelectedEntityEvent SelectingEntity(currentScene.GetHandle(entity));

					EVENTS.Publish(&SelectingEntity);
					//EditorCam.ActiveObj = &entity;
					intersected = temp_intersect;
				}
			}
		}
	}


	// I am putting it here temporarily, maybe this should move to some editor area :MOUSE PICKING
	if (intersected == FLT_MAX && checkForSelection) 
	{// This means that u double clicked, wanted to select something, but THERE ISNT ANYTHING
		SelectedEntityEvent selectedEvent{ Handle<Entity>::Invalid()};
		EVENTS.Publish(&selectedEvent);
	}



	//Currently Putting in Camera Update loop here


	// Bean: For binding framebuffer
	EditorCam.getFramebuffer().bind();

	EditorCam.Update((float)MyFrameRateController.getDt());


	if (InputHandler::isKeyButtonPressed(GLFW_KEY_G))
	{
		SwappingColorSpace = !SwappingColorSpace;
		if (SwappingColorSpace)
		{
			glEnable(GL_FRAMEBUFFER_SRGB);
		}
		else
		{
			glDisable(GL_FRAMEBUFFER_SRGB);
		}
	}
	
	// Using Mesh Manager
	/*
	// instanced bind
	glBindBuffer(GL_ARRAY_BUFFER, MeshManager.mContainer.find("Cube")->second.SRT_Buffer_Index[0]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, (EntityRenderLimit) * sizeof(glm::mat4), &entitySRT[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	for (auto mesh = MeshManager.mContainer.begin(); mesh != MeshManager.mContainer.end(); mesh++)
	{

		// Looping through submeshes
		for (int k = 0; k < mesh->second.SRT_Buffer_Index.size(); ++k)
		{
			glBindBuffer(GL_ARRAY_BUFFER, mesh->second.SRT_Buffer_Index[k]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, (EntityRenderLimit) * sizeof(glm::mat4), &SRT_Buffers[mesh->second.index].transformation_mat[0]);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			Draw_Meshes(mesh->second.Vaoids[k], SRT_Buffers[mesh->second.index].index + 1, mesh->second.Drawcounts[k], mesh->second.prim,Lighting_Source);
		}
		SRT_Buffers[mesh->second.index].index = 0;
	}
	*/

	Draw(); // call draw after update


	// Bean: For unbinding framebuffer
	EditorCam.getFramebuffer().unbind();
	//glDisable(GL_FRAMEBUFFER_SRGB);
}

void GraphicsSystem::Draw_Meshes(GLuint vaoid, unsigned int instance_count, 
	unsigned int prim_count, GLenum prim_type, LightProperties LightSource, Materials Mat)
{
	
	
	

	//testBox.instanceDraw(EntityRenderLimit);

	// Should loop through the

	glEnable(GL_DEPTH_TEST); // might be sus to place this here

	temp_instance_shader.Use();
	// UNIFORM VARIABLES ----------------------------------------
	// Persp Projection
	GLint uniform1 =
		glGetUniformLocation(temp_instance_shader.GetHandle(), "persp_projection");
	GLint uniform2 =
		glGetUniformLocation(temp_instance_shader.GetHandle(), "View");
	GLint uniform3 =
		glGetUniformLocation(temp_instance_shader.GetHandle(), "lightColor");
	GLint uniform4 =
		glGetUniformLocation(temp_instance_shader.GetHandle(), "lightPos");
	GLint uniform5 =
		glGetUniformLocation(temp_instance_shader.GetHandle(), "camPos");


	// Material
	GLint uniform6 =
		glGetUniformLocation(temp_instance_shader.GetHandle(), "Albedos");
	GLint uniform7 =
		glGetUniformLocation(temp_instance_shader.GetHandle(), "Specular");
	GLint uniform8 =
		glGetUniformLocation(temp_instance_shader.GetHandle(), "Diffuse");
	GLint uniform9 =
		glGetUniformLocation(temp_instance_shader.GetHandle(), "Ambient");
	GLint uniform10 =
		glGetUniformLocation(temp_instance_shader.GetHandle(), "Shininess");




	// Scuffed SRT
	// srt not uniform
	/*GLint uniform3 =
		glGetUniformLocation(this->shader.GetHandle(), "SRT");*/

	glUniformMatrix4fv(uniform1, 1, GL_FALSE,
		glm::value_ptr(EditorCam.getPerspMatrix()));
	glUniformMatrix4fv(uniform2, 1, GL_FALSE,
		glm::value_ptr(EditorCam.getViewMatrix()));
	glUniform3fv(uniform3, 1,
		glm::value_ptr(LightSource.lightColor));
	glUniform3fv(uniform4, 1,
		glm::value_ptr(LightSource.lightpos));
	glUniform3fv(uniform5, 1,
		glm::value_ptr(EditorCam.GetCameraPosition()));

	// Material
	glUniform4fv(uniform6, 1,
		glm::value_ptr(Mat.Albedo));
	glUniform4fv(uniform7, 1,
		glm::value_ptr(Mat.Specular));
	glUniform4fv(uniform8, 1,
		glm::value_ptr(Mat.Diffuse));
	glUniform4fv(uniform9, 1,
		glm::value_ptr(Mat.Ambient));
	glUniform1f(uniform10,
		Mat.Shininess);





	glBindVertexArray(vaoid);
	glDrawElementsInstanced(GL_TRIANGLES, prim_count, GL_UNSIGNED_INT, 0, instance_count);
	glBindVertexArray(0);

	//glBindVertexArray(0);
//}
	temp_instance_shader.UnUse();
}


void GraphicsSystem::Draw() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.f, 0.5f, 0.5f, 1.f);
	glEnable(GL_DEPTH_BUFFER);

	// Looping Properties
	for (auto& [name, prop] : properties)
	{
		/*for (size_t i = 0; i < 32; i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, texIndex[i]);
		}*/
		glBindBuffer(GL_ARRAY_BUFFER, prop.entitySRTbuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, (EntityRenderLimit) * sizeof(glm::mat4), &(prop.entitySRT[0]));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		Draw_Meshes(prop.VAO, prop.iter, prop.drawCount, GL_TRIANGLES, Lighting_Source, temp_MaterialContainer[3]);
		prop.iter = 0;
	}


	// This is to render the Rays
	if (Ray_Container.size() > 0)
	{

		for (int i = 0; i < Ray_Container.size(); ++i)
		{
			Ray3D ray = Ray_Container[i];

			//std::cout << "ray " << ray.origin.x << "\n";
			//std::cout << "ray direc" << ray.direction.x << "\n";

			glm::mat4 SRT
			{
				glm::vec4(ray.direction.x * 1000000.f, 0.f , 0.f , 0.f),
				glm::vec4(0.f, ray.direction.y * 1000000.f, 0.f , 0.f),
				glm::vec4(0.f , 0.f , ray.direction.z * 1000000.f , 0.f),
				glm::vec4(ray.origin.x, ray.origin.y, ray.origin.z,1.f)
			};
			//std::cout << "in here draw\n";
			Line.debugline_draw(SRT);

		}
	}
}

bool GraphicsSystem::Raycasting(Ray3D& _ray)
{
	// I am putting it here temporarily, maybe this should move to some editor area :MOUSE PICKING

	if (!EditorScene::Instance().UsingGizmos() && !EditorCam.isMoving && InputHandler::isMouseButtonPressed_L())
	{
		// Bean: Click within the scene imgui window
		if (!EditorScene::Instance().WindowHovered())
			return false;

		_ray = EditorCam.Raycasting(EditorCam.GetMouseInNDC().x, EditorCam.GetMouseInNDC().y,
			EditorCam.getPerspMatrix(), EditorCam.getViewMatrix(), EditorCam.GetCameraPosition());
		Ray_Container.push_back(_ray);
		return true;
	}

	return false;
}

void GraphicsSystem::Exit()
{
	//std::cout << "-- Graphics Exit -- " << std::endl;

	//CLEANUP GRAPHICS HERE
}