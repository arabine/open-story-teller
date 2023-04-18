#ifndef RESOURCESDOCK_H
#define RESOURCESDOCK_H

#include <QDockWidget>
#include "ui_ost-resources.h"
#include "resource_model.h"

class ResourcesDock : public QDockWidget
{
    Q_OBJECT
public:
    ResourcesDock();

    QList<Resource> GetResources() const { return m_resourcesModel.GetData(); }

private:
    // Resources
    Ui::ostResources m_uiOstResources;
    ResourceModel m_resourcesModel;
};

#endif // RESOURCESDOCK_H
