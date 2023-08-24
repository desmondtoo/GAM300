#include "Precompiled.h"
#include "InputSystem.h"
#include "InputHandler.h"
#include "Handler_GLFW.h"


void InputSystem::Init()
{
	InputHandler::setFullscreen(false);
	//InputHandler:: = new int[55];
}
void InputSystem::Update()
{
	InputHandler::reset();

	glfwPollEvents();

	if (InputHandler::isKeyButtonPressed(GLFW_KEY_F11))
	{
		std::cout << "toggling between fullscreen and non fullscreen\n";
		InputHandler::setFullscreen(!InputHandler::isFullscreen());
	}

	//--------------------------------------------------------------
	// KEYBOARD TESTING using button - 0
	//--------------------------------------------------------------
	
	/*if (InputHandler::isKeyButtonPressed(GLFW_KEY_0))
	{
		std::cout << "Pressed 0\n";
	}
	
	if (InputHandler::isKeyButtonHolding(GLFW_KEY_0))
	{
		std::cout << "Holding 0\n";
	}*/

	//if (InputHandler::isMouseButtonPressed_L())
	//{
	//	std::cout << "Pressing LEFT\n";
	//}
	//if (InputHandler::isMouseButtonPressed_R())
	//{
	//	std::cout << "Pressing RIGHT\n";
	//}
	//std::cout << InputHandler::getMouseY() << "\n";
	

}
void InputSystem::Exit()
{

}