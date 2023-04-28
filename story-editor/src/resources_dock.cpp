#include "resources_dock.h"
#include <QFileDialog>
#include <QStandardPaths>

ResourcesDock::ResourcesDock(StoryProject &project)
    : m_project(project)
    , DockWidgetBase(tr("Resources"))
{
    setObjectName("ResourcesDock");  // used to save the state

    m_uiOstResources.setupUi(this);
    m_uiOstResources.resourcesView->setModel(&m_resourcesModel);

    connect(m_uiOstResources.addImageButton, &QPushButton::clicked, [=](bool checked) {

        QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                        ".",
                                                        tr("Images (*.bmp)"));

        std::filesystem::path p(fileName.toStdString());
        std::filesystem::path p2 = m_project.ImagesPath() /  p.filename().generic_string();
        std::filesystem::copy(p, p2);

        Resource res;
        res.format = "BMP";
        res.file = p.filename();
        m_resourcesModel.append(res);

    });

    connect(m_uiOstResources.addSoundButton, &QPushButton::clicked, [=](bool checked) {

        QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                        ".",
                                                        tr("Sounds (*.wav)"));

        std::filesystem::path p(fileName.toStdString());
        std::filesystem::path p2 = m_project.SoundsPath() /  p.filename().generic_string();
        std::filesystem::copy(p, p2);

        Resource res;
        res.format = "WAV";
        res.file = p.filename();
        m_resourcesModel.append(res);
    });
}

void ResourcesDock::Initialize()
{

}

void ResourcesDock::SaveToProject()
{
    m_project.Clear();
    for (auto & r : m_resourcesModel.GetData())
    {
        m_project.m_images.push_back(r);
    }
}

void ResourcesDock::slotClear()
{
    m_resourcesModel.Clear();
}


