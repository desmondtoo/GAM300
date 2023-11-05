/*!***************************************************************************************
\file			EditorHistory.h
\project
\author         Joseph Ho

\par			Course: GAM300
\date           07/10/2023

\brief
	This file contains the declarations and types of the History system that handles the
	undo and redo functionality in the editor.

All content � 2023 DigiPen Institute of Technology Singapore. All rights reserved.
******************************************************************************************/

#ifndef EDITORHISTORY_H
#define EDITORHISTORY_H

#include "Precompiled.h"
#include "Scene/Object.h"
#include "Core/EventsManager.h"
#include "Scene/Components.h"

using Component = Object*;

enum ChangeType { REDO, UNDO };

//struct for each undo/redo change
struct Change {

	Change() {
		component = nullptr;
		oldreference = newreference = nullptr;
	}



	Change(Component comp, std::string prop) : component(comp), property(prop) { oldreference = newreference = nullptr; }

	std::string property; //name of property that changed
	Component component; //component of which value was changed

	//variables for script references
	bool isreference = false; //check if value changed is a reference
	Component oldreference; //holds the old reference that was changed
	Component newreference; //holds the new reference that was changed



	property::data previousValue; //holds the old value that was changed
	property::data newValue;	  //holds the new value that was changed
};

using History = std::stack<Change>;

//History Manager contains the data containers that store all the undo and redo moves in the editor
class HistoryManager {
	public:
		//Checks whether the undo stack is empty
		bool UndoStackEmpty();

		//Checks whether the redo stack is empty
		bool RedoStackEmpty();

		//Clears the entire redo stack
		void ClearRedoStack();

		//Return the reference to under
		History& GetUndoStack() { return UndoStack; }

		void SetScriptField(Change& change, ChangeType);
		void SetScriptReference(Change& change, ChangeType);

		bool UndoChange();

		//redo will be populated with undo changes
		bool RedoChange();

		//Add a reference 
		void AddReferenceChange(Change& change, Component oldRef, Component newRef);

		//change a variable and add the change to the undo buffer
		template <typename T>
		void SetPropertyValue(Change& change, T& var, T& value) {

			//replace the variable and add it to the undo change
			change.previousValue = var;
			var = value;
			change.newValue = var;

			//push this change onto the undo stack
			UndoStack.push(change);

			//clear redo when new changes are made
			ClearRedoStack();
		}
	private: 
		History UndoStack;
		History RedoStack;
};

#endif //EDITORHISTORY_H