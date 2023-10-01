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
#include "Scene/SceneManager.h"
#include "ScriptingSystem.h"


#ifndef SCRIPT_WRAPPERS_H
#define SCRIPT_WRAPPERS_H

	static std::unordered_map<MonoType*, size_t> monoComponentToType;

	#define Register(METHOD) mono_add_internal_call("BeanFactory.InternalCalls::"#METHOD,METHOD)

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

	//Gets object that entity has
	static void* Get(Object* pEntity, MonoReflectionType* componentType)
	{
		MonoType* mType = mono_reflection_type_get_type(componentType);
		auto pair = monoComponentToType.find(mType);
		if (pair == monoComponentToType.end())
		{
			//Cant find
		}
		size_t addr = reinterpret_cast<size_t>(MySceneManager.GetCurrentScene().Get(pair->second, pEntity));
		addr += 8;
		return reinterpret_cast<void*>(addr);
	}


	//Checks if entity has a component
	static bool HasComponent(Entity* pEntity, MonoReflectionType* componentType)
	{
		MonoType* managedType = mono_reflection_type_get_type(componentType);
		return pEntity->hasComponentsBitset.test(monoComponentToType[managedType]);
	}


	//Deletes a gameobject
	static void DestroyGameObject(Entity* pGameObject)
	{
		MySceneManager.GetCurrentScene().Destroy(*pGameObject);
	}

	//GENERIC_RECURSIVE(void, DestroyRecursive, MySceneManager.GetCurrentScene().Destroy(*(T*)pObject))
	//static void DestroyComponent(void* pComponent, MonoReflectionType* componentType)
	//{
	//	MonoType* managedType = mono_reflection_type_get_type(componentType);
	//	DestroyRecursive(monoComponentToType[managedType],pComponent);
	//}

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
		Register(GetKey);
		Register(GetKeyUp);
		Register(GetKeyDown);
		Register(GetMouseDown);
		Register(DestroyGameObject);
		Register(HasComponent);
		Register(Get);
	}
#endif // !SCRIPT_WRAPPERS_H