#include "Precompiled.h"
#include "EditorHeaders.h"
#include "Editor.h"
#include "Scene/Scene.h"
#include "Scene/SceneManager.h"

void EditorHierarchy::Init() {
    //no selected entity at start
    selectedEntity = NON_VALID_ENTITY;
}

void EditorHierarchy::ClearLayer() {
    layer.clear();
    selectedEntity = NON_VALID_ENTITY;
}

void EditorHierarchy::DisplayEntity(const ObjectIndex& Index) {

    // ImGuiTreeNodeFlags_SpanAvailWidth

    ImGuiTreeNodeFlags NodeFlags = ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_DefaultOpen;

    if (Index == selectedEntity) {
        NodeFlags |= ImGuiTreeNodeFlags_Selected;
    }

    Scene& curr_scene = SceneManager::Instance().GetCurrentScene();

    Transform& currEntity = curr_scene.GetComponent<Transform>(curr_scene.entities.DenseSubscript(Index));

    if (currEntity.isLeaf()) {
        NodeFlags |= ImGuiTreeNodeFlags_Bullet;
    }

    //Invisible button for drag drop reordering
    ImGui::InvisibleButton("##", ImVec2(ImGui::GetWindowContentRegionWidth(), 2.5f));

    //Drag drop reordering implementation
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity")) {
            const ObjectIndex childId = *static_cast<ObjectIndex*>(payload->Data);


            if (childId != Index) {
                Entity& currEntity = curr_scene.entities.DenseSubscript(childId);
                Entity& targetEntity = curr_scene.entities.DenseSubscript(Index);

                Transform& currTransform = curr_scene.GetComponent<Transform>(currEntity);
                Transform& targetTransform = curr_scene.GetComponent<Transform>(targetEntity);

                //if target entity is a child
                if (targetTransform.isChild()) {

                    //if current entity has a parent
                    if (currTransform.isChild()) {
                        //if reordering within the same parent
                        if (currTransform.parent->child == targetTransform.parent->child) {

                            std::vector<Transform*>& arr = targetTransform.parent->child;
                            //delete curr entity from layer position
                            //           
                            auto prev_it = std::find(arr.begin(), arr.end(), &currTransform);
                            arr.erase(prev_it);
                            //reorder (reinsert) the current entity into new layer position
                            auto it = std::find(arr.begin(), arr.end(), &targetTransform);
                            arr.insert(it, &currTransform);
                        }
                        //if current entity has a different previous parent, remove it.
                        else{
                            auto& children = currTransform.parent->child;
                            auto it = std::find(children.begin(), children.end(), &currTransform);
                            children.erase(it);
                            Set_ParentChild(*targetTransform.parent, currTransform);

                            std::vector<Transform*>& arr = targetTransform.parent->child;
                            //Reorder entity to target entity location     
                            auto prev_it = std::find(arr.begin(), arr.end(), &currTransform);
                            arr.erase(prev_it);
                            //reorder (reinsert) the current entity into new layer position
                            auto it2 = std::find(arr.begin(), arr.end(), &targetTransform);
                            arr.insert(it2, &currTransform);
                        }                        
                    }
                    //if current entity is a base node (no parent)
                    else {
                        //delete instance of entity in container
                        /*auto prev_it = std::find(layer.begin(), layer.end(), &currEntity);
                        layer.erase(prev_it);*/

                        auto& parent = targetTransform.parent->child;

                        auto it = std::find(parent.begin(), parent.end(), &targetTransform);
                        parent.insert(it, &currTransform);
                        currTransform.parent = targetTransform.parent;
                    }
                   
                }
                //if target entity is a base node (no parent)
                else {
                    //if current entity has a parent, delink it
                    if (currTransform.isChild()) {
                        Break_ParentChild(childId);
                    }
                    //delete instance of entity in container
                    auto prev_it = std::find(layer.begin(), layer.end(), &currEntity);
                    layer.erase(prev_it);
                    
                    //reorder (reinsert) the current entity into new layer position
                    auto it = std::find(layer.begin(), layer.end(), &targetEntity);
                    layer.insert(it, &currEntity);
                }

            }

        }
        ImGui::EndDragDropTarget();
    }

    auto EntityName = curr_scene.GetComponent<Tag>(curr_scene.entities.DenseSubscript(Index)).name.c_str();
    bool open = ImGui::TreeNodeEx(EntityName, NodeFlags);

    //select entity from hierarchy
    if (ImGui::IsItemClicked()) {
        selectedEntity = Index;
    }

    if (ImGui::BeginDragDropSource()) {
        ImGui::SetDragDropPayload("Entity", &selectedEntity, sizeof(selectedEntity));
        ImGui::Text(curr_scene.GetComponent<Tag>(curr_scene.entities.DenseSubscript(selectedEntity)).name.c_str());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity")) {
            const ObjectIndex childId = *static_cast<ObjectIndex*>(payload->Data);
            
            Transform& currEntity = curr_scene.GetComponent<Transform>(curr_scene.entities.DenseSubscript(childId));
            Transform& targetEntity = curr_scene.GetComponent<Transform>(curr_scene.entities.DenseSubscript(Index));

            if (currEntity.isLeaf()) {
                if (childId != Index) {
                    Set_ParentChild(Index, childId);
                }
            }
            else {
                if (!currEntity.isEntityChild(targetEntity)) {
                    Set_ParentChild(Index, childId);
                }
            }
        }
        ImGui::EndDragDropTarget();
    }

   

    if (open) {
        for (int i = 0; i < currEntity.child.size(); ++i) {
            ObjectIndex childId = curr_scene.singleComponentsArrays.GetArray<Transform>().GetDenseIndex(*currEntity.child[i]);
            DisplayEntity(childId);
        }   
        ImGui::TreePop();
    }
}

void EditorHierarchy::Update(float dt) {
    ImGui::Begin("Hierarchy");

    //List out all entities in current scene
    //When clicked on, shows all children
    //Drag and drop of entities into and from other entities to form groups (using a node system, parent child relationship)
    //Add/Delete entities using right click
    Scene& curr_scene = SceneManager::Instance().GetCurrentScene();

    bool sceneopen = ImGui::TreeNodeEx(curr_scene.sceneName.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

    if(sceneopen){
        for (int i = 0; i < layer.size(); ++i) {
            if (!curr_scene.GetComponent<Transform>(*layer[i]).isChild()) {
                //Recursive function to display entities in a hierarchy tree
                DisplayEntity(layer[i]->denseIndex);
            }
        }

        ImGui::InvisibleButton("##", ImGui::GetContentRegionAvail());

        if (ImGui::BeginDragDropTarget()) {
            /*if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity")) {

                const ObjectIndex Index = *static_cast<ObjectIndex*>(payload->Data);

                Break_ParentChild(Index);
            }*/
            const ImGuiPayload* payload = ImGui::GetDragDropPayload();
            const ObjectIndex Index = *static_cast<ObjectIndex*>(payload->Data);

            Entity& currEntity = curr_scene.entities.DenseSubscript(Index);

            if (curr_scene.GetComponent<Transform>(currEntity).isChild()) {
                Break_ParentChild(Index);
            }
            else {
                auto it = std::find(layer.begin(), layer.end(), &currEntity);
                layer.erase(it);
                layer.insert(layer.end(), &currEntity);
            }


            ImGui::EndDragDropTarget();
        }

        //Right click adding of entities in hierarchy window
        if (ImGui::BeginPopupContextWindow(0, true)) {
            if (ImGui::MenuItem("Add Entity")) {
                selectedEntity = curr_scene.AddEntity().denseIndex;
            }

            std::string name = "Delete Entity";
            if (selectedEntity != NON_VALID_ENTITY) {                
                if (ImGui::MenuItem(name.c_str())) {
                    Entity& ent = curr_scene.entities.DenseSubscript(selectedEntity);
                    //Delete all children of selected entity as well
                    auto currEntity = curr_scene.GetComponent<Transform>(curr_scene.entities.DenseSubscript(selectedEntity));
                    for (auto child : currEntity.child) {
                        ObjectIndex id = curr_scene.singleComponentsArrays.GetArray<Transform>().GetDenseIndex(*child);
                        curr_scene.Destroy(child);
                    }
                    curr_scene.Destroy(ent);
                    auto it = std::find(layer.begin(), layer.end(), &ent);
                    EditorHierarchy::Instance().layer.erase(it);
                    selectedEntity = NON_VALID_ENTITY;
                }
            }
            else {
                ImGui::TextDisabled(name.c_str());
            }
            
            ImGui::EndPopup();
        }

        ImGui::TreePop();
    }     
    ImGui::End();
}

void EditorHierarchy::Exit() {

}