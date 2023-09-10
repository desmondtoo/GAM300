/*!***************************************************************************************
\file			YAMLUtils.cpp
\project		
\author         Sean Ngo

\par			Course: GAM300
\date           07/09/2023

\brief
    This file contains the definitions of the following:
    1. YAML Integration

All content � 2023 DigiPen Institute of Technology Singapore. All rights reserved.
******************************************************************************************/
#include "Precompiled.h"

#include "YAMLUtils.h"
#include "Scene/Components.h"


template <typename T>
void SerializeBasic(const T& _data, YAML::Emitter& out, const std::string& _key)
{
    out << YAML::Key << _key << YAML::Value << _data;
}

template<>
void SerializeBasic<bool>(const bool& _data, YAML::Emitter& out, const std::string& _key)
{

}

YAML::Emitter& operator<<(YAML::Emitter& out, const std::vector<Transform*>& _data)
{
    out << YAML::BeginSeq << YAML::BeginMap;

    for (auto& data : _data)
    {
        out << YAML::Flow << YAML::Key << "fileID" << YAML::Value << &data;
    }
    
    out << YAML::EndMap << YAML::EndSeq;
    return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
    return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
    return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
    return out;
}