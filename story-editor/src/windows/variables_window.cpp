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

    m_story.ScanVariable([&i, this] (std::shared_ptr<Variable> var) {

        ImGui::PushID(static_cast<int>(i)); // Assure l'unicité des widgets
        std::string l = var->GetVariableName();
        if (ImGui::TreeNode((l + "###variable").c_str()))
        {
            // Modifier le nom de la variable
            static char buffer[Variable::NameMaxSize];
            std::strncpy(buffer, l.c_str(), sizeof(buffer));
            buffer[sizeof(buffer) - 1] = '\0'; // Assure la terminaison
            if (ImGui::InputText("Name", buffer, sizeof(buffer))) {
                var->SetVariableName(buffer);
            }

             // Choisir le type de la variable
            const char* types[] = {"Integer", "String"};
            static int selectedType = var->IsInteger() ? 0 : 1; // 0 for Integer, 1 for String
            if (ImGui::Combo("Type", &selectedType, types, IM_ARRAYSIZE(types))) {
                var->SetValueType(selectedType == 0 ? Variable::ValueType::INTEGER : Variable::ValueType::STRING);
            }

            if (var->IsInteger())
            {
                // Modifier l'échelle
                int scalePower = var->GetScalePower();
                if (ImGui::InputInt("Scale Power (10^x)", &scalePower))
                {
                    var->SetScalePower(scalePower);
                }

                // Modifier la valeur entière
                int intValue = static_cast<int>(var->GetIntegerValue());
                if (ImGui::InputInt("Integer Value", &intValue)) {
                    var->SetIntegerValue(static_cast<int64_t>(intValue));
                }

                // Afficher la valeur flottante calculée
                float floatValue = ScaledToFloat(var->GetIntegerValue(), var->GetScalePower());
                ImGui::Text("Float Value: %.6f", floatValue);
            }
            else
            {
                std::strncpy(buffer, var->GetStringValue().c_str(), sizeof(buffer));
                if (ImGui::InputText("Text value", buffer, sizeof(buffer)))
                {
                    var->SetTextValue(buffer);
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
