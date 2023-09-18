/*!***************************************************************************************
\file			DemoSystem.cpp
\project
\author         Sean Ngo

\par			Course: GAM300
\date           18/09/2023

\brief
    This file contains the definitions of the following:
    1.

All content � 2023 DigiPen Institute of Technology Singapore. All rights reserved.
******************************************************************************************/
#include "Precompiled.h"

#include "DemoSystem.h"
#include "Scene/Scene.h"
#include "Core/Events.h"
#include "Core/EventsManager.h"

void DemoSystem::Init()
{
	CreateSceneEvent createScene(nullptr);
	EVENTS.Publish(&createScene);

	Scene& scene = *createScene.scene;

	Handle<Entity>& floorHandle = scene.AddEntity();
	Entity& floor = floorHandle.Get();
	scene.singleComponentsArrays.GetArray<Transform>().DenseSubscript(floor.denseIndex).translation = Vector3(0.f, 0.f, 0.f);
	scene.singleComponentsArrays.GetArray<Transform>().DenseSubscript(floor.denseIndex).scale = Vector3(300.f, 10.0f, 300.f);

	// test instance rendering
	for (int i = 0; i < 5; ++i)
	{
		Handle<Entity>& entHandle = scene.AddEntity();
		Entity& tempent = entHandle.Get();
		scene.singleComponentsArrays.GetArray<Transform>().DenseSubscript(tempent.denseIndex).translation = Vector3((rand() % 1000) - 500.f, (rand() % 1000) - 500.f, (rand() % 1000) - 500.f);
		scene.singleComponentsArrays.GetArray<Transform>().DenseSubscript(tempent.denseIndex).scale = Vector3((rand() % 50), (rand() % 50), (rand() % 50));
		scene.AddComponent<MeshRenderer>(tempent);
	}
	scene.AddComponent<Script>(scene.entities[0]).name = "Player";
	scene.AddComponent<Rigidbody>(scene.entities[0]);
	Entity& box = scene.AddEntity().Get();
	scene.AddComponent<MeshRenderer>(box);
	scene.singleComponentsArrays.GetArray<Transform>().DenseSubscript(box.denseIndex).translation = Vector3(0.f, 100.f, 0.f);
	scene.singleComponentsArrays.GetArray<Transform>().DenseSubscript(box.denseIndex).scale = Vector3(25.f, 25.f, 25.f);

	Entity& box2 = scene.AddEntity().Get();
	scene.AddComponent<MeshRenderer>(box2);
	scene.singleComponentsArrays.GetArray<Transform>().DenseSubscript(box2.denseIndex).translation = Vector3(0.f, 200.0f, 35.f);
	scene.singleComponentsArrays.GetArray<Transform>().DenseSubscript(box2.denseIndex).scale = Vector3(25.f, 25.f, 25.f);

	Entity& lightsource = scene.AddEntity().Get();
	scene.AddComponent<LightSource>(lightsource);

	//scene.AddEntity();

	//Entity& titty = scene.AddEntity();
	//scene.singleComponentsArrays.GetArray<Transform>().DenseSubscript(titty.denseIndex).translation = Vector3(0.f, 100.f, 0.f);
	//scene.singleComponentsArrays.GetArray<Transform>().DenseSubscript(titty.denseIndex).scale = Vector3(100.f, 10.f, 10.f);

	//// test instance rendering
	//for (int i = 0; i < 5; ++i)
	//{
	//	Entity& tempent = scene.AddEntity();
	//	scene.singleComponentsArrays.GetArray<Transform>().DenseSubscript(tempent.denseIndex).translation = Vector3((rand() % 1000) - 500.f, (rand() % 1000) - 500.f, (rand() % 1000) - 500.f);
	//	scene.singleComponentsArrays.GetArray<Transform>().DenseSubscript(tempent.denseIndex).scale = Vector3((rand() % 50), (rand() % 50), (rand() % 50));
	//}
	//scene.AddComponent<Script>(0).name = "Player";
}

void DemoSystem::Update(float dt)
{

}

void DemoSystem::Exit()
{

}