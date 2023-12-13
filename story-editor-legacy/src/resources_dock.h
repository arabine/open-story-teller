#ifndef RESOURCESDOCK_H
#define RESOURCESDOCK_H

#include "dock_widget_base.h"
#include "ui_ost-resources.h"
#include "resource_model.h"
#include <filesystem>

class ResourcesDock : public DockWidgetBase
{
    Q_OBJECT
public:
    ResourcesDock(StoryProject &project, ResourceModel &model);

    void Initialize();

    ResourceModel &getModel() { return m_resourcesModel; }
    ResourceFilterProxyModel &getFilteredModel() { return m_proxyModel; }

    void SetFilterType(const QString &type) { m_proxyModel.setFilterType(type); }

    void SetTitleImage(const QString &name);
    void SetTitleSound(const QString &name);

signals:
    void sigChooseTitle(bool isImage);

private:
    StoryProject &m_project;
    Ui::ostResources m_uiOstResources;
    ResourceModel &m_resourcesModel;
    ResourceFilterProxyModel m_proxyModel;
};

#endif // RESOURCESDOCK_H
