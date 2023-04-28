#include "osthmi_dock.h"

#include <QPixmap>

OstHmiDock::OstHmiDock()
    : DockWidgetBase(tr("StoryTeller HMI"))
{
    setObjectName("OstHmiDock"); // used to save the state
    m_uiOstDisplay.setupUi(this);

    connect(m_uiOstDisplay.okButton, &QPushButton::clicked, this, &OstHmiDock::sigOkButton);
}

void OstHmiDock::SetImage(const QString &fileName)
{
    m_uiOstDisplay.display->setPixmap(QPixmap(fileName));
}
