#include "resources_dock.h"
#include <QFileDialog>
#include <QStandardPaths>

ResourcesDock::ResourcesDock()
    : QDockWidget(tr("Resources"))
{
    setObjectName("ResourcesDock");  // used to save the state

    m_uiOstResources.setupUi(this);
    m_uiOstResources.resourcesView->setModel(&m_resourcesModel);

    connect(m_uiOstResources.addImageButton, &QPushButton::clicked, [=](bool checked) {

        QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                        ".",
                                                        tr("Images (*.bmp)"));

        Resource res;
        res.format = "BMP";
        res.file = fileName.toStdString();

        m_resourcesModel.append(res);

    });
}


