#include "variables_window.h"
#include "gui.h"
#include "ImGuiFileDialog.h"
#include "IconsMaterialDesignIcons.h"
#include "chip32_vm.h"
#include "variable.h"

VariablesWindow::VariablesWindow(IStoryManager &proj)
    : WindowBase("Variables")
    , m_story(proj)
{

}

void VariablesWindow::Initialize()
{



}


// Fonction pour convertir un entier en float selon l'échelle
float ScaledToFloat(int64_t value, int scalePower) {
    return static_cast<float>(value) * std::pow(10.0f, scalePower);
}

// Fonction pour convertir un float en entier selon l'échelle
int64_t FloatToScaled(float floatValue, int scalePower) {
    return static_cast<int64_t>(floatValue / std::pow(10.0f, scalePower));
}


void VariablesWindow::ShowRAMEditor()
{

    if (ImGui::Button("Add Variable")) {
        // Ajouter une nouvelle variable par défaut
        m_story.AddVariable();
    }

    ImGui::Separator();
    int i = 0;

    m_story.ScanVariable([&i, this] (Variable &var) {

        ImGui::PushID(static_cast<int>(i)); // Assure l'unicité des widgets
        if (ImGui::TreeNode((var.name + "###variable").c_str()))
        {
            // Modifier le nom de la variable
            static char buffer[Variable::NameMaxSize];
            std::strncpy(buffer, var.name.c_str(), sizeof(buffer));
            buffer[sizeof(buffer) - 1] = '\0'; // Assure la terminaison
            if (ImGui::InputText("Name", buffer, sizeof(buffer))) {
                var.name = buffer;
            }

             // Choisir le type de la variable
            const char* types[] = {"Integer", "String"};
            static int selectedType = (var.type == "Integer") ? 0 : 1;
            if (ImGui::Combo("Type", &selectedType, types, IM_ARRAYSIZE(types))) {
                var.type = types[selectedType];
            }

            if (var.type == "Integer")
            {
                // Modifier l'échelle
                ImGui::InputInt("Scale Power (10^x)", &var.scalePower);

                // Modifier la valeur entière
                int intValue = static_cast<int>(var.value);
                if (ImGui::InputInt("Integer Value", &intValue)) {
                    var.value = static_cast<int64_t>(intValue);
                }

                // Afficher la valeur flottante calculée
                float floatValue = ScaledToFloat(var.value, var.scalePower);
                ImGui::Text("Float Value: %.6f", floatValue);
            }
            else
            {
                std::strncpy(buffer, var.valueText.c_str(), sizeof(buffer));
                if (ImGui::InputText("Text value", buffer, sizeof(buffer)))
                {
                    var.valueText = buffer;
                }
            }

            // Bouton pour supprimer la variable
            if (ImGui::Button("Delete")) {
                m_story.DeleteVariable(i);
                ImGui::TreePop();
                ImGui::PopID();

            }

            ImGui::TreePop();
        }
        ImGui::PopID();
        i++;

    });
}

void VariablesWindow::Draw()
{  
    WindowBase::BeginDraw();


    ShowRAMEditor();

    WindowBase::EndDraw();
}
