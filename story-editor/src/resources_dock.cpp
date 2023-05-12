#include "resources_dock.h"
#include <QFileDialog>
#include <QStandardPaths>

ResourcesDock::ResourcesDock(StoryProject &project)
    : m_project(project)
    , m_resourcesModel(project)
    , DockWidgetBase(tr("Resources"), true)
{
    setObjectName("ResourcesDock");  // used to save the state

    m_uiOstResources.setupUi(this);
    m_uiOstResources.resourcesView->setModel(&m_resourcesModel);

    m_proxyModel.setSourceModel(&m_resourcesModel);

    connect(m_uiOstResources.addImageButton, &QPushButton::clicked, [=](bool checked) {

        QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                        ".",
                                                        tr("Images (*.bmp)"));

        if (std::filesystem::exists(fileName.toStdString()))
        {
            std::filesystem::path p(fileName.toStdString());
            std::filesystem::path p2 = m_project.ImagesPath() /  p.filename().generic_string();
            std::filesystem::copy(p, p2, std::filesystem::copy_options::overwrite_existing);

            Resource res;
            res.format = "BMP";
            res.type = "image";
            res.file = p.filename();
            m_resourcesModel.append(res);
        }
    });

    connect(m_uiOstResources.addSoundButton, &QPushButton::clicked, [=](bool checked) {

        QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                        ".",
                                                        tr("Sound files (*.wav, *.mp3, *.m4a)"));

        if (std::filesystem::exists(fileName.toStdString()))
        {
            std::filesystem::path p(fileName.toStdString());
            std::filesystem::path p2 = m_project.SoundsPath() /  p.filename().generic_string();
            std::filesystem::copy(p, p2, std::filesystem::copy_options::overwrite_existing);

            Resource res;
            res.format = "WAV";
            res.type = "sound";
            res.file = p.filename();
            m_resourcesModel.append(res);
        }
    });
}

void ResourcesDock::Initialize()
{

}

void ResourcesDock::Append(const Resource &res)
{
    m_resourcesModel.append(res);
}

void ResourcesDock::SaveToProject()
{
    m_project.Clear();
//    for (auto & r : m_resourcesModel.GetData())
//    {
//        m_project.m_images.push_back(r);
    //    }
}

void ResourcesDock::Clear()
{
    m_resourcesModel.Clear();
}


