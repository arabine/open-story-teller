#include "main_window.h"
#include <filesystem>
#include <random>

#ifdef USE_WINDOWS_OS
#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <stdio.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#endif



MainWindow::MainWindow()
    : m_resourcesWindow(m_project)
{
    m_project.Clear();
}

MainWindow::~MainWindow()
{

}



void MainWindow::SetupMainMenuBar()
{
    bool showAboutPopup = false;
    bool showParameters = false;
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Paramètres"))
            {
                showParameters = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("About"))
            {
                showAboutPopup = true;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (showAboutPopup)
    {
        ImGui::OpenPopup("AboutPopup");
    }

    if (showParameters)
    {
        ImGui::OpenPopup("Options");
    }

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    //ImVec2 parent_pos = ImGui::GetWindowPos();
    //ImVec2 parent_size = ImGui::GetWindowSize();
    //ImVec2 center(parent_pos.x + parent_size.x * 0.5f, parent_pos.y + parent_size.y * 0.5f);
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("AboutPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Moniteo");
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Platform");
//        ImGui::Text("%s", SDL_GetPlatform());
//        ImGui::Text("CPU cores: %d", SDL_GetCPUCount());
//        ImGui::Text("RAM: %.2f GB", SDL_GetSystemRAM() / 1024.0f);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Separator();

        ImGui::SameLine(300);
        if (ImGui::Button("Close", ImVec2(120, 40)))
        {
           ImGui::CloseCurrentPopup();
        }
       ImGui::EndPopup();
    }
}

void MainWindow::Initialize()
{
    // GUI Init
    gui.Initialize();
  //  gui.ApplyTheme();

    editor.Initialize();
    imgWindow.Initialize();
    m_nodeEditorWindow.Initialize();

    LoadParams();
}


void MainWindow::ShowOptionsWindow()
{
    static int pingState = 0;

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(600, 0.0f));
    if (ImGui::BeginPopupModal("Options", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::PushItemWidth(-1.0f);

        ImGui::Text("Adresse du serveur");
        ImGui::SameLine();
        ImGui::InputText("##addr",  mBufAddress, sizeof(mBufAddress));

        ImGui::Text("Chemin de récupération");
        ImGui::SameLine();
        ImGui::InputText("##rec_path",  mBufReceivePath, sizeof(mBufReceivePath));

        ImGui::Text("Chemin d'envoi des données");
        ImGui::SameLine();
        ImGui::InputText("##send_path",  mBufSendPath, sizeof(mBufSendPath));

        ImGui::PushItemWidth(100);
        ImGui::Text("Port");
        ImGui::SameLine();
        ImGui::InputText("##port",  mBufPort, sizeof(mBufPort), ImGuiInputTextFlags_CharsDecimal);

        float width = 50;
        ImGui::BeginGroup();
        ImGui::PushID("Zebra7500");
        ImGui::TextUnformatted("Adresse IP Zebra7500");
        ImGui::SameLine();
        for (int i = 0; i < 4; i++) {
            ImGui::PushItemWidth(width);
            ImGui::PushID(i);
            bool invalid_octet = false;
            if (octets[i] > 255) {
                // Make values over 255 red, and when focus is lost reset it to 255.
                octets[i] = 255;
                invalid_octet = true;
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            }
            if (octets[i] < 0) {
                // Make values below 0 yellow, and when focus is lost reset it to 0.
                octets[i] = 0;
                invalid_octet = true;
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
            }
            ImGui::InputInt("##v", &octets[i], 0, 0, ImGuiInputTextFlags_CharsDecimal);
            if (invalid_octet) {
                ImGui::PopStyleColor();
            }
            ImGui::SameLine();
            ImGui::PopID();
            ImGui::PopItemWidth();
        }
        ImGui::PopID();
        ImGui::EndGroup();

        // Example action button and way to build the IP string
        ImGui::SameLine();

        ImGui::SameLine();
        if (pingState == 1)
        {
            ImGui::TextUnformatted("Ping en cours...");
        }
        else if (pingState == 2)
        {
            ImGui::TextUnformatted("Ping succès!");
        }
        else if (pingState == 3)
        {
            ImGui::TextUnformatted("Ping erreur :(");
        }
        else
        {
            ImGui::TextUnformatted("");
        }

        ImGui::Separator();


        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

bool MainWindow::ShowQuitConfirm()
{
    bool quitRequest = false;
    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
   // ImGui::SetNextWindowSize(ImVec2(200, 150));
    if (ImGui::BeginPopupModal("QuitConfirm", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Voulez-vous vraiment quitter le logiciel ?");
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            quitRequest = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    return quitRequest;
}


void MainWindow::Loop()
{
    // Main loop
    bool done = false;

    while (!done)
    {
        bool aboutToClose = gui.PollEvent();

        gui.StartFrame();

        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
        SetupMainMenuBar();

        console.Draw("Console", nullptr);
        imgWindow.Draw("ImageWindow", nullptr);
        editor.Draw("Code Editor", nullptr);

        m_resourcesWindow.Draw("Resources", nullptr);
        m_nodeEditorWindow.Draw("Blueprint", nullptr);
        ShowOptionsWindow();

        if (aboutToClose)
        {
             ImGui::OpenPopup("QuitConfirm");
        }
        if (ShowQuitConfirm())
        {
            done = true;
        }

        gui.EndFrame();


    }

    gui.Destroy();
}

void MainWindow::SaveParams()
{

}

void MainWindow::LoadParams()
{

}
