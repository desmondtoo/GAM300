/*!***************************************************************************************
\file			EditorHeaders.h
\project
\author         Sean Ngo
\co-author      Joseph Ho

\par			Course: GAM300
\date           04/09/2023

\brief
    This file contains the declarations of the following:
    1. All windows in the editor system

All content � 2023 DigiPen Institute of Technology Singapore. All rights reserved.
******************************************************************************************/
#ifndef EDITORHEADERS_H
#define EDITORHEADERS_H

#include <glm/vec2.hpp>
#include <unordered_map>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"
#include <implot.h>
#include <implot_internal.h>

#include "Core/SystemInterface.h"
#include "Utilities/SparseSet.h"
#include "Utilities/PlatformUtils.h"
#include "Scene/Entity.h"
#include "Core/Events.h"
#include <imgui_node_editor.h>

#define NON_VALID_ENTITY 0
#define GET_TEXTURE_ID(filepath) TextureManager.GetTexture(filepath);
#define FIND_TEXTURE(filepath) TextureManager.FindTexture()

//types of files that can be dragged drop from the content browser
enum filetype {
    NONE, MESH, PREFAB, MATERIAL, MODELTYPE
};

//an object containing the data needed for the payload
struct ContentBrowserPayload {  

    ContentBrowserPayload() { type = NONE; }
    ContentBrowserPayload(filetype _type, Engine::HexID _guid) : type(_type), guid(_guid) {}

    Engine::HexID guid;
    filetype type;
    char* name;
};

struct BaseCamera;

// utility structure for realtime plot
struct ScrollingBuffer {
    int MaxSize;
    int Offset;
    ImVector<ImVec2> Data;
    ScrollingBuffer(int max_size = 2000) {
        MaxSize = max_size;
        Offset = 0;
        Data.reserve(MaxSize);
    }
    void AddPoint(float x, float y) {
        if (Data.size() < MaxSize)
            Data.push_back(ImVec2(x, y));
        else {
            Data[Offset] = ImVec2(x, y);
            Offset = (Offset + 1) % MaxSize;
        }
    }
    void Erase() {
        if (Data.size() > 0) {
            Data.shrink(0);
            Offset = 0;
        }
    }
};

struct layer {
    layer(std::string _name) : name(_name) {}
    std::string name;
};

struct tag {
    //create new tag
    tag(std::string _name) : name(_name) { uuid = Engine::CreateUUID(); }
    std::string name;
    Engine::UUID uuid;
};

ENGINE_EDITOR_SYSTEM(EditorMenuBar)
{
public:
    // Initializing the Menu Bar
    void Init();

    // Updating and displaying of the Menu Bar
    void Update(float dt);

    // Exit the system
    void Exit();

    // Open a scene file
    void OpenFile();

    // Create a new scene
    void NewScene();

    // Save the current loaded scene
    void SaveScene();

private:
    bool exitApp = false;
};

ENGINE_EDITOR_SYSTEM(EditorToolBar)
{
public:
    // Initializing the Menu Bar
    void Init();

    // Updating and displaying of the Menu Bar
    void Update(float dt);

    // Exit the system
    void Exit();

    void CallbackStopPreview(StopPreviewEvent* pEvent);
private:
};

ENGINE_EDITOR_SYSTEM(EditorHierarchy)
{
public:
    // Initializing the Hierarchy
    void Init();

    // Updating and displaying of the Hierarchy
    void Update(float dt);

    // Exit the system
    void Exit();

    void DisplayEntity(Engine::UUID euid);
    //void DisplayChildren(const ObjectIndex& Parent);
    Engine::UUID selectedEntity;
    bool newselect = false;
    bool initLayer = true;
    bool movetoitem = true;
private:
    void CallbackSelectedEntity(SelectedEntityEvent* pEvent);
};


ENGINE_EDITOR_SYSTEM(EditorContentBrowser)
{
public:
    // Initializing the Content Browser
    void Init();

    // Updating and displaying of the Content Browser
    void Update(float dt);

    // Exit the system
    void Exit();

    void CallbackGetCurrentDirectory(EditorGetCurrentDirectory* pEvent);

    Engine::HexID selectedAss;

    bool setPayload;

private:
    std::filesystem::path currentDirectory;
};


ENGINE_EDITOR_SYSTEM(EditorScene)
{
public:
    // Initializing the Scene View
    void Init();

    // Updating and displaying of the Scene View
    void Update(float dt);

    bool SelectEntity();

    void ToolBar();
    
    void SceneView();
    
    void DisplayGizmos();

    int GetCoordSelectionMode() { return coord_selection; }

    // Exit the system
    void Exit();

    // Getters for the data members
    glm::vec2 const GetDimension() { return sceneDimension; }
    glm::vec2 const GetPosition() { return scenePosition; }
    bool const WindowOpened() { return windowOpened; }
    bool const WindowHovered() { return windowHovered; }
    bool const WindowFocused() { return windowFocused; }
    bool const UsingGizmos() { return inOperation; }
    bool const DebugDraw() { return debug_draw; }

    void CallbackEditorWindow(EditorWindowEvent* pEvent);
    void ClearMultiselect();

    //Multiselect variables
    std::list<Engine::UUID>multiselectEntities;
    Transform multiTransform; 
    MeshRenderer multiMeshRenderer;
    bool useMeshRenderer = true;


private:
    glm::vec2 sceneDimension{}; // Dimensions of the viewport
    glm::vec2 scenePosition{};  // Position of the viewport relative to the engine
    glm::vec2 min{}, max{};     // Minimum and maximum position of the viewport
    bool windowOpened = false;
    bool windowHovered = false;
    bool windowFocused = false;
    bool inOperation = false;
    bool debug_draw = false;

    int coord_selection = 1;
};

ENGINE_EDITOR_SYSTEM(EditorGame)
{
public:
    // Initializing the Game View
    void Init();

    // Updating and displaying of the Game View
    void Update(float dt);

    void ToolBar();

    void UpdateTargetDisplay();

    void GameView();

    // Exit the system
    void Exit();

    // Getters for the data members
    glm::vec2 const GetDimension() { return dimension; }
    glm::vec2 const GetPosition() { return position; }
    bool const WindowOpened() { return windowOpened; }
    bool const WindowHovered() { return windowHovered; }
    bool const WindowFocused() { return windowFocused; }

    void CallbackEditorWindow(EditorWindowEvent* pEvent);
    void CallbackSetCamera(ObjectCreatedEvent<Camera>* pEvent);
    void CallbackDeleteCamera(ObjectDestroyedEvent<Camera>* pEvent);
    void CallbackSceneChange(SceneChangingEvent* pEvent);
    void CallbackSceneStop(SceneStopEvent* pEvent);

private:

    struct DisplayTarget
    {
        unsigned int targetDisplay = 0;
        std::string name;
    };

    BaseCamera* camera = nullptr;
    DisplayTarget displayTargets[8];
    glm::vec2 dimension{};          // Dimensions of the viewport
    glm::vec2 position{};           // Position of the viewport relative to the engine
    glm::vec2 min{}, max{};         // Minimum and maximum position of the viewport
    unsigned int targetDisplay = 0;
    float padding = 4.f;
    float AspectRatio = 16.f / 9.f;
    bool windowOpened = false;
    bool windowHovered = false;
    bool windowFocused = false;
};

ENGINE_EDITOR_SYSTEM(EditorInspector)
{
public:
    // Initializing the Inspector
    void Init();

    // Updating and displaying of the Inspector
    void Update(float dt);

    // Exit the system
    void Exit();

    bool isAddComponentPanel;
    bool isAddTagPanel;
    bool isAddLayerPanel;

    bool material_inspector;
    bool model_inspector;

    //std::vector<layer> Layers;
    //std::vector<std::string> Tags;

private:
};

ENGINE_EDITOR_SYSTEM(EditorDebugger)
{
public:

    // Initializing the Debugger
    void Init();

    // Updating and displaying of the Debugger
    void Update(float dt);

    // Exit the system
    void Exit();

    void Clear();

    void AddLog(const char* fmt, ...) IM_FMTARGS(2);

    void Draw();

    ImGuiTextBuffer& GetBuffer() { return Buffer; }
    ImVector<int>& GetLineOffset() { return LineOffsets; }

    int debugcounter;

private:
    ImGuiTextBuffer     Buffer;
    ImGuiTextFilter     Filter;
    ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
    bool                AutoScroll;  // Keep scrolling if already at the bottom.

};

ENGINE_EDITOR_SYSTEM(EditorPerformanceViewer)
{
public:
    // Initializing the Performance Viewer
    void Init();

    // Updating and displaying of the Performance Viewer
    void Update(float dt);

    // Exit the system
    void Exit();

    std::map<std::string, ScrollingBuffer>system_plots;
};

namespace ed = ax::NodeEditor;


ENGINE_EDITOR_SYSTEM(EditorBehaviourTreeEditor)
{
    public:
        // Initializing the Performance Viewer
        void Init();

        // Updating and displaying of the Performance Viewer
        void Update(float dt);

        // Exit the system
        void Exit();

        enum class NodeType {
            ControlFlow,
            Decorator,
            Leaf
        };

        struct Node {
            ed::NodeId id;
            NodeType type;
            ImVec2 position;
            std::string filename;
            ed::PinId inputPinId;  // Add pin IDs as needed
            ed::PinId outputPinId;

            ImU32 color;  // Add color attribute

            Node(int& _id, NodeType _type, std::string _filename, const ImVec2& _position)
                : id(_id), type(_type), filename(_filename), position(_position), inputPinId(0), outputPinId(0) {}
        };

        struct ControlFlowNode : public Node {
            ControlFlowNode(int& id, std::string _filename, const ImVec2& position)
                : Node(id, NodeType::ControlFlow, _filename, position) {
                inputPinId = id++;
                outputPinId = id++;
                color = IM_COL32(255, 0, 0, 255);
            }
        };

        struct DecoratorNode : public Node {
            DecoratorNode(int& id, std::string _filename, const ImVec2& position)
                : Node(id, NodeType::Decorator, _filename, position) {
                inputPinId = id++;
                outputPinId = id++;
                color = IM_COL32(0, 255, 0, 255);  // Green for Decorator nodes
            }
        };

        struct LeafNode : public Node {
            LeafNode(int& id, std::string _filename, const ImVec2& position)
                : Node(id, NodeType::Leaf, _filename, position) {
                inputPinId = id++;
                outputPinId = id++;
                color = IM_COL32(0, 0, 255, 255);  // Blue for Leaf nodes
            }
        };

        struct LinkInfo {
            ed::LinkId Id;
            ed::PinId InputId;
            ed::PinId OutputId;
        };

        bool editorOpen = true;
        ed::EditorContext* m_Context = nullptr;

    private: 
        int uniqueId = 1;
        bool m_FirstFrame = true;
        ImVector<LinkInfo> m_Links;
        int m_NextLinkId = 100;
        std::vector<Node> m_Nodes;

        Node CreateNode(int& id, NodeType type, const std::string & filename, const ImVec2 & position);
        void DrawNode(Node& node);

        void DrawNodeHierarchyWindow();
        void DrawNodeHierarchy(Node& currentNode, int depth);
        //const Node& GetNodeById(ed::PinId nodeId);

};

#endif // !EDITORHEADERS_H