/*!***************************************************************************************
\file			EditorScene.cpp
\project		GAM300
\author			Sean Ngo

\par			Course : GAM300
\date           04/09/2023

\brief
	This file contains the definitions of the following :
	1. Editor Scene

All content � 2023 DigiPen Institute of Technology Singapore.All rights reserved.
* *****************************************************************************************/
#include "Precompiled.h"

#include "EditorHeaders.h"
#include "ImGuizmo.h"
#include "Scene/SceneManager.h"
#include "../Graphics/Editor_Camera.h"

// Bean: Need this to reference the editor camera's framebuffer

static int GizmoType = 0;


void EditorScene::Init()
{
    sceneDimension = glm::vec2(1600.f, 900.f);
    scenePosition = glm::vec2(0.f, 0.f);
}

void EditorScene::Update(float dt)
{
    if (ImGui::Begin("Scene"))
    {
        scenePosition = glm::vec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
        unsigned int textureID = EditorCam.getFramebuffer().get_color_attachment_id();
        ImVec2 viewportEditorSize = ImGui::GetContentRegionAvail();
        glm::vec2 _newDimension = *((glm::vec2*)&viewportEditorSize);

        // Only if the current scene dimension is not the same as new dimension
        if (sceneDimension != _newDimension && _newDimension.x != 0 && _newDimension.y != 0)
        {
            sceneDimension = { _newDimension.x, _newDimension.y };
            EditorCam.onResize(sceneDimension.x, sceneDimension.y);

            EditorCam.getFramebuffer().resize(sceneDimension.x, sceneDimension.y);
        }
        ImGui::Image((void*)(size_t)textureID, ImVec2{ (float)sceneDimension.x, (float)sceneDimension.y }, ImVec2{ 0 , 1 }, ImVec2{ 1 , 0 });

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();	
        float windowWidth = (float)ImGui::GetWindowWidth();
        float windowHeight = (float)ImGui::GetWindowHeight();
        std::cout << "width : " << windowWidth << "\n";
        std::cout << "height : " << windowHeight << "\n";

        // Might be wrong
        ImGuizmo::SetRect((float)ImGui::GetWindowPos().x, (float)ImGui::GetWindowPos().y, windowWidth, windowHeight);

        if (EditorCam.ActiveObj != nullptr)
        {
            Scene& currentScene = SceneManager::Instance().GetCurrentScene();

            Transform& trans = currentScene.singleComponentsArrays.GetArray<Transform>().DenseSubscript(EditorCam.ActiveObj->denseIndex);

            GizmoType = ImGuizmo::TRANSLATE;

            glm::vec4 translate = { trans.translation,0.f };
            glm::vec4 rotation = { trans.rotation,0.f };
            glm::vec4 scale = { trans.scale,0.f };

            glm::mat4 transform_1;
            glm::mat4 transform_2;


            ImGuizmo::RecomposeMatrixFromComponents(glm::value_ptr(translate),
                glm::value_ptr(rotation), glm::value_ptr(scale), glm::value_ptr(transform_1));


            ImGuizmo::Manipulate(glm::value_ptr(EditorCam.getViewMatrix()), glm::value_ptr(EditorCam.getPerspMatrix()),
                (ImGuizmo::OPERATION)GizmoType, ImGuizmo::WORLD, glm::value_ptr(transform_1));


            if (ImGuizmo::IsUsing())
            {
                glm::vec4 After_Translate;
                glm::vec4 After_Scale;
                glm::vec4 After_Rotation;
                ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform_1), glm::value_ptr(After_Translate),
                    glm::value_ptr(After_Rotation), glm::value_ptr(After_Scale));

              
                    //translate_after.x - tc.position.x;
                    //tc.localPosition = Orion::Math::Vec3(translate_after.x, translate_after.y, tc.position.z);
                    //tc.localPosition = Orion::Math::Vec3(translate_after.x - tc.position.x, translate_after.y - tc.position.y, tc.position.z);
                    //tc.localPosition += Orion::Math::Vec3(translate_after.x - tc.position.x, translate_after.y - tc.position.y, 0);

                trans.translation = After_Translate;
                trans.rotation = After_Rotation;
                trans.scale = After_Scale;

                    //if (m_GizmoType == ImGuizmo::ROTATE) {}
                    //else
                    //{
                    //    tc.localScale += Orion::Math::Vec3(scale_after.x - tc.scale.x, scale_after.y - tc.scale.y, 0.f);
                    //}

                    //tc.localRotation += Orion::Math::Vec3(0, 0, rotation_after.z - tc.rotation.z);
             
            }


        }


    }
    ImGui::End();
}

void EditorScene::Exit()
{

}