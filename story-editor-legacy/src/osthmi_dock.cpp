#include "osthmi_dock.h"

#include <QPixmap>

OstHmiDock::OstHmiDock()
    : DockWidgetBase(tr("StoryTeller HMI"), true)
{
    setObjectName("OstHmiDock"); // used to save the state
    m_uiOstDisplay.setupUi(this);

    connect(m_uiOstDisplay.okButton, &QPushButton::clicked, this, &OstHmiDock::sigOkButton);
    connect(m_uiOstDisplay.leftButton, &QPushButton::clicked, this, &OstHmiDock::sigLeftButton);
    connect(m_uiOstDisplay.rightButton, &QPushButton::clicked, this, &OstHmiDock::sigRightButton);
}

void OstHmiDock::SetImage(const QString &fileName)
{
    m_uiOstDisplay.display->setPixmap(QPixmap(fileName));
}

void OstHmiDock::ClearImage()
{
    m_uiOstDisplay.display->clear();
}
