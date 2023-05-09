/*!***************************************************************************************
\file			EngineCore.h
\project
\author			Zacharie Hong

\par			Course: GAM300
\par			Section:
\date			27/09/2022

\brief
	NOTE: DO NOT INCLUDE IN ANY OTHER PLACE OTHER THAN MAIN
	This file manages all the systems as a super system itself. 

All content � 2023 DigiPen Institute of Technology Singapore. All rights reserved.
*****************************************************************************************/
#ifndef ENGINE_CORE_H
#define ENGINE_CORE_H

#include "SystemInterface.h"
#include "FramerateController.h"
//#include "Physics/PhysicsSystem.h"
#include "Scene/SceneManager.h"
#include <vector>

#define MyEngineCore EngineCore::Instance()

enum class EngineState
{
	Run,
	Editor,
	Paused,
	Quit
};

ENGINE_SYSTEM(EngineCore)
{
public:

	/**************************************************************************/
	/*!
		\brief
		Initializes all the systems that are Instantiated under vector systems
	*/
	/**************************************************************************/
	void Init()
	{
		MyFrameRateController.Init();
		systems =
		{
			&SceneManager::Instance(),
			//&PhysicsSystem::Instance(),
		};
		for (ISystem* pSystem : systems)
		{
			pSystem->Init();
		}
	}

	/**************************************************************************/
	/*!
		\brief
		Calls the update function of all the systems in vector systems depending
		if it should only be update in play mode or not
	*/
	/**************************************************************************/
	void Update()
	{
		//while (state != EngineState::Quit)
		//{
		//	if (state == EngineState::Run)
		//	{
				//MyFrameRateController.Start();
		for (ISystem* pSystem : systems)
		{
			if (pSystem->GetMode() & mode)
			{
				pSystem->Update();
			}
		}
				//MyFrameRateController.End();
			//}
		//}
	}

	/**************************************************************************/
	/*!
		\brief
		Calls the exit function of all the systems in vector systems, to be
		called at engine exit
	*/
	/**************************************************************************/
	void Exit()
	{
		for (auto iter = systems.rbegin(); iter != systems.rend(); ++iter)
		{
			(*iter)->Exit();
		}
	}
private:
	std::vector<ISystem*> systems;
	EngineState state = EngineState::Run;
	SystemMode mode = ENUM_SYSTEM_EDITOR;
};
#endif // !CORE_H