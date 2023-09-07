#include "Precompiled.h"
#include "Entity.h"
#include "SceneManager.h"


Entity::Entity(Engine::UUID _uuid) : uuid{_uuid}{}

void Break_ParentChild(const ObjectIndex& _child) {
	Scene& curr_scene = SceneManager::Instance().GetCurrentScene();
	Transform& child = curr_scene.GetComponent<Transform>(curr_scene.entities.DenseSubscript(_child));

	//remove child from old parent
	auto& children = child.Parent->child;
	auto it = std::find(children.begin(), children.end(), &child);
	children.erase(it);

	//child does not have a parent now, is a leaf node
	child.Parent = nullptr;
}

void Set_ParentChild(const ObjectIndex& _parent, const ObjectIndex& _child) {

	

	Scene& curr_scene = SceneManager::Instance().GetCurrentScene();
	Transform& parent = curr_scene.GetComponent<Transform>(curr_scene.entities.DenseSubscript(_parent));
	Transform& child = curr_scene.GetComponent<Transform>(curr_scene.entities.DenseSubscript(_child));

	//if child has a previous parent, replace parent with this new one, remove child from old parent
	if (child.isChild()) {
		auto& children = child.Parent->child;
		auto it = std::find(children.begin(), children.end(), &child);
		children.erase(it);
	}

	parent.child.push_back(&child);

	child.Parent = &parent;
}


