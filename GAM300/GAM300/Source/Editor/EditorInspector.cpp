﻿/*!***************************************************************************************
\file			EditorInspector.cpp
\project
\author         Joseph Ho
\coauthor       Zachary Hong

\par			Course: GAM300
\date           07/09/2023

\brief
    This file contains the definitions of the functions that renders the inspector window in
    Editor. These functionalities include:
    1. Displaying Components of the selected entity
    2. Display the individual types and fields in the editor

All content © 2023 DigiPen Institute of Technology Singapore. All rights reserved.
******************************************************************************************/

#include "Precompiled.h"

#include "Editor.h"
#include "EditorHeaders.h"
#include "Scene/Scene.h"
#include "Scene/SceneManager.h"
#include "EditorTemplates.h"
#include "Scene/Components.h"
#include "Graphics/MeshManager.h"
#include <variant>
#include "PropertyConfig.h"
#include "Utilities./ThreadPool.h"

#define BUTTON_HEIGHT .1 //Percent
#define BUTTON_WIDTH .6 //Percent
#define TEXT_BUFFER_SIZE 2048

//Flags for inspector headers/windows
static ImGuiTableFlags windowFlags =
ImGuiTableFlags_Resizable |
ImGuiTableFlags_NoBordersInBody |
ImGuiTableFlags_NoSavedSettings |
ImGuiTableFlags_SizingStretchProp;

bool isAddingReference = false;
size_t editedContainer{};

template <typename T>
void Display(const char* name, T& val);

// DisplayType contains overloads that display the respective fields based on the type passed into the function
template <typename T>
void DisplayType(const char* name, T& val)
{
    UNREFERENCED_PARAMETER(name);
    UNREFERENCED_PARAMETER(val);
}

void DisplayType(const char* name, bool& val)
{
    static std::string idName{};
    idName = "##";
    idName += name;
    ImGui::Checkbox(idName.c_str(), &val);
}

void DisplayType(const char* name, std::string& val)
{
    static std::string idName{};
    idName = "##";
    idName += name;
    static char buffer[2048];
    strcpy_s(buffer, val.c_str());
    ImGui::InputText(idName.c_str(), buffer,2048);
}

void DisplayType(const char* name, int& val)
{
    static std::string idName{};
    idName = "##";
    idName += name;
    ImGui::DragInt(idName.c_str(), &val);
}

template <size_t SZ>
void DisplayType(const char* name, char(&val)[SZ])
{
    static std::string idName{};
    idName = "##";
    idName += name;
    ImGui::InputTextMultiline(idName.c_str(), val, SZ, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16));
}

void DisplayType(const char* name, char*& val)
{
    static std::string idName{};
    idName = "##";
    idName += name;
    ImGui::InputTextMultiline(idName.c_str(), val, TEXT_BUFFER_SIZE, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16));
}

void DisplayType(const char* name, float& val)
{
    static float temp{};
    static std::string idName{};
    const char* cIdName{};
    if (name[0] == '#' && name[1] == '#')
    {
        cIdName = name;
    }
    else
    {
        idName = "##";
        idName += name;
        cIdName = idName.c_str();
    }
    ImGui::DragFloat(cIdName, &val, 0.15f);
}

void DisplayType(const char* name, double& val)
{
    static std::string idName{};
    idName = "##";
    idName += name;
    float temp{ (float)val };
    ImGui::DragFloat(idName.c_str(), &temp, 0.15f);
    val = temp;
}

void DisplayType(const char* name, Vector3& val)
{
    static float temp{};
    static std::string idName{};
    idName = "##";
    idName += name;

    if (ImGui::BeginTable("Vector3", 3, windowFlags))
    {
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        idName += 'X';
        ImGui::Text("X"); ImGui::SameLine(); ImGui::SetNextItemWidth(-FLT_MIN);
        DisplayType(idName.c_str(), val.x);

        ImGui::TableNextColumn();
        idName.back() = 'Y';
        ImGui::Text("Y"); ImGui::SameLine(); ImGui::SetNextItemWidth(-FLT_MIN);
        DisplayType(idName.c_str(), val.y);

        ImGui::TableNextColumn();
        idName.back() = 'Z';
        ImGui::Text("Z"); ImGui::SameLine(); ImGui::SetNextItemWidth(-FLT_MIN);
        DisplayType(idName.c_str(), val.z);
        ImGui::EndTable();
    }
}

void DisplayType(const char* name, Vector4& val)
{
    static float temp{};
    static std::string idName{};
    idName = "##";
    idName += name;
    
    ImGui::ColorEdit4("MyColor##4", (float*)&val, ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_DisplayHSV);
}

void DisplayType(const char* name, Vector2& val)
{
    static float temp{};
    static std::string idName{};
    idName = "##";
    idName += name;
    if (ImGui::BeginTable("Vector2", 2, windowFlags))
    {
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        idName += 'X';
        ImGui::Text("X"); ImGui::SameLine(); ImGui::SetNextItemWidth(-FLT_MIN);
        DisplayType(idName.c_str(), val.x);

        ImGui::TableNextColumn();
        idName.back() = 'Y';
        ImGui::Text("Y"); ImGui::SameLine(); ImGui::SetNextItemWidth(-FLT_MIN);
        DisplayType(idName.c_str(), val.y);

        ImGui::EndTable();
    }
}


template <typename T>
void AddReferencePanel(T*& container)
{
    Scene& scene = MySceneManager.GetCurrentScene();
    static ImGuiTextFilter filter;
    static std::string windowName;
    windowName = "Add ";
    windowName += GetType::Name<T>();
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(100.f, 100.f), ImGuiCond_FirstUseEver);
    windowName += " Reference";
    bool open = true;
    if (ImGui::Begin(windowName.c_str(), &open))
    {
        ImGui::PushItemWidth(-1);
        filter.Draw("##References");
        ImGui::PopItemWidth();
        static std::string buttonName{};
        ImVec2 buttonSize = ImGui::GetWindowSize();
        buttonSize.y *= (float)BUTTON_HEIGHT;
        if (ImGui::Button("None", buttonSize))
        {
            editedContainer = 0;
            container = nullptr;
        }
        for (T& object : scene.GetArray<T>())
        {
            Tag& tag = scene.Get<Tag>(object);
            buttonName = tag.name;
            if constexpr (std::is_same_v<T, Entity>)
            {
                ImGui::PushID((int)object.EUID());
            }
            else
            {
                ImGui::PushID((int)object.UUID());
            }
            if (filter.PassFilter(tag.name.c_str()) && ImGui::Button(buttonName.c_str(), buttonSize))
            {
                editedContainer = 0;
                container = &object;
            }
            ImGui::PopID();
        }
        ImGui::End();
    }
    if (!open)
        editedContainer = 0;
}


GENERIC_RECURSIVE
(
    void, 
    AddReferencePanel, 
    AddReferencePanel(((Field*)pObject)->Get<T*>())
)

template <typename T>
void DisplayType(const char* name, T*& container, const char* altName = nullptr)
{
    UNREFERENCED_PARAMETER(name);
    if constexpr (AllObjectTypes::Has<T>())
    {
        static std::string btnName;
        if (container)
        {
            btnName = MySceneManager.GetCurrentScene().Get<Tag>(container->EUID()).name;
        }
        else
        {
            btnName = "None";
        }
        btnName += "("; 
        if constexpr (std::is_same_v<T,Entity>)
        {
            btnName += "GameObject";
        }
        else
        {
            if (altName)
            {
                btnName += altName;
            }
            else
            {
                btnName += GetType::Name<T>();
            }
        }
        btnName += ")";
        ImGui::Button(btnName.c_str(), ImVec2(-FLT_MIN, 0.f));
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            isAddingReference = true;
        }
        if (ImGui::BeginDragDropTarget())
        {
            const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(GetType::Name<T>());
            if (payload)
            {
                Handle handle = *(Handle*)payload->Data;
                container = &MySceneManager.GetCurrentScene().Get<T>(handle);
            }
            ImGui::EndDragDropTarget();
        }
    }
}

template <typename T, typename... Ts>
void DisplayField(const char* name, Field& field)
{
    if (GetFieldType::E<T>() == field.fType)
    {
        if (field.fType < AllObjectTypes::Size())
        {
            T*& value = *reinterpret_cast<T**>(field.data);
            if constexpr (std::is_same<T, Script>())
            {
                DisplayType(name, value,field.typeName.c_str());
            }
            else
            {
                DisplayType(name, value);
            }
        }
        else
        {
            DisplayType(name, field.Get<T>());
        }
        return;
    }
    if constexpr (sizeof...(Ts) != 0)
    {
        DisplayField<Ts...>(name, field);
    }
}

template <typename T, typename... Ts>
void DisplayField(const char* name, Field& field, TemplatePack<T,Ts...>)
{
    DisplayField<T,Ts...>(name,field);
}

void DisplayType(const char* name, Field& val)
{
    DisplayField(name, val, AllFieldTypes());
}

template <typename T>
void Display(const char* name, T& val)
{
    ImGui::AlignTextToFramePadding();
    ImGui::TableNextColumn();
    ImGui::Text(name);
    ImGui::TableNextColumn();
    DisplayType(name, val);
}

void Display(const char* string)
{
    ImGui::TableNextColumn();
    ImGui::AlignTextToFramePadding();
    ImGui::Text(string);
}

//Function to display and edit textures of a given property.
template <typename T> 
void DisplayTexturePicker(T& Value) {

    using T1 = std::decay_t<decltype(Value)>;

    ImGui::SameLine();
    ImGuiWindowFlags win_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar;
    static const std::string AssetDirectory = "Assets";
    static std::filesystem::path CurrentDirectory = AssetDirectory;
    static std::string currentFolder = "Assets";


    if (ImGui::Button("Edit")) {
        ImGui::OpenPopup("Texture");
    }

    //Component Settings window
    ImGui::SetNextWindowSize(ImVec2(250.f, 300.f));

    if (ImGui::BeginPopup("Texture", win_flags)) {
        ImGui::Text("Current Folder: %s", currentFolder.c_str()); ImGui::Spacing();
        // Back button to return to parent directory
        if (CurrentDirectory != std::filesystem::path(AssetDirectory))
        {
            if (ImGui::Button("Back", ImVec2{ 50.f, 30.f }))
            {
                CurrentDirectory = CurrentDirectory.parent_path();
                currentFolder = CurrentDirectory.string();
            }
        }

        static float padding = 15.f;
        static float iconsize = 50.f;
        float cellsize = iconsize + padding;

        float window_width = ImGui::GetContentRegionAvail().x;
        int columncount = (int)(window_width / cellsize);
        if (columncount < 1) { columncount = 1; }

        ImGui::Columns(columncount, 0, false);

        //remove texture icon
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0, 0, 0, 0 });
        size_t id = (size_t)GET_TEXTURE_ID("Cancel_Icon");
        if (ImGui::ImageButton((ImTextureID)id, { iconsize, iconsize }, { 0 , 1 }, { 1 , 0 })) {

            if constexpr (std::is_same<T, std::string>()) {
                Value = "";
            }
            
            ImGui::PopStyleColor();
            ImGui::EndPopup(); 
            ImGui::CloseCurrentPopup();
            return;
        };
        ImGui::PopStyleColor();
        //render file name below icon
        ImGui::TextWrapped("Remove Texture");
        ImGui::NextColumn();

        int i = 0;
        //using filesystem to iterate through all folders/files inside the "/Data" directory
        for (auto& it : std::filesystem::directory_iterator{ CurrentDirectory })
        {
            const auto& path = it.path();
            //if not png or dds file, dont show
            if ((path.string().find("meta") != std::string::npos)) continue;

            ImGui::PushID(i++);

            auto relativepath = std::filesystem::relative(path, AssetDirectory);
            std::string pathStr = relativepath.filename().string();

            //Draw the file / folder icon based on whether it is a directory or not
            std::string icon = it.is_directory() ? "foldericon" : "fileicon";

            size_t icon_id = 0;

            /*auto it2 = filename.begin() + filename.find_first_of(".");
            filename.erase(it2, filename.end());*/

            std::string filename = "";

            if (!it.is_directory()) {

                filename = relativepath.string();

                auto it2 = filename.begin();

                if (filename.find_last_of("\\") != std::string::npos) {
                    it2 = filename.begin() + filename.find_last_of("\\") + 1;
                    filename.erase(filename.begin(), it2);
                }
                if (filename.find_first_of(".") != std::string::npos) {
                    it2 = filename.begin() + filename.find_first_of(".");
                    filename.erase(it2, filename.end());
                }
                //PRINT(filename);
                auto tex = GET_TEXTURE_ID(filename);

                if (tex != UINT_MAX) {
                    icon = filename;
                }
            }

            //render respective file icon textures
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0, 0, 0, 0 });
            icon_id = GET_TEXTURE_ID(icon);
            ImGui::ImageButton((ImTextureID)icon_id, { iconsize, iconsize }, { 0 , 0 }, { 1 , 1 });

            ImGui::PopStyleColor();

            //Change directory into the folder clicked
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                if (it.is_directory())
                {
                    currentFolder = pathStr;
                    CurrentDirectory /= path.filename();
                }
            }

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                //update texture
                if constexpr (std::is_same<T, std::string>()) {
                    Value = icon;
                }
                
            }

            //render file name below icon
            ImGui::TextWrapped(pathStr.c_str());
            ImGui::NextColumn();
            ImGui::PopID();
        }
        ImGui::Columns(1);
        ImGui::EndPopup();
    }
}

template <typename T>
void DisplayLightTypes(T& value) {
    if constexpr (std::is_same<T, int>()) {
        ImGui::AlignTextToFramePadding();
        ImGui::TableNextColumn();
        ImGui::Text("Type");
        ImGui::TableNextColumn();

        Engine::UUID curr_index = EditorHierarchy::Instance().selectedEntity;
        Scene& curr_scene = SceneManager::Instance().GetCurrentScene();
        Entity& curr_entity = curr_scene.Get<Entity>(curr_index);

        std::vector<const char*> layers;
        layers.push_back("Spot"); layers.push_back("Directional"); layers.push_back("Point");
        static int index = value;
        ImGui::PushItemWidth(100.f);
        ImGui::Combo("##LightType", &index, layers.data(), (int)layers.size(), 5);
        ImGui::PopItemWidth();
        value = index;
    }
}

//Displays all the properties of an given entity
template <typename T>
void Display_Property(T& comp) {
    if constexpr (std::is_same<T, MeshRenderer>()) {

        //Combo field for mesh renderer
        ImGui::AlignTextToFramePadding();
        ImGui::TableNextColumn();
        ImGui::Text("MeshName");
        ImGui::TableNextColumn();
        std::vector<const char*> meshNames;
        int number = 0;
        bool found = false;
        for (auto& pair : MeshManager.mContainer)
        {
            if (pair.first == comp.MeshName)
                found = true;
            meshNames.push_back(pair.first.c_str());
            if (!found)
            {
                ++number;
            }
        }
        ImGui::PushItemWidth(-1);
        ImGui::Combo("Mesh Name", &number, meshNames.data(), (int)meshNames.size(), 5);
        ImGui::PopItemWidth();
        comp.MeshName = meshNames[number];
    }
    // @joe do the drop down hahaha, idk how to do it
    if constexpr (std::is_same<T, AudioSource>()) {
        //Combo field for mesh renderer
        ImGui::AlignTextToFramePadding();
        ImGui::TableNextColumn();
        ImGui::Text("Channel");
        ImGui::TableNextColumn();
        static int number = 0;
        ImGui::PushItemWidth(-1);
        ImGui::Combo("Channel", &number, comp.ChannelName.data(), (int)comp.ChannelName.size(), 4);
        ImGui::PopItemWidth();
        comp.channel = static_cast<AudioSource::Channel>(number);
    }

    //std::vector<property::entry> List;
    property::SerializeEnum(comp, [&](std::string_view PropertyName, property::data&& Data, const property::table&, std::size_t, property::flags::type Flags)
        {
            if (!Flags.m_isDontShow){
                auto entry = property::entry { PropertyName, Data };
                std::visit([&](auto& Value) {
                    using T1 = std::decay_t<decltype(Value)>;

                    //Edit name
                    std::string DisplayName = entry.first;
                    auto it = DisplayName.begin() + DisplayName.find_last_of("/");
                    DisplayName.erase(DisplayName.begin(), ++it);

                    ImGui::PushID(entry.first.c_str());                   
       
                    Display<T1>(DisplayName.c_str(), Value);

                    //temporary implementation for texture picker
                    if (DisplayName == "AlbedoTexture" || DisplayName == "NormalMap" || DisplayName == "MetallicTexture"
                        || DisplayName == "RoughnessTexture" || DisplayName == "AoTexture"|| DisplayName == "EmissionTexture")
                    {
                        DisplayTexturePicker(Value);
                    }

                    ImGui::PopID();
                    }
                , Data);
                property::set(comp, entry.first.c_str(), Data);

                // If we are dealing with a scope that is not an array someone may have change the SerializeEnum to a DisplayEnum they only show up there.
                assert(Flags.m_isScope == false || PropertyName.back() == ']');
            }
           
        });

    if constexpr (std::is_same<T, MeshRenderer>())
    {
        // Bean: Change this after M1, this is not suppose to be here
        if (comp.AlbedoTexture != "")
        {
            comp.textureID = GET_TEXTURE_ID(comp.AlbedoTexture)
        }
        else
            comp.textureID = 0;
        if (comp.NormalMap != "")
        {
            comp.normalMapID = GET_TEXTURE_ID(comp.NormalMap);
        }
        else
            comp.normalMapID = 0;
        if (comp.MetallicTexture != "")
        {
            comp.MetallicID = GET_TEXTURE_ID(comp.MetallicTexture);
        }
        else
            comp.MetallicID = 0;
        if (comp.RoughnessTexture != "")
        {
            comp.RoughnessID = GET_TEXTURE_ID(comp.RoughnessTexture);
        }
        else
            comp.RoughnessID = 0;
        if (comp.AoTexture != "")
        {
            comp.AoID = GET_TEXTURE_ID(comp.AoTexture);
        }
        else
            comp.AoID = 0;

        if (comp.EmissionTexture != "")
        {
            comp.EmissionID = GET_TEXTURE_ID(comp.EmissionTexture);
        }
        else
            comp.EmissionID = 0;

    }
}

void DisplayComponent(Script& script)
{
    ACQUIRE_SCOPED_LOCK(Mono);
    static char buffer[2048]{};
    ScriptGetFieldNamesEvent getFieldNamesEvent{script};
    EVENTS.Publish(&getFieldNamesEvent);
    for (size_t i = 0; i < getFieldNamesEvent.count; ++i)
    {
        const char* fieldName = getFieldNamesEvent.pStart[i];
        Field field{ AllFieldTypes::Size(),buffer };
        ScriptGetFieldEvent getFieldEvent{script,fieldName,field};
        EVENTS.Publish(&getFieldEvent);
        if (field.fType < AllFieldTypes::Size())
        {
            Display(fieldName, field);
            if (isAddingReference)
            {
                //Hash
                editedContainer = script.UUID() ^ i;
                isAddingReference = false;
            }
            if (editedContainer == (script.UUID() ^ i))
            {
                AddReferencePanel(field.fType, &field);
            }
            ScriptSetFieldEvent setFieldEvent{ script,fieldName,field};
            EVENTS.Publish(&setFieldEvent);
        }
        
    }
}

void DisplayLightProperties(LightSource& source) {

    DisplayLightTypes(source.lightType);
    
    Display<float>("Intensity", source.intensity);
    Display<Vector3>("Color", source.lightingColor);

    if (source.lightType == (int)SPOT_LIGHT) {
        Display<Vector3>("Light Position", source.lightpos);
        Display<Vector3>("Direction", source.direction);
        Display<float>("Inner Cutoff", source.inner_CutOff);
        Display<float>("Outer Cutoff", source.outer_CutOff);
    }
    else if(source.lightType == (int)DIRECTIONAL_LIGHT){
        Display<Vector3>("Direction", source.direction);
    }
    else { //POINT LIGHT
        Display<Vector3>("Light Position", source.lightpos);
    }
}

//Helper function that displays all relevant fields and types in a component
template <typename T>
void DisplayComponentHelper(T& component)
{
    Scene& curr_scene = SceneManager::Instance().GetCurrentScene();
    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_AllowItemOverlap;
    static std::string name{};
    if constexpr (std::is_same<T, Script>())
    {
        if(&component)
            name = (component.name + " [Script]");
    }
    else if constexpr (AllComponentTypes::Has<T>())
    {
        name = GetType::Name<T>();
    }
    else
    {
        //This means T is not a component
        //PRINT(typeid(T).name());
    }
    
    ImVec4 check_color = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.3f, 1.0f)); // set color of checkbox

    bool checkbox = curr_scene.IsActive(component);
    std::string label = "##" + name;
    ImGui::Checkbox(label.c_str(), &checkbox);
    curr_scene.SetActive(component,checkbox);

    ImGui::PopStyleColor(); 
    
    ImGui::SameLine(ImGui::GetItemRectSize().x + 10.f);

    bool windowopen = ImGui::CollapsingHeader(name.c_str(), nodeFlags);

    ImGui::SameLine(ImGui::GetWindowContentRegionWidth() - 30.f);

    ImGuiWindowFlags win_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    const char* popup = GetType::Name<T>();

    ImGui::PushID((int)component.UUID());

    if (ImGui::Button("...")) {
        ImGui::OpenPopup(popup);
    }

    //Component Settings window
    ImGui::SetNextWindowSize(ImVec2(150.f, 180.f));
    if (ImGui::BeginPopup(popup, win_flags)) {

        if (ImGui::MenuItem("Reset")) {

        }

        if constexpr (!std::is_same<T, Transform>()) {
            if (ImGui::MenuItem("Remove Component")) {
                //Destroy current component of current selected entity in editor

                curr_scene.Destroy(component);
            }
        }
        else {
            ImGui::TextDisabled("Remove Component");
        }

        if (ImGui::MenuItem("Copy Component")) {

        }

        ImGui::EndPopup();
    }

    ImGui::PopID();

    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Component Settings");
    
    if (windowopen)
    {
        ImGuiWindowFlags winFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_NoBordersInBody
            | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_SizingStretchProp
            | ImGuiTableFlags_PadOuterX;

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(6, 2));

        

        if (ImGui::BeginTable("Component", 2, winFlags))
        {
            ImGui::Indent();
            ImGui::TableSetupColumn("Text", 0, 0.4f);
            ImGui::TableSetupColumn("Input", 0, 0.6f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 0));

            if constexpr (std::is_same_v<T,Script>)
            {
                DisplayComponent(component);
            }
            if constexpr (std::is_same_v<T, LightSource>) {
                DisplayLightProperties(component);
            }
            else
            {
                Display_Property(component);
            }

            //ImGui::PopID();

            ImGui::PopStyleVar();
            ImGui::PopStyleVar();
            ImGui::PopStyleVar();

            ImGui::Unindent();
            ImGui::EndTable();
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();
    }

}

//Template recursive function to display the components in an entity
template<typename T, typename... Ts>
struct DisplayComponentsStruct
{
public:
    constexpr DisplayComponentsStruct(TemplatePack<T, Ts...> pack) {}
    DisplayComponentsStruct() = delete;
    DisplayComponentsStruct(Entity& entity)
    {
        ImGui::TableNextColumn();
        DisplayNext<T, Ts...>(entity);
    }
private:
    template<typename T1, typename... T1s>
    void DisplayNext(Entity& entity)
    {
        Scene& curr_scene = SceneManager::Instance().GetCurrentScene();

        if constexpr (SingleComponentTypes::Has<T1>()) {
            if (curr_scene.Has<T1>(entity)) {
                //dont display tag component as it is already on top of the inspector
                if constexpr (!std::is_same<T1, Tag>())
                {   
                    auto& component = curr_scene.Get<T1>(entity);
                    DisplayComponentHelper(component);
                }              
            }
        }
        else if constexpr (MultiComponentTypes::Has<T1>()) {

            auto components = curr_scene.GetMulti<T1>(entity);
            for (T1* component : components){
                DisplayComponentHelper(*component);
            }
        }

        if constexpr (sizeof...(T1s) != 0)
        {
            DisplayNext<T1s...>(entity);
        }
    }
};
using DisplayAllComponentsStruct = decltype(DisplayComponentsStruct(AllComponentTypes()));

void DisplayComponents(Entity& entity) { DisplayAllComponentsStruct obj{ entity }; }

//Lists out all available components to add in the add component panel
template<typename T, typename... Ts>
struct AddsStruct
{
public:
    constexpr AddsStruct(TemplatePack<T, Ts...> pack) {}
    AddsStruct(Entity& entity)
    {
        AddNext<T, Ts...>(entity,MySceneManager.GetCurrentScene());
    }
private:
    template<typename T1, typename... T1s>
    void AddNext(Entity& entity, Scene& scene)
    {
        if constexpr (SingleComponentTypes::Has<T1>()) {
            if (!scene.Has<T1>(entity))
            {
                if (CENTERED_CONTROL(ImGui::Button(GetType::Name<T1>(), ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetTextLineHeightWithSpacing()))))
                {
                    scene.Add<T1>(entity);
                    EditorInspector::Instance().isAddPanel = false;
                }
            }
        }
        else
        {
            if constexpr (std::is_same_v<T1, Script>)
            {
                GetScriptNamesEvent nameEvent;
                EVENTS.Publish(&nameEvent);

                for (size_t i = 0; i < nameEvent.count; ++i)
                {
                    if (CENTERED_CONTROL(ImGui::Button(nameEvent.arr[i], ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetTextLineHeightWithSpacing()))))
                    {
                        scene.Add<T1>(entity, nameEvent.arr[i]);
                        EditorInspector::Instance().isAddPanel = false;
                    }
                }
            }
            else
            {
                if (CENTERED_CONTROL(ImGui::Button(GetType::Name<T1>(), ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetTextLineHeightWithSpacing()))))
                {
                    scene.Add<T1>(entity);
                    EditorInspector::Instance().isAddPanel = false;
                }
            }
        }

        if constexpr (sizeof...(T1s) != 0)
        {
            AddNext<T1s...>(entity,scene);
        }
    }
};
using AddsDisplay = decltype(AddsStruct(DisplayableComponentTypes()));

//Implementation for the panel to add a component to the current entity
void AddPanel(Entity& entity) {
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(300, 500));

    //press esc to exit add component window
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        EditorInspector::Instance().isAddPanel = false;
    }
    ImGui::OpenPopup("Add Component");
    if (ImGui::BeginPopupModal("Add Component", &EditorInspector::Instance().isAddPanel, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar)) {

        (void)AddsDisplay(entity);
        ImGui::EndPopup();
    }
}

void DisplayLayers() { 
    Engine::UUID curr_index = EditorHierarchy::Instance().selectedEntity;
    Scene& curr_scene = SceneManager::Instance().GetCurrentScene();
    Entity& curr_entity = curr_scene.Get<Entity>(curr_index);

    std::vector<const char*> layers;
    for (auto& it : EditorInspector::Instance().Layers)
        layers.push_back(it.name.c_str());
    static int index = curr_entity.current_layer;
    ImGui::Text("Layer"); ImGui::SameLine();
    ImGui::PushItemWidth(100.f);
    ImGui::Combo("##Layer", &index, layers.data(), (int)layers.size(), 5);
    ImGui::PopItemWidth();
    curr_entity.current_layer = index;
}

void DisplayTags() {
    Engine::UUID curr_index = EditorHierarchy::Instance().selectedEntity;
    Scene& curr_scene = SceneManager::Instance().GetCurrentScene();
    Entity& curr_entity = curr_scene.Get<Entity>(curr_index);

    std::vector<const char*> layers;
    for (auto& it : EditorInspector::Instance().Tags)
        layers.push_back(it.c_str());
    static int index = curr_entity.tag;
    ImGui::Text("Tags"); ImGui::SameLine();
    ImGui::PushItemWidth(100.f);
    ImGui::Combo("##Tags", &index, layers.data(), (int)layers.size(), 5);
    ImGui::PopItemWidth();
    curr_entity.tag = index;
}

//Display all the components as well as the name and whether the entity is enabled in the scene.
void DisplayEntity(Entity& entity)
{
    Engine::UUID curr_index = EditorHierarchy::Instance().selectedEntity;
    Scene& curr_scene = SceneManager::Instance().GetCurrentScene();
    Entity& curr_entity = curr_scene.Get<Entity>(curr_index);

    bool enabled = curr_scene.IsActive(entity);
    ImGui::Checkbox("##Active", &enabled);
    curr_scene.SetActive(entity, enabled);
    ImGui::SameLine();
    static char buffer[256];
    std::string entity_name = curr_scene.Get<Tag>(entity).name;
    strcpy_s(buffer, entity_name.c_str());
    ImGui::PushItemWidth(-1);
    if (ImGui::InputText("##gameObjName", buffer, 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
        curr_scene.Get<Tag>(entity).name = buffer;
    }

    ImGui::PopItemWidth();
    curr_scene.Get<Tag>(entity).name = buffer;
  
    //display tags
    DisplayTags(); ImGui::SameLine();
    //display layer
    DisplayLayers();

    ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerH
        | ImGuiTableFlags_ScrollY;


    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.f);

    if (ImGui::BeginTable("Components", 1, tableFlags))
    {
        ImGui::PushID((int)entity.EUID());
        DisplayComponents(entity);
        ImGui::PopID();
        ImGui::Separator();
        if (CENTERED_CONTROL(ImGui::Button("Add Component", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, ImGui::GetTextLineHeightWithSpacing())))) {
            EditorInspector::Instance().isAddPanel = true;
        }

        ImGui::EndTable();
        
    }
    ImGui::PopStyleVar();
}

void EditorInspector::Init()
{
    isAddPanel = false;
    //default layers (same as unity)
    Layers.push_back(layer("Default"));
    Layers.push_back(layer("TransparentFX"));
    Layers.push_back(layer("Ignore Physics"));
    Layers.push_back(layer("UI"));
    Layers.push_back(layer("Water"));
    Tags.push_back("Untagged");   
}

void EditorInspector::Update(float dt)
{
    UNREFERENCED_PARAMETER(dt);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0,0 });
    ImGui::SetNextWindowSizeConstraints(ImVec2(320, 180), ImVec2(FLT_MAX, FLT_MAX));

    ImGui::Begin("Inspector");
    //Get Selected Entities from mouse picking
    //List out all components in order
    //templated functionalities (input fields, checkboxes etc.)

    Engine::UUID curr_index = EditorHierarchy::Instance().selectedEntity;

    Scene& curr_scene = SceneManager::Instance().GetCurrentScene();

    Entity& curr_entity = curr_scene.Get<Entity>(curr_index);
    //if (curr_index != NON_VALID_ENTITY) {
    if (&curr_entity) {
        ImGui::Spacing();
        std::string Header = "Current Entity: " + curr_scene.Get<Tag>(curr_index).name;
        ImGui::Text(Header.c_str()); ImGui::Spacing(); ImGui::Separator();
        DisplayEntity(curr_entity);
    }

    if (isAddPanel) {
        AddPanel(curr_scene.Get<Entity>(curr_index));
    }

    ImGui::PopStyleVar();
    ImGui::End();
}

void EditorInspector::Exit()
{

}