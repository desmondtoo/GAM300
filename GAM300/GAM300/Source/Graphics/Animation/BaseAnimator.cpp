/*!***************************************************************************************
\file			BaseAnimator.cpp
\project
\author         Theophelia Tan

\par			Course: GAM300
\date           23/10/2023

\brief
    This file contains the definitions of the following:
    1.

All content � 2023 DigiPen Institute of Technology Singapore. All rights reserved.
******************************************************************************************/
#include "Precompiled.h"
#include "BaseAnimator.h"
#include "AnimationManager.h"
#include "Scene/Components.h"
#include "Scene/SceneManager.h"
#include "Editor/EditorHistory.h"
#include "Editor/Editor.h"

BaseAnimator::BaseAnimator()
{
    m_CurrentTime = 0.0f;
    startTime = 0.0f;
    endTime = 0.0f;
    m_AnimationIdx = -1;
    m_FinalBoneMatIdx = -1;
    speedModifier = 1.f;
    currentState = nextState = defaultState = nullptr;
    playing = false;
    currBlendState = notblending;
    blendedBones = 0;
    blendDuration = 0.1f;
    blendStartTime = 0.f;
    blendTimer = 0.f;

    m_FinalBoneMatrices.reserve(100);
    for (int i = 0; i < m_FinalBoneMatrices.capacity(); i++)
        m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
}

void BaseAnimator::CreateRig(Transform* _transform)
{
    rigTransform = _transform;
    Scene& currentScene = MySceneManager.GetCurrentScene();

    // If no aniamtion exists
    if (m_AnimationIdx == -1)
    {
        // Remove all existing rig
        if (!rig.empty()) // If the root transform still exists within the scene
        {
            Change newchange;
            newchange.entity = &currentScene.Get<Entity>(*(*rig.begin()).second);
            EDITOR.History.AddEntityChange(newchange);

            for (auto& [name, transform] : rig)
            {
                if (transform->GetParent() != nullptr)
                {
                    Transform* parent = transform->GetParent();
                    parent->RemoveChild(transform);
                }
                currentScene.Destroy(transform);
            }
        }
        
        rigTransform = nullptr;
        rig.clear();
    }
    else
    {
        Animation& m_CurrentAnimation = AnimationManager.GetAnimCopy(m_AnimationIdx);
        
        // Check for existing rig
        for (size_t i = 0; i < rigTransform->child.size(); i++)
        {
            Tag& tag = currentScene.Get<Tag>(rigTransform->child[i]);
            Bone* bone = m_CurrentAnimation.FindBone(tag.name);

            if (bone)
            {
                // If there is a pre-existing bone in the animator, re-assign the rig base on the transform
                rig.clear();
                ReAssignRig(&currentScene.Get<Transform>(rigTransform->child[i]));
                return;
            }
        }

        CalculateRigTransform(m_CurrentAnimation, &m_CurrentAnimation.GetRootNode(), rigTransform);
    }
}

void BaseAnimator::ReAssignRig(Transform* _transform)
{
    if (!_transform)
        return;

    Scene& currentScene = MySceneManager.GetCurrentScene();

    std::string name = currentScene.Get<Tag>(*_transform).name;
    rig[name] = _transform;

    for (size_t i = 0; i < _transform->child.size(); i++)
    {
        ReAssignRig(&currentScene.Get<Transform>(_transform->child[i]));
    }
}

void BaseAnimator::CalculateRigTransform(Animation& animation, const AssimpNodeData* node, Transform* parentTransform)
{
    const std::string& nodeName = node->name;

    Scene& currentScene = MySceneManager.GetCurrentScene();
    Animation& m_CurrentAnimation = animation;
    Transform* transform = nullptr;

    Bone* Bone = m_CurrentAnimation.FindBone(nodeName);
    //PRINT(nodeName, "\n");

    if (Bone)
    {
        Bone->Update(m_CurrentTime);
    }

    auto& boneInfoMap = m_CurrentAnimation.GetBoneInfoMap();
    auto it = boneInfoMap.find(nodeName);
    if (it != boneInfoMap.end())
    {
        // Set Rig Display
        Entity* entity = currentScene.Add<Entity>();
        transform = &currentScene.Get<Transform>(*entity);
        Tag& tag = currentScene.Get<Tag>(*entity);
        tag.name = nodeName;
        transform->SetParent(parentTransform);
        Vector3 pos, scale, rot2;
        glm::quat rot;
        glm::mat4 temp = Bone->GetLocalTransform();
        transform->Decompose(temp, pos, rot, scale);
        transform->SetLocalMatrix(pos, glm::eulerAngles(rot), scale);
        rig[nodeName] = transform;

        rot2 = glm::degrees(glm::eulerAngles(rot));
        /*PRINT("Comp Trans: ", pos.x, " ", pos.y, " ", pos.z);
        PRINT(" : ", rot2.x, " ", rot2.y, " ", rot2.z);
        PRINT(" : ", scale.x, " ", scale.y, " ", scale.z, "\n");*/
    }

    if (!transform)
        transform = parentTransform;

    for (int i = 0; i < node->childrenCount; i++)
    {
        CalculateRigTransform(animation, &node->children[i], transform);
    }
}

void BaseAnimator::UpdateAnimation(float dt, glm::mat4& pTransform)
{
    if (dt > 1.f)
        dt = 0.016f;
    Animation& m_CurrentAnimation = AnimationManager.GetAnimCopy(m_AnimationIdx);
    UpdateStateName();

    m_CurrentTime += (m_CurrentAnimation.GetTicksPerSecond() * dt * speedModifier) - startTime;
    //std::cout << "Initial: " << m_CurrentTime << "\n";

    // Animation has ended and is not blending
    float newBlendDuration = blendDuration * m_CurrentAnimation.GetTicksPerSecond();
    if (m_CurrentTime >= endTime - startTime - newBlendDuration && currBlendState != blending)
    {
        if (!defaultState)
            return;

        if (!currentState) // One time trigger when user first run the update function
        {
            currentState = defaultState;
            startTime = currentState->minMax.x;
            endTime = currentState->minMax.y;
        }

        // No next state, set it to default state
        if (!nextState)
            nextState = defaultState;

        // Different states
        if (currentState != nextState)
        {
            currBlendState = blending;
            //endTime += newBlendDuration;
            blendStartTime = m_CurrentTime;
        }
        else
        {
            currBlendState = notblending;
        }
    }

    m_CurrentTime = fmod(m_CurrentTime, endTime - startTime);
    m_CurrentTime += startTime; // wrap within the time range then offset by the start time 

    //std::cout << "End: " << m_CurrentTime << "\n";
    if (currBlendState == blending)/*if (nextState)*/
    {
        blendTimer += (m_CurrentAnimation.GetTicksPerSecond() * dt * speedModifier);
        blendedBones = 0;
        CalculateBlendedBoneTransform(&m_CurrentAnimation.GetRootNode(), glm::mat4(1.f));
        
        float newBlendDuration = blendDuration * m_CurrentAnimation.GetTicksPerSecond();
        if (blendedBones == m_CurrentAnimation.GetBoneCount() && blendTimer >= newBlendDuration)
        {
            //std::cout << "Finished blending " << currentState->label << " and " << nextState->label << '\n';
            //std::cout << "Bone count: " << blendedBones << " and timer: " << blendTimer << '\n';
            currBlendState = blended;
            currentState = nextState;
            nextState = nullptr;

            blendTimer = 0.f;
            blendedBones = 0;
            startTime = currentState->minMax.x;
            m_CurrentTime = startTime + newBlendDuration;
            endTime = currentState->minMax.y;
            stateName = currentState->label;
            playing = true;
        }
    }
    else
        CalculateBoneTransform(&m_CurrentAnimation.GetRootNode(), glm::mat4(1.f));
}

void BaseAnimator::PlayAnimation(Animation* pAnimation)
{
    Animation& m_CurrentAnimation = AnimationManager.GetAnimCopy(m_AnimationIdx);
    m_CurrentAnimation = *pAnimation;
    m_CurrentTime = 0.0f;
}

void BaseAnimator::ChangeState()
{
    // If there is a next state and the two states are different
    //if (nextState && currentState != nextState)
    //{
    //    currBlendState = blending;
    //    //endTime = m_CurrentTime + blendDuration;
    //    blendStartTime = m_CurrentTime;
    //}
    //else // If both states are the same
    //{
    //    currBlendState = notBlending;
    //}

    //if (!currentState && defaultState) // If no next state, use default state
    //{
    //    currentState = defaultState;

    //    m_CurrentTime = startTime = currentState->minMax.x;
    //    endTime = currentState->minMax.y;
    //    blendStartTime = m_CurrentTime;
    //}

    //if (!nextState)
    //    nextState = defaultState;

    //// Check that the current state exists
    //if (currentState)
    //{
    //    playing = true;
    //}
    //else
    //{
    //    startTime = endTime = 0.f;
    //    playing = false;
    //}

    if (nextState && currentState != nextState) // E.g. Idle is not equal to Jump state and Jump state exists
    {
        currBlendState = blending;
        //std::cout << "Change state Different state blending " << currentState->label << " and " << nextState->label << '\n';
        //endTime = m_CurrentTime;
        blendStartTime = m_CurrentTime;
    }
    
    if (!currentState && defaultState) // If no next state, use default state
    {
        currentState = defaultState;
        m_CurrentTime = startTime = currentState->minMax.x;
        endTime = currentState->minMax.y;
        currBlendState = notblending;
    }

    //if(!nextState)
    //    nextState = defaultState;

    // Check that the current state exists
    if(!currentState)
    {
        startTime = endTime = 0.f;
        playing = false;
    }
    else
    {
        playing = true;
    }
}

void BaseAnimator::CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
{
    std::string nodeName = node->name;

    Animation& m_CurrentAnimation = AnimationManager.GetAnimCopy(m_AnimationIdx);

    Bone* Bone = m_CurrentAnimation.FindBone(nodeName);

    if (Bone)
    {
        Bone->Update(m_CurrentTime);
        parentTransform *= Bone->GetLocalTransform();
    }
    else
    {
        parentTransform *= node->transformation;
    }

    auto& boneInfoMap = m_CurrentAnimation.GetBoneInfoMap();
    auto it = boneInfoMap.find(nodeName);
    if (it != boneInfoMap.end())
    {
        int index = it->second.id;
        glm::mat4 offset = it->second.offset;
        m_FinalBoneMatrices[index] = parentTransform * offset;

        // Set Rig Display
        if (armature && !rig.empty())
        {
            Transform* transform = rig[nodeName];
            if (transform)
            {
                Vector3 pos, scale;
                glm::quat rot;
                glm::mat4 temp = Bone->GetLocalTransform();
                transform->Decompose(temp, pos, rot, scale);
                transform->SetLocalMatrix(pos, glm::eulerAngles(rot), scale);
                //PRINT(nodeName, " :", transform->GetLocalTranslation().x, " ", transform->GetLocalTranslation().y, " ", transform->GetLocalTranslation().z, "\n");
            }
        }
    }

    for (int i = 0; i < node->childrenCount; i++)
        CalculateBoneTransform(&node->children[i], parentTransform);
}

void BaseAnimator::CalculateBlendedBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
{
    if (nextState)
    {
        std::string nodeName = node->name;
        glm::mat4 nodeTransform = node->transformation;

        Animation& m_CurrentAnimation = AnimationManager.GetAnimCopy(m_AnimationIdx);
        Animation& m_NextAnimation = AnimationManager.GetAnimCopy(m_AnimationIdx);

        Bone* NextBone = m_NextAnimation.FindBone(nodeName);
        Bone* Bone = m_CurrentAnimation.FindBone(nodeName);

        if (Bone && NextBone)
        {
            // get anim1 xform
            int p0Index = Bone->GetPositionIndex(blendStartTime + blendTimer);
            // get anim2 xform
            int p1Index = NextBone->GetPositionIndex(nextState->minMax.x + blendTimer);

            float newBlendDuration = blendDuration * m_CurrentAnimation.GetTicksPerSecond();

            // blend factor
            //float blendFactor = Bone->GetBlendFactor(Bone->GetTimeStamp(p0Index),
            //    newBlendDuration, m_CurrentTime);

            float blendFactor = blendTimer / ((newBlendDuration == 0.f) ? 1.f : newBlendDuration);
            //std::cout << "Blend Factor: " << blendFactor << " with time: " << blendTimer << " and duration: " << newBlendDuration << "\n";

            if (blendFactor >= 1.f)
            {
                ++blendedBones;
            }

            glm::vec3 finalPosition, finalScale;
            glm::quat finalRotation;

            finalPosition = glm::mix(Bone->m_Positions[p0Index].position, NextBone->m_Positions[p1Index].position, blendFactor);
            finalRotation = glm::slerp(Bone->m_Rotations[p0Index].orientation, NextBone->m_Rotations[p1Index].orientation, blendFactor);
            finalScale = glm::mix(Bone->m_Scales[p0Index].scale, NextBone->m_Scales[p1Index].scale, blendFactor);

            finalRotation = glm::normalize(finalRotation);

            glm::mat4 translation = glm::translate(glm::mat4(1.0f), finalPosition);
            glm::mat4 rotation = glm::toMat4(finalRotation);
            glm::mat4 scale = glm::scale(glm::mat4(1.0f), finalScale);

            //set nodexform to blended one
            //nodeTransform = NextBone->GetLocalTransform(); // temp
            nodeTransform = translation * rotation * scale; // temp
        }

        glm::mat4 globalTransformation = parentTransform * nodeTransform;

        auto boneInfoMap = m_CurrentAnimation.GetBoneInfoMap();
        if (boneInfoMap.find(nodeName) != boneInfoMap.end())
        {
            int index = boneInfoMap[nodeName].id;
            glm::mat4 offset = boneInfoMap[nodeName].offset;
            m_FinalBoneMatrices[index] = globalTransformation * offset;

            // Set Rig Display
            if (armature && !rig.empty())
            {
                Transform* transform = rig[nodeName];
                if (transform)
                {
                    Vector3 pos, scale;
                    glm::quat rot;
                    glm::mat4 temp = Bone->GetLocalTransform();
                    transform->Decompose(temp, pos, rot, scale);
                    transform->SetLocalMatrix(pos, glm::eulerAngles(rot), scale);
                }
            }
        }

        for (int i = 0; i < node->childrenCount; i++)
            CalculateBlendedBoneTransform(&node->children[i], globalTransformation);
    }
    else
    {
        currBlendState = blended;
    }
}

void BaseAnimator::SetDefaultState(const std::string& _defaultState)
{
    Animation& animation = AnimationManager.GetAnimCopy(m_AnimationIdx);
    defaultState = animation.GetAnimationState(_defaultState);
    CalculateBoneTransform(&animation.GetRootNode(), glm::mat4(1.f));
}

void BaseAnimator::SetNextState(const std::string& _nextState)
{
    // Is not the same as next state
    if (nextState == nullptr || nextState->label.compare(_nextState))
    {
        Animation& animation = AnimationManager.GetAnimCopy(m_AnimationIdx);
        nextState = animation.GetAnimationState(_nextState);
        stateNextName = _nextState;
    }
}

void BaseAnimator::SetState(const std::string& _state)
{
    SetNextState(_state);
    if(currBlendState != blending)
        ChangeState();
}

void BaseAnimator::UpdateStateName()
{
    if(currentState)
        stateName = currentState->label;
    else
        stateName = "None";

    if (nextState)
        stateNextName = nextState->label;
    else
        stateNextName = "None";
}

bool BaseAnimator::AnimationAttached() {
    // Check if m_CurrentAnimation is not nullptr (i.e., it's attached)
    return m_AnimationIdx != -1 && animID != 0;
}


//void BaseAnimator::CalculateBlendFactor(float transitionDuration)
//{
//    // Calculate the blend factor as the ratio of elapsed time to transition duration
//    blendFactor = glm::clamp( (m_CurrentTime - startTime) / transitionDuration, 0.0f, 1.0f);
//}

//glm::mat4 lerp(const glm::mat4& a, const glm::mat4& b, float t) {
//    t = glm::clamp(t, 0.0f, 1.0f); // Ensure t is in the [0, 1] range
//    return a + (b - a) * t;
//}
//void BaseAnimator::InterpolateAnimations(Animation& firstAnimation, Animation& secondAnimation)
//{
//    // Ensure that both animations have the same number of bones
//    if (firstAnimation.GetBoneCount() != secondAnimation.GetBoneCount())
//    {
//        // Handle error: Animations must have the same number of bones for blending
//        return;
//    }
//
//    // Interpolate between the bone transformations of the two animations based on the blend factor
//    for (int i = 0; i < firstAnimation.GetBoneCount(); ++i)
//    {
//        // Get bone transformations from the two animations
//        glm::mat4 firstTransform = firstAnimation.GetBones()[i].GetLocalTransform();
//        glm::mat4 secondTransform = secondAnimation.GetBones()[i].GetLocalTransform();
//
//        // Interpolate between the two transformations
//        glm::mat4 interpolatedTransform = lerp(firstTransform, secondTransform, blendFactor);
//
//        // change existing finalbone mat instead of making new anim!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
//        // Update the blended animation with the interpolated transformation
//        m_BlendedAnimation.GetBones()[i].m_LocalTransform = interpolatedTransform;
//    }
//}