/*!***************************************************************************************
\file			BehaviorTreeBuilder.cpp
\project
\author         Davin Tan

\par			Course: GAM300
\date           28/09/2023

\brief
	This file contains the definitions of the following:
	1. BehaviorTreeBuilder singleton class
		a. Builds the behavior trees in the Trees folder to store into memory
		b. Helper functions for deserialization of RapidJSON format

All content � 2023 DigiPen Institute of Technology Singapore. All rights reserved.
******************************************************************************************/

#include "Precompiled.h"
#include "BehaviorTreeBuilder.h"
#include "NodeIncludes.h"

void BehaviorTreeBuilder::Init()
{
	BuildTrees();
}

void BehaviorTreeBuilder::Update(float dt)
{
	UNREFERENCED_PARAMETER(dt);
}

void BehaviorTreeBuilder::Exit()
{
	for (auto& [key, data] : mBehaviorTrees)
	{
		delete data;
	}
	mBehaviorTrees.clear();
}

BehaviorTree* BehaviorTreeBuilder::GetBehaviorTree(std::string treeName)
{
	if (mBehaviorTrees.find(treeName) != mBehaviorTrees.end())
	{
		return mBehaviorTrees[treeName];
	}

	E_ASSERT(false, "Nullptr returned while trying to find behavior tree...");
}

void BehaviorTreeBuilder::BuildTrees()
{
	const std::filesystem::path treesFile(BHTFiles);
	E_ASSERT(std::filesystem::exists(treesFile), "Behavior tree file does not exist! Check if proper filepath is entered.");

	for (const auto& dir : std::filesystem::recursive_directory_iterator(treesFile))
	{
		std::string filePath = dir.path().generic_string();
		std::string fileName;

		// Getting the file name for behavior tree name
		for (size_t i = filePath.find_last_of('/') + 1; i != filePath.find_last_of('.'); ++i)
		{
			fileName += filePath[i];
		}

		// Reading the file data here to build the tree and store into our vector of trees
		std::ifstream ifs(filePath);
		E_ASSERT(ifs, "Error opening behavior tree file to read data!");

		std::stringstream buffer;
		buffer << ifs.rdbuf();
		ifs.close();

		rapidjson::Document doc;
		const std::string data(buffer.str());
		doc.Parse(data.c_str());

		// Root node
		const rapidjson::Value& tree = doc[fileName.c_str()][0]; // We can put 0 here because there can only be 1 root node

		std::string rootName = tree["NodeName"].GetString();
		std::string rootType = tree["NodeType"].GetString();

		// Deserialize the root node first
		BehaviorNode* rootNode = BuildNode(rootType, rootName);

		// Deserialize the children nodes
		const rapidjson::Value& _children = tree["Children"];

		for (size_t i = 0; i < _children.Size(); ++i)
		{
			// Recursively build tree here from the root node
			const rapidjson::Value& mChild = _children[static_cast<int>(i)];
			BuildChildren(rootNode, mChild);
		}

		BehaviorTree* tempTree = new BehaviorTree(fileName, rootNode);

		mBehaviorTrees[fileName] = std::move(tempTree);
	}
}

void BehaviorTreeBuilder::BuildChildren(BehaviorNode* mNode, const rapidjson::Value& val)
{
	std::string nodeName = val["NodeName"].GetString();
	std::string nodeType = val["NodeType"].GetString();

	BehaviorNode* childNode = BuildNode(nodeType, nodeName);
	mNode->addChild(childNode);

	const rapidjson::Value& _children = val["Children"];

	// Build the children of this children
	for (size_t i = 0; i < _children.Size(); ++i)
	{
		const rapidjson::Value& mChild = _children[static_cast<int>(i)];
		BuildChildren(childNode, mChild);
	}
}

// Following functions below require changes every addition of behavior nodes
BehaviorNode* BehaviorTreeBuilder::BuildNode(std::string nodeType, std::string nodeName)
{
	if (nodeType == "ControlFlow")
	{
		BehaviorNode* mNode = DeserializeControlFlow(nodeName);
		return mNode;
	}
	else if (nodeType == "Decorator")
	{
		BehaviorNode* mNode = DeserializeDecorator(nodeName);
		return mNode;
	}
	else if (nodeType == "LeafNode")
	{
		BehaviorNode* mNode = DeserializeLeafNode(nodeName);
		return mNode;
	}
	else
	{
		E_ASSERT(false, "Nullptr returned while building behavior tree node.");
	}

	return nullptr;
}

BehaviorNode* BehaviorTreeBuilder::DeserializeControlFlow(std::string nodeName)
{
	if (nodeName == "Sequencer")
	{
		BehaviorNode* mNode;
		mNode = new Sequencer();
		return mNode;
	}
	else
	{
		E_ASSERT(false, "Nullptr returned while deserializing control flow.");
	}

	return nullptr;
}

BehaviorNode* BehaviorTreeBuilder::DeserializeDecorator(std::string nodeName)
{
	if (nodeName == "Inverter")
	{
		BehaviorNode* mNode;
		mNode = new Inverter();
		return mNode;
	}
	else
	{
		E_ASSERT(false, "Nullptr returned while deserializing decorator.");
	}

	return nullptr;
}

BehaviorNode* BehaviorTreeBuilder::DeserializeLeafNode(std::string nodeName)
{
	if (nodeName == "MoveToPosition")
	{
		BehaviorNode* mNode;
		mNode = new MoveToPosition();
		return mNode;
	}
	else
	{
		E_ASSERT(false, "Nullptr returned while deserializing leaf node.");
	}

	return nullptr;
}
