/*!***************************************************************************************
\file			EditorDebugger.cpp
\project
\author         Joseph Ho

\par			Course: GAM300
\date           07/09/2023

\brief
    This file contains the definitions of the functions used for the Editor debugger window.

All content � 2023 DigiPen Institute of Technology Singapore. All rights reserved.
******************************************************************************************/

#include "Precompiled.h"
#include "EditorHeaders.h"
#include "Core/EventsManager.h"

void EditorDebugger::Init()
{
    AutoScroll = true;
    Clear();
}

void EditorDebugger::Update(float dt)
{
    UNREFERENCED_PARAMETER(dt);
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Logger");
    if (ImGui::SmallButton("[Debug] Test Debug Output"))
    {
        AddLog("[%i]{Info} This is a test Debug output!\n", debugcounter++);
    }
    // Actually call in the regular Log helper (which will Begin() into the same window as we just did)
    Draw();
    ImGui::End();
}

//Definition of the editor debugger window
void EditorDebugger::Draw()
{
    // Options menu
    if (ImGui::BeginPopup("Options"))
    {
        ImGui::Checkbox("Auto-scroll", &AutoScroll);
        ImGui::EndPopup();
    }

    // Main window
    if (ImGui::Button("Options"))
        ImGui::OpenPopup("Options");
    ImGui::SameLine();
    bool clear = ImGui::Button("Clear");
    ImGui::SameLine();
    Filter.Draw("Filter", -100.0f);

    ImGui::Separator();

    if (ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar))
    {
        if (clear)
            //Clear Debug window
            Clear();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        const char* buf = Buffer.begin();
        const char* buf_end = Buffer.end();

        //Allow user to filter and find debug messages using the filter buffer.
        if (Filter.IsActive())
        {
            for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
            {
                const char* line_start = buf + LineOffsets[line_no];
                const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                if (Filter.PassFilter(line_start, line_end))
                    ImGui::TextUnformatted(line_start, line_end);
            }
        }
        else
        {
            ImGuiListClipper clipper;
            clipper.Begin(LineOffsets.Size);
            while (clipper.Step())
            {
                for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                {
                    const char* line_start = buf + LineOffsets[line_no];
                    const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                    ImGui::TextUnformatted(line_start, line_end);
                }
            }
            clipper.End();
        }
        ImGui::PopStyleVar();

        //allow the debug window to automatically scroll to the latest message
        if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();
}

//using Variable Argument List in order to create and concatenate debug outputs
//To add logs to logger, use this format: Debug_Sys.AddLog("[%i]{Type}Input Debug Text Here!\n", Debug_Sys.Debug_Counter++);
void EditorDebugger::AddLog(const char* fmt, ...) IM_FMTARGS(2)
{
    int old_size = Buffer.size();
    va_list args;
    va_start(args, fmt);
    Buffer.appendfv(fmt, args);
    va_end(args);
    for (int new_size = Buffer.size(); old_size < new_size; old_size++)
        if (Buffer[old_size] == '\n')
            LineOffsets.push_back(old_size + 1);
}

//Clears the buffer of debug messages
void EditorDebugger::Clear()
{
    Buffer.clear();
    LineOffsets.clear();
    LineOffsets.push_back(0);
}

void EditorDebugger::Exit()
{

}