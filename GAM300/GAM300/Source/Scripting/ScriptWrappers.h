﻿/*!***************************************************************************************
\file			ScriptWrappers.h
\project
\author			Zacharie Hong

\par			Course: GAM300
\par			Section:
\date			10/09/2023

\brief
	This file helps register static functions be used as internal calls in C#

All content © 2023 DigiPen Institute of Technology Singapore. All rights reserved.
*****************************************************************************************/


#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"
#include "IOManager/InputHandler.h"
#include "IOManager/InputSystem.h"
#include "Scene/SceneManager.h"
#include "ScriptingSystem.h"
#include "Scene/Identifiers.h"
#include "Audio/AudioManager.h"
#include "Graphics/Animation/BaseAnimator.h"
#include "Graphics/BaseCamera.h"
#include "Graphics/Ray3D.h"
#include "AI/NavMesh.h"
#include "AI/NavMeshBuilder.h"
#include "Physics/PhysicsSystem.h"
#include "Graphics/GraphicsHeaders.h"

#ifndef SCRIPT_WRAPPERS_H
#define SCRIPT_WRAPPERS_H

	static std::unordered_map<MonoType*, size_t> monoComponentToType;

	#define Register(METHOD) mono_add_internal_call("BeanFactory.InternalCalls::"#METHOD,METHOD)

	//Gets object that entity has
	static void Get(ScriptObject<Object> pEntity, MonoReflectionType* componentType, ScriptObject<Object>& obj)
	{
		Object* entityMaybe = pEntity;
		MonoType* mType = mono_reflection_type_get_type(componentType);
		auto pair = monoComponentToType.find(mType);
		if (pair == monoComponentToType.end())
		{
			if (SCRIPTING.IsScript(mono_class_from_mono_type(mType)))
			{
				Scene& scene = MySceneManager.GetCurrentScene();
				for (Script* pScript : scene.GetMulti<Script>((Object*)pEntity))
				{
					ScriptClass& scriptClass = SCRIPTING.GetScriptClass(pScript->scriptId);
					if (mono_class_get_type(scriptClass.mClass) == mType)
					{
						obj = ScriptObject<Script>(pScript);
						return;
					}
				}
				//Script
				obj = nullptr;
				return;
			}
			obj = nullptr;
			return;
		}
		Object* pObject = MySceneManager.GetCurrentScene().Get(pair->second, (Object*)pEntity);
		obj = pObject;
	}

	static MonoString* GetTag(ScriptObject<Object> pObject)
	{
		Object* object = pObject;
		Tag& tag = MySceneManager.GetCurrentScene().Get<Tag>(object->EUID());
		return SCRIPTING.CreateMonoString(IDENTIFIERS.GetTagString(tag.tagName));
	}

#pragma region INPUT SYSTEM
	//DOESNT WORK YET, Checks if key was released
	static bool GetKeyUp(int keyCode)
	{
		return InputHandler::isKeyButtonPressed(keyCode);
	}

	//Checks if key was pressed
	static bool GetKeyDown(int keyCode)
	{
		return InputHandler::isKeyButtonPressed(keyCode);
	}

	static int GetScrollState()
	{
		return InputHandler::getMouseScrollState();
	}

	static bool GetMouseDown(int mouseCode)
	{
		UNREFERENCED_PARAMETER(mouseCode);
		return InputHandler::isMouseButtonPressed_L();
	}

	//Checks if key is held
	static bool GetKey(int keyCode)
	{
		return InputHandler::isKeyButtonHolding(keyCode);
	}

	static void GetMouseDelta(Vector2& mouseDelta)
	{
		mouseDelta = InputHandler::mouseDeltaNormalized();
	}

	static void LockCursor(bool toggle)
	{
		InputSystem::Instance().LockCursor(toggle);
	}
#pragma endregion

#pragma region PHYSICS SYSTEM

	static EngineRayCastResult Raycast(Vector3& position, Vector3& direction, float& distance)
	{
		// jDirection MUST be normalized
		JPH::RVec3 jPosition = JPH::RVec3::sZero();
		JPH::RVec3 jDirection = JPH::RVec3::sZero();

		jPosition.SetX(position.x);
		jPosition.SetY(position.y);
		jPosition.SetZ(position.z);

		jDirection.SetX(direction.x);
		jDirection.SetY(direction.y);
		jDirection.SetZ(direction.z);

		EngineRayCastResult ercr = PHYSICS.CastRay(jPosition, jDirection, distance);
		return ercr;
	}

#pragma endregion

#pragma region AUDIO
	static void AudioSourcePlay(ScriptObject<AudioSource> audioSource)
	{
		AudioSource& audio = audioSource;
		AUDIOMANAGER.PlayComponent(audioSource);
	}

	static void SetMasterVolume(bool toggle) { // toggle for now, change to float when ready
		AUDIOMANAGER.SetMasterVolume(float(toggle));
	}

	static void StopMusic(float fade = 1.f) {
		AUDIOMANAGER.StopMusic(fade);
	}

	static void PauseMusic() {
		AUDIOMANAGER.SetMusicVolume(0.f);
	}

	static void ResumeMusic() {
		AUDIOMANAGER.PlayMusic();
	}

	static void SetMusicFade(ScriptObject<AudioSource> audioSource, float fadeOut, float fadeIn) {
		AUDIOMANAGER.SetMusicFade(audioSource, fadeOut, fadeIn);
	}

	static void EnableSFX(bool toggle) {
		AUDIOMANAGER.EnableSFX(toggle);
		if (!toggle) {
			AUDIOMANAGER.PauseLoopFX();
		}
	}
	static void PauseComponent(ScriptObject<AudioSource> audioSource) {
		AUDIOMANAGER.PauseComponent(audioSource);
	}
#pragma endregion

#pragma region ANIMATOR
	static void PlayAnimation(ScriptObject<Animator> pAnimator)
	{
		Animator& animator = pAnimator;
		animator.ChangeState();
	}

	static void PauseAnimation(ScriptObject<Animator> pAnimator)
	{
		Animator& animator = pAnimator;
		animator.playing = false;
	}

	static void StopAnimation(ScriptObject<Animator> pAnimator)
	{
		Animator& animator = pAnimator;
		animator.m_CurrentTime = 0.f;
		animator.playing = false;
	}

	static float GetProgress(ScriptObject<Animator> pAnimator)
	{
		Animator& animator = pAnimator;
		return animator.GetProgress();
	}

	static void SetProgress(ScriptObject<Animator> pAnimator, float value)
	{
		Animator& animator = pAnimator;
		animator.SetProgress(value);
	}

	static float GetSpeed(ScriptObject<Animator> pAnimator)
	{
		Animator& animator = pAnimator;
		return animator.GetSpeed();
	}

	static void SetSpeed(ScriptObject<Animator> pAnimator, float value)
	{
		Animator& animator = pAnimator;
		animator.SetSpeed(value);
	}

	static void SetDefaultState(ScriptObject<Animator> pAnimator, MonoString* mString)
	{
		Animator& animator = pAnimator;
		animator.SetDefaultState(mono_string_to_utf8(mString));
	}

	static void SetState(ScriptObject<Animator> pAnimator, MonoString* mString)
	{
		Animator& animator = pAnimator;
		animator.SetState(mono_string_to_utf8(mString));
	}

	static void SetNextState(ScriptObject<Animator> pAnimator, MonoString* mString)
	{
		Animator& animator = pAnimator;
		animator.SetNextState(mono_string_to_utf8(mString));
	}

	static MonoString* GetState(ScriptObject<Animator> pAnimator)
	{
		Animator& animator = pAnimator;
		return SCRIPTING.CreateMonoString(animator.GetCurrentState()->label);
	}

#pragma endregion
	
#pragma region TRANSFORM
	static void SetTransformParent(ScriptObject<Transform> pTransform, ScriptObject<Transform> pParent)
	{
		Transform& transform = pTransform;
		Transform& parent = pParent;
		Object* obj = pParent;

		// If the parent doesnt exist, set the parent of this transform to null
		if (obj)
			transform.SetParent(&parent);
		else
			transform.SetParent(nullptr);
	}

	static void GetWorldPosition(ScriptObject<Transform> pTransform, Vector3& position)
	{
		Transform& t = pTransform;
		position = t.GetGlobalTranslation();
	}

	static void GetWorldRotation(ScriptObject<Transform> pTransform, Vector3& rotation)
	{
		Transform& t = pTransform;
		rotation = t.GetGlobalRotation();
	}

	static void GetWorldScale(ScriptObject<Transform> pTransform, Vector3& scale)
	{
		Transform& t = pTransform;
		scale = t.GetGlobalScale();
	}

	static void GetLocalPosition(ScriptObject<Transform> pTransform, Vector3& position)
	{
		Transform& t = pTransform;
		position = t.GetLocalTranslation();
	}

	static void GetLocalRotation(ScriptObject<Transform> pTransform, Vector3& rotation)
	{
		Transform& t = pTransform;
		rotation = t.GetLocalRotation();
	}

	static void GetLocalScale(ScriptObject<Transform> pTransform, Vector3& scale)
	{
		Transform& t = pTransform;
		scale = t.GetLocalScale();
	}

	static void SetWorldPosition(ScriptObject<Transform> pTransform, Vector3& position)
	{
		Transform& t = pTransform;
		t.SetGlobalPosition(position);
	}

	static void SetWorldRotation(ScriptObject<Transform> pTransform, Vector3& rotation)
	{
		Transform& t = pTransform;
		t.SetGlobalRotation(rotation);
	}

	static void SetWorldScale(ScriptObject<Transform> pTransform, Vector3& scale)
	{
		Transform& t = pTransform;
		t.SetGlobalScale(scale);
	}

	static void SetLocalPosition(ScriptObject<Transform> pTransform, Vector3& position)
	{
		Transform& t = pTransform;
		t.SetLocalPosition(position);
	}

	static void SetLocalRotation(ScriptObject<Transform> pTransform, Vector3& rotation)
	{
		Transform& t = pTransform;
		t.SetLocalRotation(rotation);
	}

	static void SetLocalScale(ScriptObject<Transform> pTransform, Vector3& scale)
	{
		Transform& t = pTransform;
		t.SetLocalScale(scale);
	}
#pragma endregion

#pragma region PARTICLES

	static void ParticlesPlayer(ScriptObject<ParticleComponent> particleComp)
	{
		ParticleComponent& p = particleComp;
		p.particleLooping = (p.particleLooping ? false : true);
	}

#pragma endregion

#pragma region MESH_RENDERER

	static void GetMaterial(ScriptObject<MeshRenderer> meshRenderer, ScriptObject<Material_instance>  mat)
	{
		MeshRenderer& mr = meshRenderer;
		Material_instance& matInstance = mat;
		matInstance = MATERIALSYSTEM.getMaterialInstance(mr.materialGUID);
	}

	static void SetMaterial(ScriptObject<MeshRenderer> meshRenderer, ScriptObject<Material_instance> mat)
	{
		MeshRenderer& mr = meshRenderer;
		Material_instance& matInstance = MATERIALSYSTEM.getMaterialInstance(mr.materialGUID);
		Material_instance& newInstance = mat;
		if (!matInstance.isVariant)
		{
			mr.materialGUID = MATERIALSYSTEM.InstantiateRuntimeMaterial(mat);
		}
		else
		{
			matInstance = mat;
		}
	}

#pragma endregion

#pragma region CAMERA

	static void SetCameraTarget(ScriptObject<Camera> pCamera, Vector3& position)
	{
		Camera& camera = pCamera;
		camera.SetFocalPoint(position);
	}

	static void GetRightVec(ScriptObject<Camera> pCamera, Vector3& vector)
	{
		Camera& camera = pCamera;
		vector = camera.GetRightVec();
	}

	static void GetUpVec(ScriptObject<Camera> pCamera, Vector3& vector)
	{
		Camera& camera = pCamera;
		vector = camera.GetUpVec();
	}

	static void GetForwardVec(ScriptObject<Camera> pCamera, Vector3& vector)
	{
		Camera& camera = pCamera;
		vector = camera.GetForwardVec();
	}

#pragma endregion

#pragma region SPRITERENDERER
	static bool IsButtonClicked(ScriptObject<SpriteRenderer> spriteRenderer)
	{
		SpriteRenderer& sr = spriteRenderer;
		return sr.onClick;
	}


#pragma endregion

	// Load a scene
	static void LoadScene(MonoString* mString, bool loadDirect)
	{
		// Bean: Not really elegant because we can only load scenes from the scene folder
		// Zach: kek
		std::string scenePath = "Assets/Scene/";
		scenePath += mono_string_to_utf8(mString);
		scenePath += ".scene";

		MySceneManager.sceneToLoad = scenePath;
		if (loadDirect)
			MySceneManager.LoadNext();
	}

	static void LoadNext()
	{
		MySceneManager.LoadNext();
	}

	//Gets object that entity has
	static void* AddComponent(ScriptObject<Object> pEntity, MonoReflectionType* componentType)
	{
		MonoType* mType = mono_reflection_type_get_type(componentType);
		auto pair = monoComponentToType.find(mType);

		if (pair == monoComponentToType.end())
		{
			if (SCRIPTING.IsScript(mono_class_from_mono_type(mType)))
			{
				//Script
				std::string scriptName = mono_type_get_name(mType);
				size_t offset = scriptName.find_last_of(".");
				if (offset != std::string::npos)
					scriptName = scriptName.substr(offset + 1);
				//Get Mono Script instead
				return MySceneManager.GetCurrentScene().Add<Script>((Entity&)pEntity,nullptr,scriptName.c_str());
			}
			else
			{
				//Cant find
				//CONSOLE_ERROR(mono_type_get_name(mType), "is not a valid component!");
				return nullptr;
			}
		}
		return ScriptObject<Object>((Object*)MySceneManager.GetCurrentScene().Add(pair->second, pEntity));
	}

	//Checks if entity has a component
	static void HasComponent(ScriptObject<Entity> pEntity, MonoReflectionType* componentType, bool& output)
	{
		MonoType* managedType = mono_reflection_type_get_type(componentType);
		Object* entity(pEntity);
		if (!entity)
		{
			PRINT("Has component when gameobject is null!\n");
			output = false;
			return;
		}
		if (monoComponentToType.find(managedType) != monoComponentToType.end())
		{
			bool hasComp = ((Entity&)pEntity).hasComponentsBitset.test(monoComponentToType[managedType]);

			output = hasComp;
			return;
		}			
		else if (SCRIPTING.IsScript(mono_class_from_mono_type(managedType)))
		{
			Scene& scene = MySceneManager.GetCurrentScene();
			for (Script* pScript : scene.GetMulti<Script>((Object*)pEntity))
			{
				ScriptClass& scriptClass = SCRIPTING.GetScriptClass(pScript->scriptId);
				if (mono_class_get_type(scriptClass.mClass) == managedType)
				{
					output = true;
					return;
				}
			}
			//Script
			output = false;
			return;
		}
		PRINT(mono_type_get_name(managedType), "is invalid", '\n');
		output = false;
	}

	static void CloneGameObject(ScriptObject<Entity> pEntity, ScriptObject<Entity>& out)
	{
		Object* obj{ pEntity };
		out = &MySceneManager.GetCurrentScene().Clone((Entity&)pEntity);
	}

	//Deletes a gameobject
	static void DestroyGameObject(ScriptObject<Entity> pEntity)
	{
		MySceneManager.GetCurrentScene().Destroy<Entity>(pEntity);
	}

	//GENERIC_RECURSIVE(void, DestroyRecursive, MySceneManager.GetCurrentScene().Destroy(*(T*)pObject))
	//static void DestroyComponent(void* pComponent, MonoReflectionType* componentType)
	//{
	//	MonoType* managedType = mono_reflection_type_get_type(componentType);
	//	DestroyRecursive(monoComponentToType[managedType],pComponent);
	//}

	static MonoString* GetLayerName(int layer)
	{
		E_ASSERT(layer < MAX_PHYSICS_LAYERS, "Exceeded max physics layers");
		std::string& name = IDENTIFIERS.physicsLayers[layer].name;
		if (name.size() == 0)
		{
			CONSOLE_WARN("Physics Layer name is unassigned and is being used");
		}
		return SCRIPTING.CreateMonoString(name);
	}

	static int GetLayer(MonoString* mString)
	{
		std::string name = mono_string_to_utf8(mString);
		int i = 0;
		for (Layer& layer  : IDENTIFIERS.physicsLayers)
		{
			if (name == layer.name)
			{
				return i;
			}
			++i;
		}
		CONSOLE_WARN("Physics Layer ", name ,"does not exist");
		return -1;
	}

	static bool GetActive(ScriptObject<Object> object, MonoReflectionType* componentType)
	{
		MonoType* mType = mono_reflection_type_get_type(componentType);
		auto pair = monoComponentToType.find(mType);
		Scene& scene = MySceneManager.GetCurrentScene();
		if (pair == monoComponentToType.end())
		{
			if (SCRIPTING.IsScript(mono_class_from_mono_type(mType)))
			{
				ScriptObject<Script> script(object);
				return scene.IsActive<Script>(script);
			}
			else
			{
				//std::string name = mono_type_get_name(mType);
				//CONSOLE_ERROR(name, "is not a valid component!");
				return false;
			}
		}
		return scene.GetActive(pair->second, object);
	}

	

	static void SetActive(ScriptObject<Object> pObject, MonoReflectionType* componentType, bool val)
	{
		MonoType* mType = mono_reflection_type_get_type(componentType);
		auto pair = monoComponentToType.find(mType);
		Scene& scene = MySceneManager.GetCurrentScene();
		if (pair == monoComponentToType.end())
		{
			if (SCRIPTING.IsScript(mono_class_from_mono_type(mType)))
			{
				ScriptObject<Script> script(pObject);
				scene.SetActive((Script&)script, val);
				return;
			}
			else
			{
				//CONSOLE_ERROR(mono_type_get_name(mType), "is not a valid component!");
				return;
			}
		}
		Scene::SetActiveHelper helper{ pObject,val };
		return scene.SetActive(pair->second, &helper);
	}

	// Pathfinding
	static bool FindPath(ScriptObject<NavMeshAgent> pEnemy, glm::vec3 pDest)
	{
		NavMeshAgent& _player = pEnemy;

		return NAVMESHBUILDER.GetNavMesh()->FindPath(_player, pDest);
	}

	//Register all components to mono
	template<typename T,typename... Ts>
	static void RegisterComponent()
	{
		std::string typeName = "BeanFactory.";
		if constexpr (std::is_same_v<Entity, T>)
		{
			typeName += "GameObject";
		}
		else
		{
			typeName += GetType::Name<T>();
		}
		MonoType* managedType = mono_reflection_type_from_name(typeName.data(), SCRIPTING.GetAssemblyImage());
		if (managedType != nullptr)
		{
			monoComponentToType.emplace(managedType, GetType::E<T>());
		}
		if constexpr (sizeof...(Ts) != 0)
		{
			return RegisterComponent<Ts...>();
		}
	}

	//Register all components to mono
	template<typename... T>
	static void RegisterComponent(TemplatePack<T...>)
	{
		RegisterComponent<T...>();
	}

	//Register all components to mono
	static void RegisterComponents()
	{
		monoComponentToType.clear();
		RegisterComponent(AllObjectTypes());
	}

	//Registers all defined internal calls with mono
	static void RegisterScriptWrappers()
	{
		Register(DestroyGameObject);
		Register(HasComponent);
		Register(Get);
		Register(GetLayer);
		Register(GetLayerName);
		Register(GetActive);
		Register(SetActive);
		Register(LoadScene);
		Register(LoadNext);
		Register(AddComponent);
		
		Register(CloneGameObject);

		// Input System
		Register(GetKey);
		Register(GetKeyUp);
		Register(GetKeyDown);
		Register(GetMouseDown);
		Register(GetMouseDelta);
		Register(LockCursor);

		// Physics System
		Register(Raycast);

		//Mesh Renderer
		Register(GetMaterial);
		Register(SetMaterial);

		// Transform Component
		Register(SetTransformParent);
		Register(GetWorldPosition);
		Register(GetWorldRotation);
		Register(GetWorldScale);
		Register(GetLocalPosition);
		Register(GetLocalRotation);
		Register(GetLocalScale);
		Register(SetWorldPosition);
		Register(SetWorldRotation);
		Register(SetWorldScale);
		Register(SetLocalPosition);
		Register(SetLocalRotation);
		Register(SetLocalScale);

		// Audio Component
		Register(AudioSourcePlay);
		Register(StopMusic);
		Register(PauseMusic);
		Register(ResumeMusic);
		Register(SetMusicFade);
		Register(EnableSFX);
		Register(PauseComponent);


		// Animator Component
		Register(PlayAnimation);
		Register(PauseAnimation);
		Register(StopAnimation);
		Register(GetProgress);
		Register(SetProgress);
		Register(GetSpeed);
		Register(SetSpeed);
		Register(SetDefaultState);
		Register(SetState);
		Register(SetNextState);
		Register(GetState);

		// Particle Component
		Register(ParticlesPlayer);

		// Camera Component
		Register(SetCameraTarget);
		Register(GetRightVec);
		Register(GetUpVec);
		Register(GetForwardVec);

		// Tag Component
		Register(GetTag);
		Register(FindPath);
		Register(GetScrollState);

		// SpriteRenderer Component
		Register(IsButtonClicked);
	}
#endif // !SCRIPT_WRAPPERS_H