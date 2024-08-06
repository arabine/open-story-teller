
#include <sstream>
#include "function_node_widget.h"

namespace ed = ax::NodeEditor;
#include "IconsMaterialDesignIcons.h"
#include "story_project.h"
#include "uuid.h"

FunctionNodeWidget::FunctionNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node)
    : BaseNodeWidget(manager, node)
    , m_manager(manager)
{
    // Create defaut one input and one output
    AddInput();
    AddOutputs(2);
    SetOutPinName(0, "Success");
    SetOutPinName(1, "Failure");
    m_functionUuid = Uuid().String();
}

void FunctionNodeWidget::Draw()
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

void FunctionNodeWidget::Initialize()
{
    BaseNodeWidget::Initialize();
    m_functionName = "Function";
}

void FunctionNodeWidget::DrawProperties()
{
    
    

}


