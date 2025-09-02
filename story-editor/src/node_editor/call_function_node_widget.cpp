
#include <sstream>
#include "call_function_node_widget.h"

#include "IconsMaterialDesignIcons.h"
#include "story_project.h"
#include "uuid.h"

CallFunctionNodeWidget::CallFunctionNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node)
    : BaseNodeWidget(manager, node)
    , m_manager(manager)
{
    m_functionUuid = Uuid().String();
}

void CallFunctionNodeWidget::Draw()
{
    ImGui::TextUnformatted(m_functionName.c_str());
    if (ImGui::Button("> Open function"))
    {
        m_manager.OpenFunction(m_functionUuid, m_functionName);
    }
}

void CallFunctionNodeWidget::Initialize()
{
    BaseNodeWidget::Initialize();
    m_functionName = "Function";
}

void CallFunctionNodeWidget::DrawProperties(std::shared_ptr<IStoryProject> story)
{
    
    

}


