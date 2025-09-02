#include "base_node_widget.h"
#include "uuid.h"
#include "imgui.h"
#include "IconsMaterialDesignIcons.h"

unsigned long BaseNodeWidget::s_nextId = 1;

BaseNodeWidget::BaseNodeWidget(IStoryManager &manager,  std::shared_ptr<BaseNode> base)
    : m_manager(manager)
    , m_base(base)
{
    // A node is specific to the Node Gfx library
    std::cout << " --> Created node widget: " << m_base->GetId() << std::endl;
}

BaseNodeWidget::~BaseNodeWidget()
{
    
}


void BaseNodeWidget::Initialize()
{
    m_firstFrame = true;
    
}


