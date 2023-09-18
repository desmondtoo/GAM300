/*!***************************************************************************************
\file			events.h
\project
\author			Zacharie Hong
				Sean Ngo

\par			Course: GAM200
\par			Section:
\date			15/02/2023

\brief
	This file contains the declarations of all event structures in the engine.

All content � 2023 DigiPen Institute of Technology Singapore. All rights reserved.
******************************************************************************************/
#ifndef EVENTS_H
#define EVENTS_H

#include "Scene/Handle.h"

struct Entity;
struct Scene;

struct IEvent
{
protected:
	virtual ~IEvent() {};
};

struct StartPreviewEvent : IEvent
{
	StartPreviewEvent(){}
};

struct StopPreviewEvent : IEvent
{
	StopPreviewEvent(){}
};

struct QuitEngineEvent : IEvent
{
	QuitEngineEvent() {};
};

struct CreateSceneEvent : IEvent
{
	CreateSceneEvent(Scene* _scene) : scene(_scene) {}
	Scene* scene;
};

struct LoadSceneEvent : IEvent
{
	LoadSceneEvent(const std::string& _filePath) :filePath{ _filePath } {}
	std::string filePath;
};

struct SaveSceneEvent : IEvent
{
	SaveSceneEvent() {};
	SaveSceneEvent(const std::string& _filePath) :filePath{ _filePath } {}
	std::string filePath;
};

struct IsNewSceneEvent : IEvent
{
	IsNewSceneEvent() : data(false) {}
	bool data;
};

struct SceneChangingEvent : IEvent 
{
	SceneChangingEvent(Scene& _scene) : scene(_scene) {}
	Scene& scene;
	std::string filePath;
};

struct ClearEntitiesEvent: IEvent
{
	ClearEntitiesEvent() {};
};

//template <typename T>
//struct ReflectComponentEvent : IEvent
//{
//	ReflectComponentEvent(T& _component) : component{ _component } {}
//	T& component;
//};
//
//struct ReflectEntityEvent : IEvent
//{
//	ReflectEntityEvent(Entity& _entity) : entity{ _entity} {}
//	Entity& entity;
//};


template <size_t FTYPE>
struct FileTypeModifiedEvent : IEvent 
{
	FileTypeModifiedEvent(const wchar_t* _fileName, size_t _fileState) : fileName{ _fileName }, fileState{_fileState}{}
	const wchar_t* fileName;
	size_t fileState;
};

struct FileModifiedEvent : IEvent
{
	FileModifiedEvent(const wchar_t* _filePath, size_t _fileState) : filePath{ _filePath }, fileState{ _fileState }{}
	const wchar_t* filePath;
	size_t fileState;
};

struct SceneStartEvent : IEvent{};

struct SceneStopEvent : IEvent {};

struct SelectedEntityEvent : IEvent
{
	SelectedEntityEvent(Handle<Entity>& _handle) : handle{ _handle }{}
	Handle<Entity>& handle;
};

template <typename T>
struct ObjectCreatedEvent : IEvent
{
	ObjectCreatedEvent(Handle<T>& _handle) : handle{ _handle } {}
	Handle<T>& handle;
};

//struct GetCurrentSceneEvent
//{
//	GetCurrentSceneEvent(Scene& _scene) :scene{ _scene } {};
//	Scene& scene;
//};

//template <typename T>
//struct ComponentAddEvent : IEvent
//{
//	ComponentAddEvent(const Entity& _entity, T*& _componentContainer, UUID _uuid = UUID()) :
//		entity{ _entity }, componentContainer{ _componentContainer }, uuid{_uuid} {}
//	Entity& entity;
//	T*& componentContainer;
//	UUID uuid;
//};
//
//template <typename T>
//struct ComponentDeleteEvent : IEvent
//{
//	ComponentDeleteEvent(T& _component) : component{_component}{}
//	T& component;
//};

#endif //!EVENTS_H