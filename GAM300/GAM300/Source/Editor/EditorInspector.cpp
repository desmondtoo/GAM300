/*!***************************************************************************************
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
#include "Graphics/Texture/TextureManager.h"
#include "Graphics/MeshManager.h"
#include <variant>
#include "PropertyConfig.h"
#include "Utilities/ThreadPool.h"
#include "Scene/Identifiers.h"
#include "Graphics/GraphicsHeaders.h"
#include "PropertyConfig.h"
#include "Utilities/Serializer.h"

#include "Core/Events.h"
#include "Core/EventsManager.h"

#define BUTTON_HEIGHT .1 //Percent
#define BUTTON_WIDTH .6 //Percent
#define TEXT_BUFFER_SIZE 2048
#define PI 3.1415927
#define EPSILON 0.01f

enum ComponentType {
    AUDIO, MESHRENDERER, SPRITERENDERER
};

//Flags for inspector headers/windows
static ImGuiTableFlags windowFlags =
ImGuiTableFlags_Resizable |
ImGuiTableFlags_NoBordersInBody |
ImGuiTableFlags_NoSavedSettings |
ImGuiTableFlags_SizingStretchProp;

bool referenceChanged = false;
Object* previousReference = nullptr;
Object* newReference = nullptr;
std::string refFieldName{};
bool isAddingReference = false;
size_t editedContainer{};

//for checking of drag fields
bool valueChanged = false;
float initialvalue = 0.f;
float changedvalue = 0.f;
Vector3 initialVector;
Vector4 initialColor;

template <typename T>
void Display(const char* name, T& val);

// DisplayType contains overloads that display the respective fields based on the type passed into the function
template <typename T>
void DisplayType(Change& change, const char* name, T& val)
{
    UNREFERENCED_PARAMETER(change);
    UNREFERENCED_PARAMETER(name);
    UNREFERENCED_PARAMETER(val);
}

template <typename T>
void DisplayType(const char* name, T& val)
{
    UNREFERENCED_PARAMETER(name);
    UNREFERENCED_PARAMETER(val);
}

void DisplayType(Change& change, const char* name, bool& val)
{
    static std::string idName{};
    idName = "##";
    idName += name;
    bool buf = val;
    if (ImGui::Checkbox(idName.c_str(), &buf)) {
        EDITOR.History.SetPropertyValue(change, val, buf);
    }
}

//for texture picker strings (read-only)
void DisplayType(const char* name, std::string& val)
{
    static std::string idName{};
    idName = "##";
    idName += name;
    PRINT(val, '\n');
    static char buffer[TEXT_BUFFER_SIZE];
    strcpy(buffer, val.c_str());
    ImGui::InputText(idName.c_str(), buffer, ImGuiInputTextFlags_ReadOnly);
    val = buffer;
}

void DisplayType(Change& change, const char* name, std::string& val)
{
    static std::string idName{};
    idName = "##";
    idName += name;
    static char buffer[TEXT_BUFFER_SIZE];
    strcpy(buffer, val.c_str());
    if(ImGui::InputText(idName.c_str(), buffer, 1024, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        std::string newString = buffer;
        EDITOR.History.SetPropertyValue(change, val, newString);
    }
    val = buffer;
}

void DisplayType(Change& change, const char* name, int& val)
{
    static std::string idName{};
    idName = "##";
    idName += name;
    int buf = val;
    if (ImGui::DragInt(idName.c_str(), &buf)) {
        EDITOR.History.SetPropertyValue(change, val, buf);
    }
}

template <size_t SZ>
void DisplayType(Change& change, const char* name, char(&val)[SZ])
{
    static std::string idName{};
    idName = "##";
    idName += name;
    char buf[SZ] = val;
    if (ImGui::InputTextMultiline(idName.c_str(), buf, SZ, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16))) {
        EDITOR.History.SetPropertyValue(change, val, buf);
    }
}

void DisplayType(Change& change, const char* name, char*& val)
{
    static std::string idName{};
    idName = "##";
    idName += name;
    char* buf = val; 
    if (ImGui::InputTextMultiline(idName.c_str(), buf, TEXT_BUFFER_SIZE, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16))) {
        EDITOR.History.SetPropertyValue(change, val, buf);
    }
}

template<typename AssetType>
void DisplayAssetPicker(Change& change,const fs::path& fp, Engine::GUID<AssetType>& guid)
{
    static ImGuiTextFilter filter;

    ImGui::SameLine();
    ImGuiWindowFlags win_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar;


    if (ImGui::Button("Edit")) {
        filter.Clear();
        ImGui::OpenPopup("Texture");
    }

    GLuint defaultFileIcon = TextureManager.GetTexture("Assets/Icons/fileicon.dds");

    //Component Settings window
    ImGui::SetNextWindowSize(ImVec2(250.f, 300.f));


    GetAssetsEvent<AssetType> assetsEvent{};
    EVENTS.Publish(&assetsEvent);

    // Bean: Ideally should only call this function when assigning a material onto a mesh
    MATERIALSYSTEM.BindAllTextureIDs();

    if (ImGui::BeginPopup("Texture", win_flags)) {
        
        ImGui::Dummy(ImVec2(0, 10.f));
        ImGui::Text("Filter: "); ImGui::SameLine();
        filter.Draw();

        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -20.f), false);
        // Back button to return to parent directory
        static float padding = 15.f;
        static float iconsize = 50.f;
        float cellsize = iconsize + padding;

        float window_width = ImGui::GetContentRegionAvail().x;
        int columncount = (int)(window_width / cellsize);
        if (columncount < 1) { columncount = 1; }

        ImGui::Columns(columncount, 0, false);

        //remove texture icon
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0, 0, 0, 0 });
        size_t id = (size_t)GET_TEXTURE_ID("Assets/Icons/Cancel_Icon.dds");
        if (ImGui::ImageButton((ImTextureID)id, { iconsize, iconsize }, { 0 , 1 }, { 1 , 0 })) 
        {
            static Engine::GUID<AssetType> none{ 0 };
            EDITOR.History.SetPropertyValue(change, guid, none);
            ImGui::PopStyleColor();
            ImGui::EndChild();
            ImGui::EndPopup();
            ImGui::CloseCurrentPopup();
            return;
        };
        ImGui::PopStyleColor();
        //render file name below icon
        ImGui::TextWrapped("None");
        ImGui::NextColumn();

        int i = 0;

        //using filesystem to iterate through all folders/files inside the "/Data" directory

        for (auto& assetPair : *assetsEvent.pAssets)
        {
            const auto& path = assetPair.second.mFilePath;

            if (!filter.PassFilter(path.string().c_str()))
                continue;
            //if (path.extension() != extension)

            //if not png or dds file, dont show
            Engine::GUID<AssetType> currentGUID{ assetPair.first };

            //Draw the file / folder icon based on whether it is a directory or not
            GLuint icon_id = defaultFileIcon;
            if constexpr (std::is_same_v<AssetType,TextureAsset>)
            {
                icon_id = GET_TEXTURE_ID(currentGUID);
                if (icon_id == 0) {
                    icon_id = defaultFileIcon;
                }
            }

            ImGui::PushID(i++);
            ImTextureID textureID = (ImTextureID)icon_id;
            //render respective file icon textures
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0, 0, 0, 0 });
            if (ImGui::ImageButton(textureID, { iconsize, iconsize }, { 0 , 0 }, { 1 , 1 }))
            {
                EDITOR.History.SetPropertyValue(change, guid, currentGUID);
            }
            
            ImGui::PopStyleColor();
            ImGui::TextWrapped(path.stem().string().c_str());

            ImGui::NextColumn();
            ImGui::PopID();
        }
       
        ImGui::Columns(1);
        ImGui::EndChild();
        ImGui::EndPopup();
    }
}

template <typename AssetType>
void DisplayType(Change& change, const char* name, Engine::GUID<AssetType>& val)
{
    static std::string idName{};
    idName = "##";
    idName += name;
    //Val is a default asset guid
    GetFilePathEvent<AssetType> e{ val };
    EVENTS.Publish(&e);
    const std::string& pathStr = e.filePath.stem().string();
    ImGui::InputText(idName.c_str(), (char*)pathStr.c_str(), pathStr.size(), ImGuiInputTextFlags_ReadOnly);

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {

            ContentBrowserPayload data = *(const ContentBrowserPayload*)payload->Data;
            Engine::HexID guid = data.guid;

            if (data.type == MATERIAL) {

                //Assign material to mesh renderer
                Engine::GUID<AssetType> matGuid = guid;
                EDITOR.History.SetPropertyValue(change, val, matGuid);
            }

            //add other file types here              
        }
        ImGui::EndDragDropTarget();
    }
    DisplayAssetPicker(change,e.filePath, val);
}



bool DisplayType(const char* name, float& val)
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
    bool ischanged = false;
    float buf = val;

    if (ImGui::DragFloat(cIdName, &buf, 0.15f)) {
        if (!valueChanged) {
            initialvalue = val;
        }
        valueChanged = true;
        val = buf;
    }

    if (ImGui::IsItemDeactivatedAfterEdit()) {
        ischanged = true;
        valueChanged = false;
    }

    return ischanged;
}


void DisplayType(Change& change, const char* name, float& val)
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
    //ImGui::DragFloat(cIdName, &val, 0.15f);

    float buf = val;

    if (!strcmp(name, "Alpha Scalar")) { //clamp alpha scalar for sprite renderer UI
        if (ImGui::DragFloat(cIdName, &buf, 0.001f, 0, 1.f)) {
            if (!valueChanged) {
                initialvalue = val;
            }
            valueChanged = true;
            val = buf;
            changedvalue = buf;
        }
    } 
    else if (ImGui::DragFloat(cIdName, &buf, 0.01f)) {
        if (!valueChanged) {
            initialvalue = val;
        }
        valueChanged = true;
        val = buf;
        changedvalue = buf;
    }

    if (ImGui::IsItemDeactivatedAfterEdit()) 
    {
        valueChanged = false;
        val = initialvalue;
        EDITOR.History.SetPropertyValue(change, val, changedvalue);
    }
}

//void DisplayType(Change& change, const char* name, double& val)
//{
//    static std::string idName{};
//    idName = "##";
//    idName += name;
//    float buf{ (float)val };
//    if (ImGui::DragFloat(idName.c_str(), &buf, 0.15f)) {
//        double temp = buf;
//        /EDITOR.History.SetPropertyValue(change, val, temp);
//    }
//}

void DisplayType(Change& change, const char* name, Vector3& val)
{
    static float temp{};
    static std::string idName{};
    idName = "##";
    idName += name;

    if (!strcmp(name, "Color") || !strcmp(name, "Trail Color") || !strcmp(name, "Color to Fade to")) {
        Vector4 buf = Vector4(val.x, val.y, val.z, 1.f);
        ImVec4 color = ImVec4(buf.x, buf.y, buf.z, buf.w);

        bool ischanged = false;

        if (ImGui::ColorButton("##color", color, 0, ImVec2(ImGui::GetContentRegionAvail().x, 20.f)))
            ImGui::OpenPopup("colorpicker");

        if (ImGui::BeginPopup("colorpicker"))
        {
            if (ImGui::ColorPicker4("##picker", (float*)&buf, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueWheel)) {
                ischanged = true;
                if (!valueChanged)
                    initialColor = Vector4(val.x, val.y, val.z, 1.f);

                valueChanged = true;
                val = Vector3(buf.x, buf.y, buf.z);
            }

            if (ImGui::IsItemDeactivatedAfterEdit()) {
                valueChanged = false;
                buf = Vector4(val.x, val.y, val.z, 1.f);;
                val = Vector3(initialColor.x, initialColor.y, initialColor.z);

                if (buf.w != initialColor.w) {
                    auto& material = MATERIALSYSTEM.getMaterialInstance(EditorContentBrowser::Instance().selectedAss);
                    material.shaderType = (int)SHADERTYPE::DEFAULT;
                }
                Vector3 convert = Vector3(buf.x, buf.y, buf.z);
                EDITOR.History.SetPropertyValue(change, val, convert);
            }
            ImGui::EndPopup();
        }
    }
    else {
        Vector3 buf = val;
        if (!std::strcmp(name, "Rotation")) {
            buf *= (180.f / PI);
        }
        bool changed = false;
        if (ImGui::BeginTable("Vector3", 3, windowFlags))
        {
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            idName += 'X';
            ImGui::Text("X"); ImGui::SameLine(); ImGui::SetNextItemWidth(-FLT_MIN);

            changed = DisplayType(idName.c_str(), buf.x);
            if (changed) {
                initialVector = val;

                initialVector.x = initialvalue;
                val = initialVector;
            }

            ImGui::TableNextColumn();
            idName.back() = 'Y';
            ImGui::Text("Y"); ImGui::SameLine(); ImGui::SetNextItemWidth(-FLT_MIN);
            if (!changed && DisplayType(idName.c_str(), buf.y)) {
                changed = true;
                initialVector = val;
                initialVector.y = initialvalue;
                val = initialVector;
            }

            ImGui::TableNextColumn();
            idName.back() = 'Z';
            ImGui::Text("Z"); ImGui::SameLine(); ImGui::SetNextItemWidth(-FLT_MIN);
            if (!changed && DisplayType(idName.c_str(), buf.z)) {
                changed = true;
                initialVector = val;
                initialVector.z = initialvalue;
                val = initialVector;
            }

            //convert rotation from degree back to radians
            if (!std::strcmp(name, "Rotation")) {
                buf *= (PI / 180.f);
                val *= (PI / 180.f);
                //Check whether value is within epsilon range (to avoid negative 0)
                for (int i = 0; i < 3; ++i) {
                    if (std::fabs(buf[i] - 0.f) < EPSILON)
                        buf[i] = 0;
                }
            }

            if (!changed)
                val = buf;

            ImGui::EndTable();
        }

        if (changed) {
            EDITOR.History.SetPropertyValue(change, val, buf);
        }
    }
}

void DisplayType(Change& change, const char* name, Vector4& val)
{
    static float temp{};
    static std::string idName{};
    idName = "##";
    idName += name;

    Vector4 buf = val;
    ImVec4 color = ImVec4(buf.x, buf.y, buf.z, buf.w);

    bool ischanged = false;

    if (ImGui::ColorButton("##color", color, 0, ImVec2(ImGui::GetContentRegionAvail().x, 20.f)))
        ImGui::OpenPopup("colorpicker");

    if (ImGui::BeginPopup("colorpicker"))
    {
        if (ImGui::ColorPicker4("##picker", (float*)&buf, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueWheel)) {
            ischanged = true;
            if (!valueChanged)
                initialColor = val;

            valueChanged = true;
            val = buf;
        }

        if (ImGui::IsItemDeactivatedAfterEdit()) {
            valueChanged = false;
            buf = val;
            val = initialColor;

            if (buf.w != initialColor.w) {
                auto& material = MATERIALSYSTEM.getMaterialInstance(EditorContentBrowser::Instance().selectedAss);
                material.shaderType = (int)SHADERTYPE::DEFAULT;
            }

            EDITOR.History.SetPropertyValue(change, val, buf);
        }
        ImGui::EndPopup();
    }

}

void DisplayType(Change& change, const char* name, Vector2& val)
{
    static float temp{};
    static std::string idName{};
    idName = "##";
    idName += name;
    bool changed = false;
    Vector2 buf = val;
    if (ImGui::BeginTable("Vector2", 2, windowFlags))
    {
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        idName += 'X';
        ImGui::Text("X"); ImGui::SameLine(); ImGui::SetNextItemWidth(-FLT_MIN);
        changed = DisplayType(idName.c_str(), buf.x);

        ImGui::TableNextColumn();
        idName.back() = 'Y';
        ImGui::Text("Y"); ImGui::SameLine(); ImGui::SetNextItemWidth(-FLT_MIN);

        if (!changed && DisplayType(idName.c_str(), buf.y))
            changed = true;

        if (changed)
            //EDITOR.History.SetPropertyValue(change, val, buf);

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
        ImGui::Text("Filter"); ImGui::SameLine();
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
                //if reference is changed, add it to reference buffer
                if (container != &object) {
                    previousReference = container;
                    newReference = &object;
                    referenceChanged = true;
                }
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
    AddReferencePanel(((Field*)pObject )->Get<T*>())
)

template <typename T>
void DisplayType(Change& change, const char* name, T*& container, const char* altName = nullptr)
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
void DisplayField(Change& change, const char* name, Field& field)
{
    if (GetFieldType::E<T>() == field.fType)
    {
        if (field.fType < AllObjectTypes::Size())
        {
            Scene& scene{ MySceneManager.GetCurrentScene() };
            T*& value = *reinterpret_cast<T**>(field.data);
            if constexpr (std::is_same<T, Script>())
            {
                DisplayType(change, name, value, field.typeName.c_str());
            }
            else
            {
                
                DisplayType(change, name, value);
            }
        }
        else
        {
            if constexpr (std::is_same_v<T, char*>)
            {
                char* str = (char*)field.data;
                std::string val = str;
                DisplayType(change, name, val);
                if (val.size() >= TEXT_BUFFER_SIZE - 1)
                {
                    memcpy(str, val.data(), TEXT_BUFFER_SIZE - 1);
                    str[TEXT_BUFFER_SIZE - 1] = 0;
                }
                else
                {
                    strcpy(str, val.data());
                }
            }
            else
            {
                DisplayType(change,name, field.Get<T>());
            }
        }
        return;
    }
    if constexpr (sizeof...(Ts) != 0)
    {
        DisplayField<Ts...>(change, name, field);
    }
}

template <typename T, typename... Ts>
void DisplayField(Change& change, const char* name, Field& field, TemplatePack<T,Ts...>)
{
    DisplayField<T,Ts...>(change, name,field);
}

void DisplayType(Change& change, const char* name, Field& val)
{
    DisplayField(change, name, val, AllFieldTypes());
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

template <typename T>
void Display(Change& change, const char* name, T& val)
{
    ImGui::AlignTextToFramePadding();
    ImGui::TableNextColumn();
    ImGui::Text(name);
    ImGui::TableNextColumn();
    DisplayType(change, name, val);
}

void Display(const char* string)
{
    ImGui::TableNextColumn();
    ImGui::AlignTextToFramePadding();
    ImGui::Text(string);
}

//Function to display and edit textures of a given property.

template <typename T>
void DisplayLightTypes(Change& change, T& value) {
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
        int index = value;
        ImGui::PushItemWidth(100.f);
        if (ImGui::Combo("##LightType", &index, layers.data(), (int)layers.size(), 5)) {
            EDITOR.History.SetPropertyValue(change, value, index);
        }
        ImGui::PopItemWidth();
    }
}

//Display all available audio channels inside the system for user to choose
template <typename T>
void DisplayAudioChannels(Change& change, T& value) {
    if constexpr (std::is_same<T, int>()) {
        ImGui::AlignTextToFramePadding();
        ImGui::TableNextColumn();
        ImGui::Text("Channel");
        ImGui::TableNextColumn();

        Engine::UUID curr_index = EditorHierarchy::Instance().selectedEntity;
        Scene& curr_scene = SceneManager::Instance().GetCurrentScene();
        Entity& curr_entity = curr_scene.Get<Entity>(curr_index);

        std::vector<const char*> layers;
        layers.push_back("Music"); layers.push_back("SFX"); layers.push_back("Loop FX");
        int index = value;
        ImGui::PushItemWidth(100.f);
        if (ImGui::Combo("##AudioChannel", &index, layers.data(), (int)layers.size(), 5)) {
            EDITOR.History.SetPropertyValue(change, value, index);
        }
        ImGui::PopItemWidth();
    }
}

int shader_id;

//Display all available shaders inside the system for user to choose
template <typename T>
void DisplayShaders(Change& change, T& value) {
    if constexpr (std::is_same_v<T, int>) {
        ImGui::AlignTextToFramePadding();
        ImGui::TableNextColumn();
        ImGui::Text("Shader");
        ImGui::TableNextColumn();

        Engine::UUID curr_index = EditorHierarchy::Instance().selectedEntity;
        Scene& curr_scene = SceneManager::Instance().GetCurrentScene();
        Entity& curr_entity = curr_scene.Get<Entity>(curr_index);
        MeshRenderer& rend = static_cast<MeshRenderer&>(*change.component);

        std::vector<const char*> layers;
        shader_id = value;
        //Get all materials inside PBR shader
        for (auto& shader : MATERIALSYSTEM.available_shaders) {
            layers.push_back(shader.name.c_str());
        }


        //ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::Combo("##Shader", &shader_id, layers.data(), (int)layers.size(), 5)) {
            EDITOR.History.SetPropertyValue(change, value, shader_id);
        }
    }
}


//Displays all the properties of an given entity
template <typename T>
void Display_Property(T& comp) {

    property::DisplayEnum(comp, [&](std::string_view PropertyName, property::data&& Data, const property::table&, std::size_t, property::flags::type Flags)
        {
            auto entry = property::entry { PropertyName, Data };
            std::visit([&](auto& Value) {
                using T1 = std::decay_t<decltype(Value)>;

                std::string DisplayName = entry.first;
                auto it = DisplayName.begin() + DisplayName.find_last_of("/");
                DisplayName.erase(DisplayName.begin(), ++it);

                Change newchange(&comp, entry.first);

                ImGui::PushID(entry.first.c_str());

                    //Temporary implementation for materials
                    if (entry.first.find("Shader") != std::string::npos) {
                        DisplayShaders(newchange, Value);
                    }
                    else if (entry.first.find("AudioChannel") != std::string::npos) {
                        DisplayAudioChannels(newchange, Value);
                    }
                    else {
                        Display<T1>(newchange, DisplayName.c_str(), Value);
                        if (DisplayName == "Material_ID" || DisplayName == "Particle Material") {
                            ImGui::SameLine();
                            if (!EditorScene::Instance().multiselectEntities.size()) {
                                if (ImGui::Button("Mat")) {
                                    Scene& scene = MySceneManager.GetCurrentScene();
                                    Entity& ent = scene.Get<Entity>(comp);
                                    auto& mr = scene.Get<MeshRenderer>(ent);
                                    auto& particle = scene.Get<ParticleComponent>(ent);
                                    if (!EditorInspector::Instance().material_inspector) {
                                        EditorInspector::Instance().material_inspector = true;
                                    }

                                    ImGui::SetWindowFocus("Material");
                                    if(DisplayName == "Material_ID")
                                        EditorContentBrowser::Instance().selectedAss = mr.materialGUID;
                                    else
                                        EditorContentBrowser::Instance().selectedAss = particle.materialGUID;
                                }
                            }                       
                        }
                    }

                ImGui::PopID();

                }
            , Data);
            property::set(comp, entry.first.c_str(), Data);

            // If we are dealing with a scope that is not an array someone may have change the SerializeEnum to a DisplayEnum they only show up there.
            //assert(Flags.m_isScope == false || PropertyName.back() == ']');
           
        });

    if constexpr (std::is_same_v<T, Transform>)
    {
        comp.RecalculateLocalMatrices();
    }

    //property::SerializeEnum(comp, [&](std::string_view PropertyName, property::data&& Data, const property::table&, std::size_t, property::flags::type Flags)
    //    {
    //        if (!Flags.m_isDontShow) {
    //            auto entry = property::entry { PropertyName, Data };
    //            std::visit([&](auto& Value) {
    //                using T1 = std::decay_t<decltype(Value)>;

    //                std::string DisplayName = entry.first;
    //                auto it = DisplayName.begin() + DisplayName.find_last_of("/");
    //                DisplayName.erase(DisplayName.begin(), ++it);

    //                Change newchange(&comp, entry.first);

    //                ImGui::PushID(entry.first.c_str());

    //                //Temporary implementation for materials
    //                if (entry.first.find("Shader") != std::string::npos) {
    //                    DisplayShaders(newchange, Value);
    //                }
    //                else if (entry.first.find("AudioChannel") != std::string::npos) {
    //                    DisplayAudioChannels(newchange, Value);
    //                }
    //                else {
    //                    Display<T1>(newchange, DisplayName.c_str(), Value);
    //                    if (DisplayName == "Material_ID") {
    //                        ImGui::SameLine();
    //                        if (ImGui::Button("Mat")) {
    //                            Scene& scene = MySceneManager.GetCurrentScene();
    //                            Entity& ent = scene.Get<Entity>(comp);
    //                            auto& mr = scene.Get<MeshRenderer>(ent);
    //                            if(!EditorInspector::Instance().material_inspector){
    //                                EditorInspector::Instance().material_inspector = true; 
    //                            }

    //                            ImGui::SetWindowFocus("Material");                                
    //                            EditorContentBrowser::Instance().selectedAss = mr.materialGUID;
    //                        }
    //                    }
    //                }

    //                ImGui::PopID();

    //                }
    //            , Data);
    //            property::set(comp, entry.first.c_str(), Data);

    //            // If we are dealing with a scope that is not an array someone may have change the SerializeEnum to a DisplayEnum they only show up there.
    //            //assert(Flags.m_isScope == false || PropertyName.back() == ']');
    //        }
    //       
    //    });
}

//Display all fields from a script component
void DisplayComponent(Script& script)
{
    static char buffer[2048]{};
    ScriptGetFieldNamesEvent getFieldNamesEvent{script};
    EVENTS.Publish(&getFieldNamesEvent);
    for (size_t i = 0; i < getFieldNamesEvent.count; ++i)
    {
        const char* fieldName = getFieldNamesEvent.pStart[i];
        Field field{ AllFieldTypes::Size(),2048, buffer};
        ScriptGetFieldEvent getFieldEvent{script,fieldName,field};
        EVENTS.Publish(&getFieldEvent);
        if (field.fType < AllFieldTypes::Size())
        {
            std::string fieldproperty = "Script/" + std::string(fieldName);
            Change change(&script, fieldproperty);
            change.type = SCRIPT;
            Display(change, fieldName, field);
            if (isAddingReference)
            {
                //Hash
                editedContainer = script.UUID() ^ i;
                isAddingReference = false;
            }
            if (editedContainer == (script.UUID() ^ i))
            {
                AddReferencePanel(field.fType, &field);
                //add reference change to undo stack
                if (referenceChanged) {
                    Change refChange(&script, fieldName);
                    EDITOR.History.AddReferenceChange(refChange, previousReference, newReference);
                    referenceChanged = false;
                }
            }
            ScriptSetFieldEvent setFieldEvent{ script,fieldName,field};
            EVENTS.Publish(&setFieldEvent);
        }
        
    }
}

//Display all available light types inside the system for user to choose
void DisplayLightProperties(LightSource& source) {

    Change enableshadow(&source, "LightSource/EnableShadow");
    Display<bool>(enableshadow, "Enable Shadow", source.enableShadow);

    Change lighttypes(&source, "LightSource/lightType");
    DisplayLightTypes(lighttypes, source.lightType);
    
    Change intensity(&source, "LightSource/Intensity");
    Display<float>(intensity, "Intensity", source.intensity);

    Change color(&source, "LightSource/Color");
    Display<Vector3>(color, "Color", source.lightingColor);

    Change lightpos(&source, "LightSource/lightpos");
    Change lightdir(&source, "LightSource/Direction");
    Change innercutoff(&source, "LightSource/Inner Cutoff");
    Change outercutoff(&source, "LightSource/Outer Cutoff");
    if (source.lightType == (int)SPOT_LIGHT) {
        Display<Vector3>(lightpos, "Light Position", source.lightpos);
        Display<float>(innercutoff, "Inner Cutoff", source.inner_CutOff);
        Display<float>(outercutoff, "Outer Cutoff", source.outer_CutOff);
    }
    else if(source.lightType == (int)DIRECTIONAL_LIGHT){
        Display<Vector3>(lightdir, "Direction", source.direction);
    }
    //else { //POINT LIGHT
    //    Display<Vector3>(lightpos, "Light Position", source.lightpos);
    //}

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
        GetFilePathEvent<ScriptAsset> e{component.scriptId};
        EVENTS.Publish(&e);
        if(&component)
            name = (e.filePath.stem().string() + " (Script)");
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

    bool checkbox = curr_scene.IsActive(component,false);
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

            property::SerializeEnum(component, [&](std::string_view PropertyName, property::data&& Data, const property::table&, std::size_t, property::flags::type Flags)
                {
                    if (!Flags.m_isDontShow) {
                        auto entry = property::entry { PropertyName, Data };
                        std::visit([&](auto& Value) {

                            using T1 = std::decay_t<decltype(Value)>;

                            if constexpr (std::is_same <T1, std::string>()) {
                                Value = std::string();
                            }
                            else if constexpr (std::is_same <T1, int>()) {
                                Value = 0;
                            }
                            else if constexpr (std::is_same < T1, float>()) {
                                Value = 0.f;
                            }

                            if constexpr (std::is_same<T1, Vector3>()) {
                                //if scale reset to 1 instead or else 0 
                                if (entry.first == "Transform/Scale") 
                                    Value = Vector3(1.f, 1.f, 1.f);                             
                                else
                                    Value = Vector3();
                            }
                            }
                        , Data);
                        property::set(component, entry.first.c_str(), Data);

                        // If we are dealing with a scope that is not an array someone may have change the SerializeEnum to a DisplayEnum they only show up there.
                        assert(Flags.m_isScope == false || PropertyName.back() == ']');
                    }

                });
        }

        if constexpr (!std::is_same<T, Transform>()) {
            if (ImGui::MenuItem("Remove Component")) {
                //Destroy current component of current selected entity in editor
                Entity& entity = curr_scene.Get<Entity>(component);
                if constexpr (SingleComponentTypes::Has<T>())
                {
                    entity.hasComponentsBitset.set(GetType::E<T>(), false);
                }
                else if constexpr (MultiComponentTypes::Has<T>())
                {
                    if (curr_scene.GetMulti<T>(entity).size() == 1)
                        entity.hasComponentsBitset.set(GetType::E<T>(), false);
                }
                Change newchange;
                newchange.component = &component;
                newchange.property = popup;
                EDITOR.History.AddComponentChange(newchange);
            }
        }
        else {
            ImGui::TextDisabled("Remove Component");
        }

        /*if (ImGui::MenuItem("Copy Component")) {

        }*/

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
            else if constexpr (std::is_same_v<T, LightSource>) {
                DisplayLightProperties(component);
                ImGui::AlignTextToFramePadding();
                ImGui::TableNextRow();
                ImGui::TableNextColumn();             
                if (ImGui::Button("Bake Light")) {
                    //Add bake function here (Euan)
                }
            }
            else
            {
                Display_Property(component);
            }

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
                    if (component.state != DELETED)
                        DisplayComponentHelper(component);
                }              
            }
        }
        else if constexpr (MultiComponentTypes::Has<T1>()) {

            auto components = curr_scene.GetMulti<T1>(entity);
            for (T1* component : components){
                if (component->state != DELETED)
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

static ImGuiTextFilter Componentfilter;

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
        bool old_component = false;

        if (scene.Has<T1>(entity)) {
            if (scene.Get<T1>(entity).state == DELETED) {
                old_component = true;
            }
        }

        if constexpr (!std::is_same_v<T1, Script>)
        {
            if (Componentfilter.PassFilter(GetType::Name<T1>())) {
                if constexpr (SingleComponentTypes::Has<T1>()) {

                    if (!scene.Has<T1>(entity) || old_component)
                    {
                        if (CENTERED_CONTROL(ImGui::Button(GetType::Name<T1>(), ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetTextLineHeightWithSpacing()))))
                        {
                            if (old_component)
                                scene.Destroy(scene.Get<T1>(entity));

                            T1* pObject = scene.Add<T1>(entity);

                            if constexpr (std::is_same<T1, BoxCollider>())
                            {

                                geometryDebugData temp;
                                if (scene.Has<MeshRenderer>(entity))
                                {
                                    MeshRenderer& mr = scene.Get<MeshRenderer>(entity);

                                    temp = MESHMANAGER.offsetAndBoundContainer.find(mr.meshID)->second;
                                }
                                else
                                {
                                    temp = MESHMANAGER.offsetAndBoundContainer.find(ASSET_CUBE)->second;
                                }

                                pObject->dimensions.x = temp.scalarBound.x;
                                pObject->dimensions.y = temp.scalarBound.y;
                                pObject->dimensions.z = temp.scalarBound.z;
                                pObject->offset = temp.offset;
                            }

                            Change newchange(pObject);
                            newchange.action = CREATING;
                            EDITOR.History.AddComponentChange(newchange);

                            EditorInspector::Instance().isAddComponentPanel = false;
                        }
                    }

                }
                else //multicomponent
                {
                    if (CENTERED_CONTROL(ImGui::Button(GetType::Name<T1>(), ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetTextLineHeightWithSpacing()))))
                    {
                        if (old_component)
                            scene.Destroy(scene.Get<T1>(entity));

                        T1* comp = scene.Add<T1>(entity);
                        Change newchange(comp);
                        newchange.action = CREATING;
                        EDITOR.History.AddComponentChange(newchange);
                        EditorInspector::Instance().isAddComponentPanel = false;
                    }
                }
            }
        }
        else { //SCRIPT
            GetAssetsEvent<ScriptAsset> e;
            EVENTS.Publish(&e);

            for (auto& pair : *e.pAssets)
            {
                if (Componentfilter.PassFilter(pair.second.mFilePath.stem().string().c_str())) {
                    if (CENTERED_CONTROL(ImGui::Button(pair.second.mFilePath.stem().string().c_str(), ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetTextLineHeightWithSpacing()))))
                    {
                        if (old_component)
                            scene.Destroy(scene.Get<T1>(entity));

                        T1* comp = scene.Add<T1>(entity, nullptr, pair.first);
                        Change newchange(comp);
                        newchange.action = CREATING;
                        EDITOR.History.AddComponentChange(newchange);
                        EditorInspector::Instance().isAddComponentPanel = false;
                    }
                }         
            }
        }



        if constexpr (sizeof...(T1s) != 0)
        {
            AddNext<T1s...>(entity, scene);
        }
    }
};
using AddsDisplay = decltype(AddsStruct(DisplayableComponentTypes()));

void DisplayMultiTransform() {

    ImGuiWindowFlags Flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_NoBordersInBody
        | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_SizingStretchProp
        | ImGuiTableFlags_PadOuterX;

    //Display_Property(EditorScene::Instance().multiTransform);
    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed
        | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_AllowItemOverlap;

    if (ImGui::BeginTable("Multiselect_Components", 1, Flags))
    {  
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(6, 2));

        ImGui::TableNextColumn();

        bool t_open = ImGui::CollapsingHeader("Group Transform", nodeFlags);     

        if (t_open) {
            if (ImGui::BeginTable("m_transform", 2, Flags))
            {
                ImGui::Indent();
                ImGui::TableSetupColumn("Text", 0, 0.4f);
                ImGui::TableSetupColumn("Input", 0, 0.6f);
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 0));
                ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 0));

                Display_Property(EditorScene::Instance().multiTransform);
  
                ImGui::PopStyleVar();
                ImGui::PopStyleVar();
                ImGui::PopStyleVar();

                ImGui::Unindent();
                ImGui::EndTable();
            }
        }

        if (EditorScene::Instance().useMeshRenderer) {
            ImGui::TableNextColumn();

            bool m_open = ImGui::CollapsingHeader("Group Mesh Renderer", nodeFlags);

            if (m_open) {
                if (ImGui::BeginTable("m_mesh", 2, Flags))
                {
                    ImGui::Indent();
                    ImGui::TableSetupColumn("Text", 0, 0.4f);
                    ImGui::TableSetupColumn("Input", 0, 0.6f);
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 0));
                    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 0));

                    Display_Property(EditorScene::Instance().multiMeshRenderer);

                    ImGui::PopStyleVar();
                    ImGui::PopStyleVar();
                    ImGui::PopStyleVar();

                    ImGui::Unindent();
                    ImGui::EndTable();
                }
            }
        }
       

        ImGui::PopStyleVar();
        ImGui::PopStyleVar();
        ImGui::EndTable();
    }
}

//Implementation for the panel to add a component to the current entity
void AddComponentPanel(Entity& entity) {
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(300, 500));

    //press esc to exit add component window
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        EditorInspector::Instance().isAddComponentPanel = false;
    }

    ImGui::OpenPopup("Add Component");
    if (ImGui::BeginPopup("Add Component", ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
        ImGui::Text("Filter: "); ImGui::SameLine();
        Componentfilter.Draw();
        (void)AddsDisplay(entity);
        ImGui::EndPopup();
    }
}

void AddTagPanel(){
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400, 500));

    //press esc to exit add component window
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        EditorInspector::Instance().isAddTagPanel = false;
    }

    ImGui::OpenPopup("Tags");
    if (ImGui::BeginPopupModal("Tags", &EditorInspector::Instance().isAddTagPanel, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {

        Tags& tags = IDENTIFIERS.GetTags();

        if (ImGui::BeginTable("Tags", 3)) {
            ImGui::TableSetupColumn("1", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("2", ImGuiTableColumnFlags_WidthFixed, 250.0f);
            float cellpadding = 15.f;
            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(cellpadding, cellpadding));
            int i = 0;
            for (auto& tag : tags) {
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                std::string tag_id = "Tag " + std::to_string(i++);
                ImGui::Text(tag_id.c_str());

                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                if (tag.second == -1) //deleted tag
                    ImGui::Text("(Removed)");
                else
                    ImGui::Text(tag.first.c_str());

                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                std::string tag_name = tag.first;
                if (tag_name != "Untagged") {
                    ImGui::PushID(i);
                    if (ImGui::Button("-")) {
                        IDENTIFIERS.DeleteTag(tag.first);
                    }
                    ImGui::PopID();
                }
            }
            ImGui::PopStyleVar();
            ImGui::EndTable();
        }
        std::string newname;
        ImGui::Text("New Tag Name"); ImGui::SameLine();
        ImGui::InputText("##tag", &newname);
        if (CENTERED_CONTROL(ImGui::Button("Add"))) {
            IDENTIFIERS.CreateTag(newname);
        };
        ImGui::EndPopup();
    }
}

void AddLayerPanel() {
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(480, 680));

    //press esc to exit add component window
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        EditorInspector::Instance().isAddLayerPanel = false;
    }
    ImGui::OpenPopup("Layers");
    if (ImGui::BeginPopupModal("Layers", &EditorInspector::Instance().isAddLayerPanel, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
        std::string newname;
        /*ImGui::Text("New Layer Name"); ImGui::SameLine();
        ImGui::InputText("##layer", &newname);*/
        if (ImGui::BeginTable("Layers", 2)) {
            ImGui::TableSetupColumn("1", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            float cellpadding = 15.f;
            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(cellpadding, cellpadding));
            for (int i = 0; i < MAX_PHYSICS_LAYERS; i++) {
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                std::string layer = "Layer " + std::to_string(i);      
                if (i <= 4)//default layers
                    layer += " (Default)";
                ImGui::Text(layer.c_str());

                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                std::string label = "##" + layer;
                std::string& layername = IDENTIFIERS.physicsLayers[i].name;
                ImGui::SetNextItemWidth(-FLT_MIN);
                if(i <= 4)
                    ImGui::Text(layername.c_str());
                else
                    ImGui::InputText(label.c_str(), &layername);
                ImGui::TableNextRow();
            }
            ImGui::PopStyleVar();
            ImGui::EndTable();
        }
        
        /*if (CENTERED_CONTROL(ImGui::Button("Add"))) {
            IDENTIFIERS.CreateLayer(newname);
            EditorInspector::Instance().isAddLayerPanel = false;
        };*/
        ImGui::EndPopup();
    }
}

void DisplayLayers(Entity& entity) {
    Scene& curr_scene = SceneManager::Instance().GetCurrentScene();

    //Entity current tag
    auto& tag = curr_scene.Get<Tag>(entity);

    std::vector<const char*> layers;
    int i = 0;
    for (auto& it : IDENTIFIERS.physicsLayers) {
        if (it.name.empty()) {
            i++;
            continue;
        }        
        layers.push_back(it.name.c_str());
    }

    int index = (int)tag.physicsLayerIndex;
    ImGui::Text("Layer"); ImGui::SameLine();
    ImGui::PushItemWidth(100.f);
    Change change(&tag, "Tag/Layer Index");
    if (ImGui::Combo("##Layer", &index, layers.data(), (int)layers.size(), 5))
    {
        size_t buf = (size_t)index;
        EDITOR.History.SetPropertyValue(change, tag.physicsLayerIndex, buf);
    }
        //tag.physicsLayerIndex = index;
    ImGui::PopItemWidth();
}

void DisplayTags(Entity& entity) {
    Scene& curr_scene = SceneManager::Instance().GetCurrentScene();

    //Entity current tag
    auto& tag = curr_scene.Get<Tag>(entity);

    //tags in the project
    auto& Tags = IDENTIFIERS.GetTags();

    std::vector<const char*> layers;
    int i = 0;
    int index = 0; //default
    bool found = false;

    for (auto& it : Tags) {
        if (it.second == tag.tagName) {
            index = i;
        }
        layers.push_back(it.first.c_str());
        i++;
    }

    ImGui::Text("Tags"); ImGui::SameLine();
    ImGui::PushItemWidth(100.f);
    Change change(&tag, "Tag/Tag Name");
    if(ImGui::Combo("##Tags", &index, layers.data(), (int)layers.size(), 5)){

        Engine::UUID buf = Tags[layers[index]];
        EDITOR.History.SetPropertyValue(change, tag.tagName, buf);
        //tag.tagName = Tags[layers[index]];
    }

    ImGui::PopItemWidth();
}

//Display all the components as well as the name and whether the entity is enabled in the scene.
void DisplayEntity(Entity& entity)
{
    ImGui::PushID((int)entity.EUID());

    Scene& curr_scene = SceneManager::Instance().GetCurrentScene();
    bool enabled = curr_scene.IsActive(entity, false);
    ImGui::Checkbox("##Active", &enabled);
    curr_scene.SetActive(entity, enabled);
    ImGui::SameLine();
    static char buffer[256];

    auto& tag = curr_scene.Get<Tag>(entity);
    Change change(&tag, "Tag/name");

    strcpy_s(buffer, tag.name.c_str());
    ImGui::PushItemWidth(-1);
    if (ImGui::InputText("##gameObjName", buffer, 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
        std::string buf = buffer;
        EDITOR.History.SetPropertyValue(change, tag.name, buf);
    }

    ImGui::PopItemWidth();

    //display tags
    DisplayTags(entity);
    ImGui::SameLine();
    ImGui::PushID(1);
    if (ImGui::Button("+")) { EditorInspector::Instance().isAddTagPanel = true; }
    ImGui::PopID();
    ImGui::SameLine(); ImGui::Dummy(ImVec2(22.f, 0.f)); ImGui::SameLine();

    //display layers
    DisplayLayers(entity); ImGui::SameLine();
    if (ImGui::Button("+")) { EditorInspector::Instance().isAddLayerPanel = true; }


    ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_ScrollY;

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.f);
 
    if (ImGui::BeginTable("Components", 1, tableFlags))
    {

        DisplayComponents(entity);
        ImGui::Separator();
        if (CENTERED_CONTROL(ImGui::Button("Add Component", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, ImGui::GetTextLineHeightWithSpacing())))) {
            EditorInspector::Instance().isAddComponentPanel = true;
        }
        ImGui::EndTable();

    }

    ImGui::PopID();
    ImGui::PopStyleVar();
}

void EditorInspector::Init()
{
    isAddComponentPanel = false;
    //default layers (same as unity)
}

enum MODEL_STATE { MODEL, ANIMATION };

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

    if (EditorScene::Instance().multiselectEntities.size() > 1) {
        DisplayMultiTransform();
    }
    else if (&curr_entity) {
        ImGui::Spacing();
        std::string Header = "Current Entity: " + curr_scene.Get<Tag>(curr_index).name;
        ImGui::Text(Header.c_str()); ImGui::Spacing(); ImGui::Separator();
        DisplayEntity(curr_entity);
    }

    if (isAddComponentPanel) {
        AddComponentPanel(curr_scene.Get<Entity>(curr_index));
    }

    if (material_inspector) {

        ImGui::Begin("Material", &material_inspector);

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(6, 2));
        
        ImGuiWindowFlags tableflags = ImGuiTableFlags_Resizable | ImGuiTableFlags_NoBordersInBody
            | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_SizingStretchProp
            | ImGuiTableFlags_PadOuterX;

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(450.f, 600.f));

        auto& material = MATERIALSYSTEM.getMaterialInstance(EditorContentBrowser::Instance().selectedAss);

        ImGui::Dummy(ImVec2(0, 10.f));

        if (ImGui::BeginTable("Mats", 2, tableflags))
        {
            ImGui::Indent();
            ImGui::TableSetupColumn("Text", 0, 0.4f);
            ImGui::TableSetupColumn("Input", 0, 0.6f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 0));

            property::SerializeEnum(material, [&](std::string_view PropertyName, property::data&& Data, const property::table&, std::size_t, property::flags::type Flags)
                {
                    if (!Flags.m_isDontShow) {
                        auto entry = property::entry { PropertyName, Data };
                        std::visit([&](auto& Value) {

                            using T1 = std::decay_t<decltype(Value)>;

                            //Edit name
                            std::string DisplayName = entry.first;
                            
                            auto it = DisplayName.begin() + DisplayName.find_last_of("/");
                            DisplayName.erase(DisplayName.begin(), ++it);
                            Change newchange(&material, entry.first);
                            ImGui::PushID(entry.first.c_str());

                            if (entry.first.find("Shader") != std::string::npos) {
                                DisplayShaders(newchange, Value);
                            }
                            else {                            
                                 Display<T1>(newchange, DisplayName.c_str(), Value);     
                            }

                            ImGui::PopID();
                            }
                        , Data);
                        property::set(material, entry.first.c_str(), Data);

                        // If we are dealing with a scope that is not an array someone may have change the SerializeEnum to a DisplayEnum they only show up there.
                        //assert(Flags.m_isScope == false || PropertyName.back() == ']');
                    }

                });

            ImGui::PopStyleVar();
            ImGui::PopStyleVar();
            ImGui::PopStyleVar();

            ImGui::Unindent();
            ImGui::EndTable();
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();

        ImGui::Separator();

        if (CENTERED_CONTROL(ImGui::Button("Save Material", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, ImGui::GetTextLineHeightWithSpacing()))))
        {
            GetFilePathEvent<MaterialAsset> e{ EditorContentBrowser::Instance().selectedAss };
            EVENTS.Publish(&e);
            std::string fPathStr{ e.filePath.string() };
            size_t strPos = fPathStr.find_last_of("\\");
            if (strPos == std::string::npos)
                strPos = fPathStr.find_last_of("/");
            fs::path fPath;
            if (strPos != std::string::npos)
            {
                fPathStr = std::string(fPathStr.begin(), fPathStr.begin() + strPos);
                fPath = fPathStr;
            }
            if (!fPath.empty())
            {
                std::string directory = fPathStr;
                fPath += "\\";
                fPath += material.name.c_str();
                fPath += ".material";
                fs::rename(e.filePath, fPath);
                Serialize(material, fPathStr);
            }
            else
            {
                fPath = "Assets\\";
                fPath += material.name.c_str();
                fPath += ".material";
                Serialize(material);
                GetAssetEvent<MaterialAsset> pathEvent(fPath);
                EVENTS.Publish(&pathEvent);
                EditorContentBrowser::Instance().selectedAss = e.guid;
            }
            PRINT(fPath);
        }

        ImGui::End();
    }

    

    if (model_inspector) {

        ImGui::Begin("Model", &model_inspector);
        ImGui::Dummy(ImVec2(0, 20.f));

        ImVec2 buttonSize = { 80.f, ImGui::GetTextLineHeight() + 10.f };

        ImGuiStyle& style = ImGui::GetStyle();
        float width = 0.0f;
        width += buttonSize.x * 2;
        width += style.ItemSpacing.x;
        AlignForWidth(width);

        static bool Model = true;

        ImVec4 normal = ImVec4(0.6, 0.6, 0.6, 1.f);
        ImVec4 selected = ImVec4(0.27f, 0.525f, 0.706f, 1.f);

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(2, 4));

        ImGuiWindowFlags tableflags = ImGuiTableFlags_Resizable | ImGuiTableFlags_NoBordersInBody
            | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_SizingStretchProp
            | ImGuiTableFlags_PadOuterX;

       
        ImGui::PushStyleColor(ImGuiCol_Button, Model ? selected : normal); 
        if (ImGui::Button("Model", buttonSize))
            Model = true;
        ImGui::SameLine();
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Button, Model ? normal : selected);
        if (ImGui::Button("Animation", buttonSize))
            Model = false;
        ImGui::PopStyleColor();

        ImGui::Dummy(ImVec2(0, 10.f));
        ImGui::Separator();

        /*for (auto& anim_guid : model->animations) {
            GetAssetByGUIDEvent<AnimationAsset> anim{ anim_guid };
            EVENTS.Publish(&anim);
            anim.asset->ticksPerSecond;
        }*/

        //Get current model asset
        GetAssetByGUIDEvent<ModelAsset> e{ EditorContentBrowser::Instance().selectedAss };
        EVENTS.Publish(&e);
        auto model = e.importer;

        if (ImGui::BeginTable("Model", 4, tableflags))
        {
            ImGui::Indent();
            ImGui::TableSetupColumn("Text", ImGuiTableColumnFlags_WidthFixed, 150.f);
            ImGui::TableSetupColumn("Input", ImGuiTableColumnFlags_WidthFixed, 100.f);
            ImGui::TableSetupColumn("Input2", ImGuiTableColumnFlags_WidthFixed, 100.f);

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 0));


            if (Model) {
                ImGui::AlignTextToFramePadding();
                ImGui::TableNextColumn();
                ImGui::Text("Scale Factor");
                ImGui::TableNextColumn();
                ImGui::DragFloat("##scalefloat", &model->scaleFactor, 0.01f);
            }
            else { //Animation         
                auto& anims = model->animations;

                //Get current (first) animation asset
                if (!anims.empty()) {
                    auto& anim_states = model->animationStates;

                    GetAssetByGUIDEvent<AnimationAsset> anim{ model->animations[0] };
                    EVENTS.Publish(&anim);                    

                    ImGui::AlignTextToFramePadding();
                    ImGui::TableNextColumn();
                    ImGui::Text("Duration");
                    ImGui::TableNextColumn();
                    int dur = (int)anim.asset->duration;
                    std::string duration = std::to_string(dur);
                    ImGui::Text(duration.c_str());
                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    ImGui::Text("Ticks Per Second");
                    ImGui::TableNextColumn();
                    std::string tickspersecond = std::to_string(anim.asset->ticksPerSecond);
                    ImGui::Text(tickspersecond.c_str());
                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    ImGui::Text("Animation States");
                    ImGui::TableNextColumn();
                    if (ImGui::Button("+")) {
                        anim_states.push_back(AnimationState());
                    }

                    int i = 0;
                    for (auto& state : model->animationStates) {
                        ImGui::PushID(i);
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("Label"); ImGui::SameLine();
                        ImGui::InputText("##Label", &state.label);
                        ImGui::TableNextColumn();
                        ImGui::Text("Min"); ImGui::SameLine();
                        ImGui::InputFloat("##Min", &state.minMax.x);
                        ImGui::TableNextColumn();
                        ImGui::Text("Max"); ImGui::SameLine();
                        ImGui::InputFloat("##Max", &state.minMax.y);
                        ImGui::SameLine();
                        ImGui::TableNextColumn();
                        if (ImGui::Button("-")) {
                            auto& states = model->animationStates;
                            states.erase(states.begin() + i);
                        }
                        ImGui::PopID();
                        i++;
                    }                  
                }
                else {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("No animation found");
                }

               
            }

            ImGui::PopStyleVar();
            ImGui::PopStyleVar();
            ImGui::PopStyleVar();

            ImGui::Unindent();
            ImGui::EndTable();
        }

        ImGui::Separator();

        if (CENTERED_CONTROL(ImGui::Button("Save Model", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, ImGui::GetTextLineHeightWithSpacing()))))
        {
            GetFilePathEvent<ModelAsset> modelpath{ EditorContentBrowser::Instance().selectedAss };
            EVENTS.Publish(&modelpath);

            std::string modelPath{ modelpath.filePath.string() + ".meta"};

            Serialize(modelPath, *model);
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleVar();

        

        ImGui::End();
    }


    if (isAddTagPanel) {
        AddTagPanel();
    }

    if (isAddLayerPanel) {
        AddLayerPanel();
    }

    ImGui::PopStyleVar();
    ImGui::End();
}

void EditorInspector::Exit()
{

}