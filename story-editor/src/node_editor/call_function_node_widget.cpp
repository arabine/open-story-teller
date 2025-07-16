
#include <sstream>
#include "call_function_node_widget.h"

namespace ed = ax::NodeEditor;
#include "IconsMaterialDesignIcons.h"
#include "story_project.h"
#include "uuid.h"

CallFunctionNodeWidget::CallFunctionNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node)
    : BaseNodeWidget(manager, node)
    , m_manager(manager)
{
    // Create defaut one input and one output
    AddInputs(1);
    AddOutputs(2);
    SetOutPinName(0, "Success");
    SetOutPinName(1, "Failure");
    m_functionUuid = Uuid().String();
}

void CallFunctionNodeWidget::Draw()
{
    BaseNodeWidget::FrameStart();

    ImGui::TextUnformatted(m_functionName.c_str());
    if (ImGui::Button("> Open function"))
    {
        m_manager.OpenFunction(m_functionUuid, m_functionName);
    }

    DrawPins();

    BaseNodeWidget::FrameEnd();

}

void CallFunctionNodeWidget::Initialize()
{
    BaseNodeWidget::Initialize();
    m_functionName = "Function";
}

void CallFunctionNodeWidget::DrawProperties()
{
    
    

}


