/*!***************************************************************************************
\file			Serializer.h
\project		
\author         Sean Ngo

\par			Course: GAM300
\date           07/09/2023

\brief
    This file contains the declarations of the following:
    1. Scene Serialization
        a. Scene Settings
        b. Entity
        c. Components
    2. Scene Deserialization
        a. Scene Settings
        b. Entity
        c. Components
    3. Clone Helper for De/Serialization which copies fields for the script component

All content � 2023 DigiPen Institute of Technology Singapore. All rights reserved.
******************************************************************************************/
#ifndef SERIALIZER_H
#define SERIALIZER_H

#include "YAMLUtils.h"
#include "Properties.h"
#include "Scene/Scene.h"
#include <filesystem>

#include "Graphics/GraphicsHeaders.h"

// Bean: May be added in the future for modularity of serialization
//enum CLASSID
//{
//    OBJECT,
//    GAMEOBJECT,
//    COMPONENT
//};

// Deserializing components helper
struct DeComHelper
{
    YAML::Node* node;
    Scene* scene;
    bool linker;
};

// Serialize the scene
bool SerializeScene(Scene& _scene);

// Serialize the settings in the scene
bool SerializeSettings(YAML::Emitter& out, Scene& _scene);

// Serialize the entities in the specific scene
bool SerializeEntity(YAML::Emitter& out, Entity& _entity, Scene& _scene);

bool SerializePrefab(Entity& _entity, Scene& _scene);

// When serializing ids/pointers
template <typename T>
bool SerializeReferenceField(YAML::Emitter& out, T& _component);

// Serialize the components in the specific scene, the id parameter checks for serialization of components
// for the GameObject or for mainly the Component
template <typename T>
bool SerializeComponent(YAML::Emitter& out, T& _component);

// Serialization specifically for scripts
void SerializeScript(YAML::Emitter& out, Script& _component);

void Serialize(Material_instance& material, std::string directory = "Assets");

void Deserialize(Material_instance& material, const fs::path& path);

template <typename T>
void Serialize(YAML::Emitter& out, T& object)
{
    if constexpr (std::is_same<T, ModelImporter>())
    {
        out << YAML::BeginMap;
        out << YAML::Key << "guid" << YAML::Value << object.guid;
        out << object;
        out << YAML::EndMap;
    }
    else
    {
        property::SerializeEnum(object, [&](std::string_view PropertyName, property::data&& Data, const property::table&, std::size_t, property::flags::type Flags)
            {
                if (!Flags.m_isDontSave)
                {
                    auto entry = property::entry { PropertyName, Data };
                    fs::path name{ entry.first };
                    std::visit([&](auto& Value)
                        {
                            using T1 = std::decay_t<decltype(Value)>;
                            namespace fs = std::filesystem;
                            std::string keyName = name.string();
                            //Has a header
                            if (name.parent_path().empty())
                            {
                                //Deserialize again somehow get the type
                                //T1 object;
                            }
                            //Has no header
                            else
                            {
                                keyName.erase(keyName.begin(), keyName.begin() + keyName.find_last_of('/') + 1);
                            }

                            // Store Component value
                            out << YAML::BeginMap;
                            out << YAML::Key << keyName << YAML::Value << Value;
                            out << YAML::EndMap;
                        }
                    , entry.second);
                }
            });
    }
}

template <typename T>
void Serialize(const std::filesystem::path& path, T& object)
{
    YAML::Emitter out;

    Serialize(out, object);

    std::ofstream outFile{ path };
    outFile << out.c_str();
    outFile.close();
}

template <typename T>
bool Deserialize(const std::filesystem::path& path, T& object)
{
    namespace fs = std::filesystem;

    if (!std::filesystem::exists(path))
        return false;
    YAML::Node node = YAML::LoadFile(path.string());

    // Assign to the component
    property::SerializeEnum(object, [&](std::string_view PropertyName, property::data&& Data, const property::table&, std::size_t, property::flags::type)
        {
            auto entry = property::entry { PropertyName, Data };
            fs::path name{ entry.first };
            std::visit([&](auto& Value)
                {
                    using T1 = std::decay_t<decltype(Value)>;
                    namespace fs = std::filesystem;
                    std::string keyName = name.string();
                    //Has a header
                    if (name.parent_path().empty())
                    {
                        //Deserialize again somehow get the type
                        //T1 object;
                    }
                    //Has no header
                    else
                    {
                        keyName.erase(keyName.begin(), keyName.begin() + keyName.find_last_of('/') + 1);
                    }

                    // Extract Component value
                    if (node[keyName])
                    {
                        if constexpr (std::is_same<char*, T1>()) {
                            std::string buf = node[keyName].as<std::string>();
                            property::set(object, entry.first.c_str(), buf);

                        }
                        else
                            property::set(object, entry.first.c_str(), node[keyName].as<T1>());
                    }
                }
            , entry.second);
        });

    if constexpr (std::is_same<T, ModelImporter>())
    {
        if (node["meshes"])
        {
            YAML::Node meshes = node["meshes"];
            for (YAML::const_iterator it = meshes.begin(); it != meshes.end(); ++it) 
            {
                YAML::Node data = it->second;
                if (data["guid"])
                {
                    object.meshes.push_back(data["guid"].as<Engine::GUID>());
                }
            }
        }

        if (node["animations"])
        {
            YAML::Node animations = node["animations"];
            for (YAML::const_iterator it = animations.begin(); it != animations.end(); ++it)
            {
                YAML::Node data = it->second;
                if (data["guid"])
                {
                    object.animations.push_back(data["guid"].as<Engine::GUID>());
                }
            }
        }
        
    }
    else
    {
        return true;
    }
   
    return true;
}

// Deserialize the scene
bool DeserializeScene(Scene& _scene);

// Deserialize the scene setttings
bool DeserializeSettings(YAML::Node& _node, Scene& _scene);

// Deserialize the entities in the specific scene
void DeserializeEntity(YAML::Node& _node, Scene& _scene);

// Deserialize the components in the specific scene, the id parameter checks for deserialization of components
// for the GameObject or for mainly the Component
template <typename T>
void DeserializeComponent(const DeComHelper& _helper);

// After deserializing the scene, perform this function for linkage
void DeserializeLinker(Scene& _scene, const std::vector<YAML::Node>& _data);

// Helper function for serializing the script component
template <typename T, typename... Ts>
void SerializeScriptHelper(Field& rhs, YAML::Emitter& out);

// Helper function for serializing the script component
template <typename T, typename... Ts>
void SerializeScriptHelper(Field& rhs, YAML::Emitter& out, TemplatePack<T, Ts...>);

// Helper function for deserializing the script component
template <typename T, typename... Ts>
void DeserializeScriptHelper(Field& rhs, YAML::Node& node);

// Helper function for deserializing the script component
template <typename T, typename... Ts>
void DeserializeScriptHelper(Field& rhs, YAML::Node& node, TemplatePack<T, Ts...>);

// This struct contains functions which serializes components without using a switch case
template<typename T, typename... Ts>
struct SerializeComponentsStruct
{
public:
    // Default constructors
    constexpr SerializeComponentsStruct(TemplatePack<T, Ts...> pack) {}
    SerializeComponentsStruct() = default;
    
    // Serializing all components in the scene
    template<bool SerializeReference>
    bool SerializeComponents(YAML::Emitter& out, Entity& _entity, Scene& _scene)
    {
        return SerializeNext<SerializeReference,T, Ts...>(out, _entity, _scene);
    }
private:
    // Next component to serialize
    template<bool SerializeReference,typename T1, typename... T1s>
    bool SerializeNext(YAML::Emitter& out, Entity& _entity, Scene& _scene)
    {
        if constexpr (SingleComponentTypes::Has<T1>())
        {
            if (_scene.Has<T1>(_entity))
            {
                auto& component = _scene.Get<T1>(_entity);
                //skip deleted components 
                if (component.state == DELETED) return true;
                if constexpr (!std::is_same<T1, Tag>())
                {
                    if constexpr (SerializeReference)
                    {
                        if (!SerializeReferenceField(out, component))
                            return false;
                    }
                    else
                    {
                        if (!SerializeComponent(out, component))
                            return false;
                    }
                }
            }
        }
        else if constexpr (MultiComponentTypes::Has<T1>())
        {
            auto components = _scene.GetMulti<T1>(_entity);
            for (T1* component : components)
            {
                //skip deleted components 
                if (component->state == DELETED)  return true;
                if constexpr (SerializeReference)
                {
                    if (!SerializeReferenceField(out, *component))
                        return false;
                }
                else
                {
                    if (!SerializeComponent(out, *component))
                        return false;
                }
            }
        }

        if constexpr (sizeof...(T1s) != 0)
        {
            if (!SerializeNext<SerializeReference,T1s...>(out, _entity, _scene))
                return false;
        }

        return true;
    }
};

using ComponentsSerializer = decltype(SerializeComponentsStruct(AllComponentTypes()));

#endif // !SERIALIZER_H