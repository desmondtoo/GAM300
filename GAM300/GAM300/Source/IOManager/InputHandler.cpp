#include "Precompiled.h"

#include "InputHandler.h"
// ---------------------------------------------------------------------------------------
// KEYBOARD 
// ---------------------------------------------------------------------------------------

bool InputHandler::isKeyButtonHolding(int key)
{
	//return glfwGetKey(GLFW_Handler::ptr_window, key);
	if ( (InputHandler::curr_Key_states[key] == 2) || (InputHandler::curr_Key_states[key] == 1))
	{
		return true;
	}

	return false;
}

bool InputHandler::isKeyButtonPressed(int key)
{
	if ( (InputHandler::curr_Key_states[key] == 1) && (InputHandler::last_Key_states[key] == 0))
	{
		InputHandler::last_Key_states[key] = 1;
		return true;
	}
	return false;
}


void InputHandler::setKeystate_Press(int key)
{
	if (InputHandler::curr_Key_states[key] == 0)
	{
		InputHandler::curr_Key_states[key] = 1;
		InputHandler::last_Key_states[key] = 0;
	}
	else if (InputHandler::last_Key_states[key] == 1)
	{
		InputHandler::curr_Key_states[key] = 2;
	}
}

void InputHandler::setKeystate_Hold(int key)
{
	InputHandler::curr_Key_states[key] = 2;
}

void InputHandler::setKeystate_Release(int key)
{
	InputHandler::curr_Key_states[key] = 0;
}

void InputHandler::copyingCurrStatetoLast()
{
	for (int i = 0; i < MAX_KEY_COUNT; ++i)
		InputHandler::last_Key_states[i] = InputHandler::curr_Key_states[i];
}


// ---------------------------------------------------------------------------------------
// MOUSE 
// ---------------------------------------------------------------------------------------

bool InputHandler::isMouseButtonPressed_L()
{
	return glfwGetMouseButton(GLFW_Handler::ptr_window, GLFW_MOUSE_BUTTON_LEFT);
}
bool InputHandler::isMouseButtonPressed_R()
{
	return glfwGetMouseButton(GLFW_Handler::ptr_window, GLFW_MOUSE_BUTTON_RIGHT);
}

std::pair<double, double> InputHandler::getMousePos()
{
	std::pair<double, double>X_Y;
	glfwGetCursorPos(GLFW_Handler::ptr_window, &X_Y.first, &X_Y.second);
	X_Y.second = GLFW_Handler::height - X_Y.second;
	return X_Y;
}

double InputHandler::getMouseX()
{
	return getMousePos().first;
}

double InputHandler::getMouseY()
{
	return getMousePos().second;
}
