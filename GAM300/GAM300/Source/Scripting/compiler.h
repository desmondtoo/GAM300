/*!***************************************************************************************
\file			Compiler.h
\project
\author			Zacharie Hong

\par			Course: GAM300
\par			Section:
\date			01/09/2023

\brief
	This file holds the definition of functions for compiler

All content © 2023 DigiPen Institute of Technology Singapore. All rights reserved.
*****************************************************************************************/

#pragma once

namespace Utils
{
	//Compiles all CS scripts in project path using Roslyn Compiler
	static void CompileDll()
	{
		const char* command {"Tools\\Roslyn\\csc.exe -r:System.Numerics.dll -recurse:*.cs -platform:x64 -t:library -out:scripts.dll -unsafe"};

		system(command);
		//std::ifstream myfile("output.txt"); // this is equivalent to the above 
		//std::string buffer;
		//char mychar;
		//if (!myfile.is_open())
		//	return;
		//while (myfile) 
		//{
		//	mychar = (char)myfile.get();
		//	buffer += mychar;
		//}

		//// Remove copyright header
		//size_t pos = buffer.find("reserved.");
		//buffer.erase(0, pos + 11);

		//size_t pos1 = 0;
		//size_t pos2 = 0;
		//std::string parsedBuffer;
		//// Parse each message into tmpBuffer
		//while ((pos1 = buffer.find_first_of("\n", pos1)) != std::string::npos)
		//{

		//	if (pos2 >= buffer.size() || pos1 == buffer.size())
		//		break;



		//	std::string tmpBuffer = buffer.substr(pos2, pos1 - pos2 + 1);
		//	size_t pos3 = tmpBuffer.find_last_of("\\");
		//	if (pos3 != std::string::npos)
		//	{
		//		tmpBuffer.erase(0, pos3+1);
		//	}

		//	std::string errorCode;
		//	size_t errorCodePos{0};
		//	size_t end{ 0 };
		//	if ((errorCodePos = tmpBuffer.find("CS")) != std::string::npos)
		//	{
		//		end = tmpBuffer.find_first_of(":", errorCodePos);
		//		errorCode = "[" + tmpBuffer.substr(errorCodePos, end - errorCodePos) + "]";
		//	}
		//	else
		//		errorCode = "[]";

		//	std::string msgType;

		//	if (tmpBuffer.find("warning") != std::string::npos)
		//	{
		//		msgType = "[Warning]";
		//	}
		//	else if (tmpBuffer.find("error") != std::string::npos)
		//	{
		//		msgType = '[' + "ERROR" + ']';
		//	}
		//	else
		//	{
		//		msgType = "[Message]";
		//	}

		//	std::string fileName;
		//	if (tmpBuffer.find_first_of(":") != std::string::npos)
		//		fileName = tmpBuffer.substr(0, tmpBuffer.find_first_of(":"));
		//	else
		//		fileName = "";

		//	fileName += tmpBuffer.substr(end, tmpBuffer.size() - end);


		//	parsedBuffer += msgType + errorCode + fileName;



		//	pos2 = ++pos1;



		//}
		////MyEventSystem->publish(new EditorConsoleLogEvent(buffer));
		////MyEventSystem->publish(new EditorConsoleLogEvent(parsedBuffer));
		//myfile.close();

	}
}
