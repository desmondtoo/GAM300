﻿/*!***************************************************************************************
\file			scene.h
\project
\author			Matthew Lau

\par			Course: GAM200
\par			Section:
\date			28/07/2022

\brief
	Contains declarations for the Scene class.
	The Scene contains:
		1. load, init, update, draw, free, unload function
		2. string containing the filename of the file in which the scene data is stored on
		3. Data pertaining to the game objects in the scene

	Note: load, init, free and unload functions MUST be defined by scene sub-classes

	Contains definitions for NormalScene class which is a derived class from Scene class.
	Note: this is the latest version of our scene class, use this


All content � 2022 DigiPen Institute of Technology Singapore. All rights reserved.
******************************************************************************************/


#ifndef SCENE_H
#define SCENE_H

#include <filesystem>
#include <string>
#include <bitset>
#include "Core/Debug.h"
#include "Entity.h"
#include "Components.h"
#include <unordered_map>
#include "Editor/EditorHeaders.h"
#include "Core/EventsManager.h"


template <typename T>
using Table = std::unordered_map<Engine::UUID, T*>;

template <typename T>
using MultiTable = std::unordered_map<Engine::UUID, Table<T>>;

//UUIDs are shared between components and entities
//For multicomponents, they will have a secondary UUID for linkage

template<typename... Ts>
struct HandlesTable
{
	constexpr HandlesTable(TemplatePack<Ts...>) {}
	HandlesTable() = default;
	template <typename T1>
	bool Has(Engine::UUID uuid)
	{
		auto& entries = std::get<Table<T1>>(tables);
		if (entries.find(uuid) == entries.end())
			return false;
		return true;
	}

	template <typename T1>
	bool Has(T1& object)
	{
		auto& entries = std::get<Table<T1>>(tables);
		if (entries.find(object.uuid) == entries.end())
			return false;
		return true;
	}

	template <typename T1>
	constexpr T1& Get(Engine::UUID uuid)
	{
		return std::get<Table<T1>>(tables)[uuid];
	}

	template <typename T1>
	constexpr void erase(Engine::UUID uuid)
	{
		auto& entries = std::get<Table<T1>>(tables);
		entries.erase(uuid);
	}

	template <typename T1>
	constexpr void erase(Handle<T1>& handle)
	{
		auto& entries = std::get<Table<T1>>(tables);
		entries.erase(handle);
	}

	template <typename T1,typename... Args>
	constexpr T1* emplace(Engine::UUID uuid,T1* object)
	{
		auto& table = std::get<Table<T1>>(tables);
		auto pair = table.emplace(std::make_pair(uuid, object));
		return pair.first->second;
	}

private:
	std::tuple<Table<Ts>...> tables;
};

template<typename... Ts>
struct MultiHandlesTable
{

	constexpr MultiHandlesTable(TemplatePack<Ts...>) {}
	MultiHandlesTable() = default;
	template <typename T1>
	bool HasHandle(Engine::UUID uuid, Engine::UUID sub_uuid)
	{
		auto& entries = std::get<MultiTable<T1>>(tables);
		auto it = entries.find(uuid);
		if (it == entries.end())
			return false;
		if (it->find(sub_uuid) == it->end())
			return false;
		return true;
	}

	template <typename T1>
	constexpr T1& Get(Engine::UUID euid, Engine::UUID uuid)
	{
		return std::get<MultiTable<T1>>(tables)[uuid][sub_uuid];
	}

	template <typename T1>
	constexpr void erase(Engine::UUID uuid, Engine::UUID sub_uuid)
	{
		auto& entries = std::get<MultiTable<T1>>(tables);
		entries.erase(uuid);
	}

	template <typename T1>
	constexpr void erase(Handle<T1>& handle)
	{
		auto& entries = std::get<MultiTable<T1>>(tables);
		entries.erase(handle);
	}

	template <typename T1, typename... Args>
	constexpr Handle<T1>& emplace(Engine::UUID uuid, T1& object)
	{
		auto& table = std::get<MultiTable<T1>>(tables);
		auto pair = table.emplace(std::make_pair(uuid, Handle<T1>(uuid, &object)));
		return pair.first->second;
	}

private:
	std::tuple<MultiTable<Ts>...> tables;
};



using SingleObjects = decltype(AllComponentTypes::Concatenate(TemplatePack<Entity>()));
using SingleHandles = decltype(HandlesTable(SingleObjects()));
using MultiHandles = decltype(MultiHandlesTable(MultiComponentTypes()));

using EntitiesList = ObjectsList<Entity, MAX_ENTITIES>;

using EntitiesPtrArray = std::vector<Entity*>;

struct Scene
{
	enum class State : char 
	{
		Edit = 0,
		Play,
		Paused
	};

	std::string sceneName;
	EntitiesList entities;	//Vector should be in order
	SingleComponentsArrays singleComponentsArrays;
	MultiComponentsArrays multiComponentsArrays;
	EntitiesPtrArray entitiesDeletionBuffer;
	ComponentsBufferArray componentsDeletionBuffer;
	SingleHandles singleHandles;
	MultiHandles multiHandles;

	std::filesystem::path filePath;
	State state;

	Scene(const std::string& _filepath);

	Scene(Scene&) = delete;
	Scene& operator=(Scene&) = delete;

	template<typename T>
	T& Get(Engine::UUID uuid)
	{
		E_ASSERT
		(
			singleHandles.HasHandle<T>(uuid),
			"UUID: ", uuid, " of ", typeid(T).name() + strlen("struct "), " doesn't exist in this scene"
		);
		return singleHandles.GetHandle<T>(uuid);
	}

	template<typename T, typename Owner>
	T& Get(Owner& object)
	{
		E_ASSERT
		(
			singleHandles.HasHandle<T>(object.EUID()),
			"EUID: ", object.EUID(), " of ", typeid(T).name() + strlen("struct "),
			" doesn't exist in this scene"
		);
		return singleHandles.GetHandle<T>(object.euid);
	}


	Entity* AddEntity(Engine::UUID euid = Engine::CreateUUID())
	{
		while (singleHandles.Has<Entity>(euid) || euid == 0)
		{
			euid = Engine::CreateUUID();
		}
		Entity& entity = entities.emplace_back(euid);
		entity.pScene = this;
		entity.denseIndex = entities.GetDenseIndex(entity);
		entities.SetActive(entity.denseIndex);
		singleHandles.emplace(euid, &entity);
		AddComponent<Transform>(entity);
		Tag& tag = AddComponent<Tag>(entity);
		tag.name = "New GameObject(";
		tag.name += std::to_string(entities.size());
		tag.name += ")";
		// Add the entity to the inspector
		ObjectCreatedEvent<Entity> e = {entity.EUID()};
		EVENTS.Publish(&e);
		return &entity;
	}

	template<typename T, typename... Ts>
	struct DestroyComponentsGroup
	{
		Scene& scene;
		Entity& entity;
		DestroyComponentsGroup(TemplatePack<T, Ts...> pack) {}
		DestroyComponentsGroup(Scene& _scene, Entity& _entity) : scene{ _scene }, entity{_entity}
		{
			DestroyComponents<T, Ts...>();
		}

		template <typename T1, typename... T1s>
		void DestroyComponents()
		{
			if (scene.HasComponent<T1>(entity))
			{
				if constexpr (SingleComponentTypes::Has<T1>())
				{
					scene.singleComponentsArrays.GetArray<T1>().TryErase(entity.denseIndex);
				}
				else if constexpr (MultiComponentTypes::Has<T1>())
				{
					scene.multiComponentsArrays.GetArray<T1>().erase(entity.denseIndex);
				}
			}
			if constexpr (sizeof...(T1s) != 0)
			{
				DestroyComponents<T1s...>();
			}
		}
	};

	using DestroyEntityComponents = decltype(DestroyComponentsGroup((AllComponentTypes())));

	template<typename T>
	void Destroy(T& object)
	{
		if constexpr (std::is_same<T, Entity>())
		{
			entitiesDeletionBuffer.push_back(&object);
			entities.SetActive(object.denseIndex,false);
		}
		else if constexpr (SingleComponentTypes::Has<T>())
		{
			componentsDeletionBuffer.GetArray<T>().push_back(&object);
			auto& arr = singleComponentsArrays.GetArray<T>();
			ObjectIndex index = arr.GetDenseIndex(object);
			arr.SetActive(index,false);
			entities.DenseSubscript(index).hasComponentsBitset.set(GetComponentType::E<T>(), false);
		}
		else if constexpr (MultiComponentTypes::Has<T>())
		{
			componentsDeletionBuffer.GetArray<T>().push_back(&object);
			auto& arr = multiComponentsArrays.GetArray<T>();
			ObjectIndex index = arr.GetDenseIndex(object);
			arr.SetActive(object, false);
			if (arr.DenseSubscript(index).size() == 1)
				entities.DenseSubscript(index).hasComponentsBitset.set(GetComponentType::E<T>(), false);
		}
		static_assert(true,"Not a valid type of object to destroy");
	}

	template <typename T, typename... Ts>
	struct ClearBufferStruct
	{
		ClearBufferStruct(TemplatePack<T, Ts...> pack) {}
		ClearBufferStruct(Scene& _scene) : scene{ _scene }
		{
			CleanComponents<T, Ts...>();
		}
		Scene& scene;
		template <typename T1, typename... T1s>
		void CleanComponents()
		{
			auto& arr = scene.componentsDeletionBuffer.GetArray<T1>();
			if constexpr (SingleComponentTypes::Has<T1>())
			{
				auto& compArray = scene.singleComponentsArrays.GetArray<T1>();
				for (T1* pComponent : arr)
				{
					compArray.erase(*pComponent);
				}
			}
			else if constexpr (MultiComponentTypes::Has<T1>())
			{
				auto& compArray = scene.multiComponentsArrays.GetArray<T1>();
				for (T1* pComponent : arr)
				{
					compArray.erase(*pComponent);
				}
			}
			arr.clear();
			if constexpr (sizeof...(T1s) != 0)
			{
				CleanComponents<T1s...>();
			}
		}
	};

	using ClearBufferHelper = decltype(ClearBufferStruct(AllComponentTypes()));

	void ClearBuffer()
	{
		//Destroy components
		ClearBufferHelper(*this);
		for (Entity* pEntity : entitiesDeletionBuffer)
		{
			DestroyEntityComponents(*this,*pEntity);
			entities.erase(*pEntity);
		}
		entitiesDeletionBuffer.clear();
	}

	template <typename Component>
	bool ComponentIsEnabled(uint32_t index, size_t multiIndex);

	template <typename Component>
	auto& GetComponentsArray()
	{
		if constexpr (SingleComponentTypes::Has<Component>())
		{
			return singleComponentsArrays.GetArray<Component>();
		}
		else if constexpr (MultiComponentTypes::Has<Component>())
		{
			return multiComponentsArrays.GetArray<Component>();
		}
	}

	template <typename Component>
	auto GetComponents(Entity& entity)
	{
		return multiComponentsArrays.GetArray<Component>().DenseSubscript(entity.denseIndex);
	}

	bool IsActive(Entity& entity)
	{
		return entities.IsActiveDense(entity.denseIndex);
	}

	void SetActive(Entity& entity, bool val = true)
	{
		entities.SetActive(entity, val);
	}


	template <typename Component>
	void ComponentSetEnabled(uint32_t index,bool value, size_t multiIndex = 0);


	template <typename Component>
	bool HasComponent(const Entity& entity)
	{
		if constexpr (AllComponentTypes::Has<Component>())
		{
			return entity.hasComponentsBitset.test(GetComponentType::E<Component>());
		}
		return false;
	}

	template <typename Component>
	bool HasComponent(ObjectIndex& denseIndex)
	{
		if constexpr (AllComponentTypes::Has<Component>())
		{
			return entities.DenseSubscript(denseIndex).hasComponentsBitset.test(GetComponentType::E<Component>());
		}
		return false;
	}

	Entity& GetEntityByUUID(size_t UUID)
	{
		for (Entity& entity : entities)
			if (UUID == entity.EUID())
				return entity;

		std::string str = "Entity of EUID:";
		str += UUID + " cannot be found";
		E_ASSERT(false, str.c_str());
	}

	template <typename Component, typename Owner>
	Component& GetComponent(Owner& obj)
	{
		//ASSERT(HasComponent<Component>(entity), "Entity does not have component");
		ObjectIndex denseIndex;
		if constexpr (std::is_same_v<Entity, Owner>)
		{
			denseIndex = obj.denseIndex;
		}
		else if constexpr (AllComponentTypes::Has<Owner>())
		{
			denseIndex = GetComponentsArray<Owner>().GetDenseIndex(obj);
		}

		if constexpr (SingleComponentTypes::Has<Component>())
		{
			return GetComponentsArray<Component>().DenseSubscript(denseIndex);
		}
		else if constexpr (MultiComponentTypes::Has<Component>())
		{
			return *GetComponentsArray<Component>().DenseSubscript(denseIndex).front();
		}
	}

	template <typename Component>
	Component& AddComponent(Engine::UUID uuid)
	{
		static_assert(AllComponentTypes::Has<Component>(), "Type is not a valid component!");
		if constexpr (SingleComponentTypes::Has<Component>())
		{
			auto& arr = singleComponentsArrays.GetArray<Component>();
			Handle<Entity>& entityHandle{GetHandle<Entity>(uuid)};
			Component& component = arr.emplace(entityHandle.Get().denseIndex);
			entityHandle.Get().hasComponentsBitset.set(GetComponentType::E<Component>(), true);
			arr.SetActive(entityHandle.Get().denseIndex);
			return component;
		}
		else if constexpr (MultiComponentTypes::Has<Component>())
		{
			auto& arr = multiComponentsArrays.GetArray<Component>();
			Handle<Entity>& entityHandle{ GetHandle<Entity>(uuid) };
			Component& component = arr.emplace(entityHandle.Get().denseIndex);
			entityHandle.Get().hasComponentsBitset.set(GetComponentType::E<Component>(), true);
			arr.SetActive(component);
			return component;
		}
	}


	template <typename Component>
	Component& AddComponent(const Entity& entity)
	{
		return AddComponent<Component>(entity.UUID());
	}
};
#endif SCENE_H