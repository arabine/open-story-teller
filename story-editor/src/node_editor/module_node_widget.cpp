
#include <sstream>
#include "module_node_widget.h"

namespace ed = ax::NodeEditor;
#include "IconsMaterialDesignIcons.h"
#include "story_project.h"
#include "uuid.h"

ModuleNodeWidget::ModuleNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node)
    : BaseNodeWidget(manager, node)
    , m_manager(manager)
{
    // Create defaut one input and one output
    AddInputs(1);
    AddOutputs(2);
    SetOutPinName(0, "Success");
    SetOutPinName(1, "Failure");
}

void ModuleNodeWidget::Draw()
{
    BaseNodeWidget::FrameStart();

    ImGui::TextUnformatted(m_functionName.c_str());


    DrawPins();

    BaseNodeWidget::FrameEnd();

}

void ModuleNodeWidget::Initialize()
{
    BaseNodeWidget::Initialize();
    m_functionName = "Function";
}

void ModuleNodeWidget::DrawProperties()
{
    
    

}


