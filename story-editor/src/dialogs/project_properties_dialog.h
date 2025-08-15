// about_dialog.h
#ifndef PROJECT_PROPERTIES_DIALOG_H
#define PROJECT_PROPERTIES_DIALOG_H

#include "dialog_base.h" // Pour DialogBase
#include <string>   // Pour std::string
#include "library_manager.h" // Pour LibraryManager::GetVersion()
#include <SDL3/SDL.h> // Pour SDL_GetPlatform(), SDL_GetNumLogicalCPUCores(), SDL_GetSystemRAM()
#include "i_story_manager.h"
#include "IconsMaterialDesignIcons.h"

class ProjectPropertiesDialog : public DialogBase
{
public:
    ProjectPropertiesDialog(IStoryManager &story, ResourceManager &resources)
        : m_storyManager(story)
        , m_resources(resources)
    {
       m_project_name[0] = '\0';
    }

    // Affiche le popup "Project Properties".
    // Doit être appelée à chaque frame dans la boucle de rendu ImGui.
    void Draw()
    {
        auto story = m_storyManager.GetCurrentProject();
        if (story)
        {
            if (m_firstOpen)
            {
                ImVec2 center = ImGui::GetMainViewport()->GetCenter();
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                m_firstOpen = false;
                std::size_t length = story->GetName().copy(m_project_name, sizeof(m_project_name));
                m_project_name[length] = '\0';
            }
            DrawContent(story);
        }
        else
        {
            Reset();
            return;
        }

    }
private:
    IStoryManager &m_storyManager;
    ResourceManager &m_resources;
    char m_project_name[256] = "";
    

    // Implémentation de la méthode virtuelle pure GetTitle()
    const char* GetTitle() const override
    {
        return "ProjectPropertiesPopup";
    }


    void DrawContent(std::shared_ptr<IStoryProject> story)
    {
        if (ImGui::BeginPopupModal(GetTitle(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {

            ImGui::Text("Project name: "); ImGui::SameLine();
            ImGui::InputTextWithHint("##project_name", "Project name", m_project_name, IM_ARRAYSIZE(m_project_name));

            ImGui::Text("Size of display screen: ");
            ImGui::SameLine();

            static ImGuiComboFlags flags = 0;

            static int display_item_current_idx = 0; // Here we store our selection data as an index.
            static int image_item_current_idx = 0; // Here we store our selection data as an index.
            static int sound_item_current_idx = 0; // Here we store our selection data as an index.

            {
                // Using the generic BeginCombo() API, you have full control over how to display the combo contents.
                // (your selection data could be an index, a pointer to the object, an id for the object, a flag intrusively
                // stored in the object itself, etc.)
                const char* display_items[] = { "320x240", "640x480" };

                const char* combo_preview_value = display_items[display_item_current_idx];  // Pass in the preview value visible before opening the combo (it could be anything)
                if (ImGui::BeginCombo("##ComboDisplay", combo_preview_value, flags))
                {
                    for (int n = 0; n < IM_ARRAYSIZE(display_items); n++)
                    {
                        const bool is_selected = (display_item_current_idx == n);
                        if (ImGui::Selectable(display_items[n], is_selected))
                            display_item_current_idx = n;

                        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
            }

            ImGui::Text("Image format: ");
            ImGui::SameLine();
            {
                // Using the generic BeginCombo() API, you have full control over how to display the combo contents.
                // (your selection data could be an index, a pointer to the object, an id for the object, a flag intrusively
                // stored in the object itself, etc.)
                const char* image_items[] = { "Native (no conversion)", "QOIF (Quite Ok Image Format" };
                const char* image_combo_preview_value = image_items[image_item_current_idx];  // Pass in the preview value visible before opening the combo (it could be anything)
                if (ImGui::BeginCombo("##ComboImage", image_combo_preview_value, flags))
                {
                    for (int n = 0; n < IM_ARRAYSIZE(image_items); n++)
                    {
                        const bool is_selected = (image_item_current_idx == n);
                        if (ImGui::Selectable(image_items[n], is_selected))
                            image_item_current_idx = n;

                        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
            }


            ImGui::Text("Sound format: ");
            ImGui::SameLine();
            {
                // Using the generic BeginCombo() API, you have full control over how to display the combo contents.
                // (your selection data could be an index, a pointer to the object, an id for the object, a flag intrusively
                // stored in the object itself, etc.)
                const char* sound_items[] = { "Native (no conversion)", "WAV (16-bit stereo)", "QOAF (Quite Ok Audio Format" };
                const char* sound_combo_preview_value = sound_items[sound_item_current_idx];  // Pass in the preview value visible before opening the combo (it could be anything)
                if (ImGui::BeginCombo("##ComboSound", sound_combo_preview_value, flags))
                {
                    for (int n = 0; n < IM_ARRAYSIZE(sound_items); n++)
                    {
                        const bool is_selected = (sound_item_current_idx == n);
                        if (ImGui::Selectable(sound_items[n], is_selected))
                            sound_item_current_idx = n;

                        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
            }


            ImGui::AlignTextToFramePadding();
            ImGui::Text("Image");
            ImGui::SameLine();

            ImGui::Text("%s", story->GetTitleImage().c_str());

            ImGui::SameLine();

            static bool isImage = true;
            if (ImGui::Button("Select...##image")) {
                ImGui::OpenPopup("popup_button");
                isImage = true;
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_MDI_CLOSE_BOX_OUTLINE "##delimage")) {
                story->SetTitleImage("");
            }

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Sound");
            ImGui::SameLine();

            ImGui::Text("%s", story->GetTitleSound().c_str());

            ImGui::SameLine();

            if (ImGui::Button("Play " ICON_MDI_PLAY))
            {
                m_storyManager.PlaySoundFile(m_storyManager.BuildFullAssetsPath(story->GetTitleSound()));
            }

            ImGui::SameLine();

            if (ImGui::Button("Select...##sound")) {
                ImGui::OpenPopup("popup_button");
                isImage = false;
            }

            ImGui::SameLine();
            if (ImGui::Button(ICON_MDI_CLOSE_BOX_OUTLINE "##delsound")) {
                story->SetTitleSound("");
            }


            if (ImGui::BeginPopup("popup_button")) {
                ImGui::SeparatorText(isImage ? "Images" : "Sounds");

                auto [filtreDebut, filtreFin] = isImage ? m_resources.Images() : m_resources.Sounds();
                int n = 0;
                for (auto it = filtreDebut; it != filtreFin; ++it, n++)
                {
                    if (ImGui::Selectable((*it)->file.c_str()))
                    {
                        if (isImage)
                        {
                            story->SetTitleImage((*it)->file);
                        }
                        else
                        {
                            story->SetTitleSound((*it)->file);
                        }
                    }
                }

                ImGui::EndPopup(); // Note this does not do anything to the popup open/close state. It just terminates the content declaration.
            }


            auto GetImageFormat = [](int idx) -> Resource::ImageFormat
            {
                Resource::ImageFormat img{Resource::IMG_SAME_FORMAT};
                if (idx < Resource::IMG_FORMAT_COUNT) {
                    img = static_cast<Resource::ImageFormat>(idx);
                }
                return img;
            };

            auto GetSoundFormat = [](int idx) -> Resource::SoundFormat {

                Resource::SoundFormat img{Resource::SND_FORMAT_WAV};
                if (idx < Resource::IMG_FORMAT_COUNT) {
                    img = static_cast<Resource::SoundFormat>(idx);
                }

                return img;
            };


            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                if (display_item_current_idx == 0)
                {
                    story->SetDisplayFormat(320, 240);
                }
                else
                {
                    story->SetDisplayFormat(640, 480);
                }

                story->SetImageFormat(GetImageFormat(image_item_current_idx));
                story->SetSoundFormat(GetSoundFormat(sound_item_current_idx));
                story->SetName(m_project_name);

                m_storyManager.SaveProject();

                ImGui::CloseCurrentPopup();

            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
                Reset();
            }
            ImGui::EndPopup();
        }
    }
};

#endif // PROJECT_PROPERTIES_DIALOG_H
