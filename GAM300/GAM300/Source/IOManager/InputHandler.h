#pragma once

#include "IOManager/Handler_GLFW.h"

#define MAX_KEY_COUNT 348
class InputHandler
{
public:
	/*!*****************************************************************************
	\author
		Euan Lim
	\brief
		Check if KEYBOARD button is pressed
	param [in] key
		put in GLFW key code -> ex: GLFW_KEY_A
	\return
		GLFW_PRESS -> if pressed
		GLFW_RELEASE -> if released / not pressed
	*******************************************************************************/
	static bool isKeyButtonHolding(int key);// I guess u can use this for things like moving around

	static bool isKeyButtonPressed(int key);// UNIQUE 1 Time Press -> Pressing Tab or something like that
											// I am sure u know what i mean big boi

	static bool isMouseButtonPressed_L();
	static bool isMouseButtonPressed_R();

	static std::pair<double, double> getMousePos(); // < X , Y >	
	static double getMouseX(); 
	static double getMouseY();








	static void setKeystate_Press(int key);

	static void setKeystate_Hold(int key);

	static void setKeystate_Release(int key);

	static void copyingCurrStatetoLast();

private:
	inline static int curr_Key_states[MAX_KEY_COUNT]; // 0 (Release) , 1 (Pressed), 2 Hold
	inline static int last_Key_states[MAX_KEY_COUNT]; // Used for the difference between press and hold
};