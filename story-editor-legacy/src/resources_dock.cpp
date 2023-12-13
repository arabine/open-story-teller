#include "resources_dock.h"
#include <QFileDialog>
#include <QStandardPaths>

ResourcesDock::ResourcesDock(StoryProject &project, ResourceModel &model)
    : m_project(project)
    , m_resourcesModel(model)
    , DockWidgetBase(tr("Resources"), true)
{
    setObjectName("ResourcesDock");  // used to save the state

    m_uiOstResources.setupUi(this);
    m_uiOstResources.resourcesView->setModel(&m_resourcesModel);

    m_proxyModel.setSourceModel(&m_resourcesModel);

    connect(m_uiOstResources.addImageButton, &QPushButton::clicked, [&](bool checked) {

        QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                        ".",
                                                        tr("Images (*.bmp *.png *.jpg)"));

        if (std::filesystem::exists(fileName.toStdString()))
        {
            std::filesystem::path p(fileName.toStdString());
            std::filesystem::path p2 = m_project.AssetsPath() /  p.filename().generic_string();
            std::filesystem::copy(p, p2, std::filesystem::copy_options::overwrite_existing);

            Resource res;

            std::string ext = p.extension().string();
            ext.erase(ext.begin()); // remove '.' dot sign
            std::transform(ext.begin(), ext.end(), ext.begin(), ::toupper);

            res.format = ext;
            res.type = "image";
            res.file = p.filename().generic_string();
            m_resourcesModel.Append(res);
        }
    });

    connect(m_uiOstResources.addSoundButton, &QPushButton::clicked, [&](bool checked) {

        QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                        ".",
                                                        tr("Sound files (*.wav *.mp3)"));

        if (std::filesystem::exists(fileName.toStdString()))
        {
            std::filesystem::path p(fileName.toStdString());
            std::filesystem::path p2 = m_project.AssetsPath() /  p.filename().generic_string();
            std::filesystem::copy(p, p2, std::filesystem::copy_options::overwrite_existing);

            Resource res;

            std::string ext = p.extension().string();
            ext.erase(ext.begin()); // remove '.' dot sign
            std::transform(ext.begin(), ext.end(), ext.begin(), ::toupper);

            res.format = ext;
            res.type = "sound";
            res.file = p.filename().generic_string();
            m_resourcesModel.Append(res);
        }
    });

    connect(m_uiOstResources.deleteButton, &QPushButton::clicked, [&](bool checked) {
        QItemSelectionModel *selectionModel = m_uiOstResources.resourcesView->selectionModel();
        // Récupération des lignes sélectionnées
        QModelIndexList selectedRows = selectionModel->selectedRows();

        if (selectedRows.count() > 0)
        {
            for (int i = 0; i < selectedRows.count(); i++)
            {
                m_resourcesModel.Delete(selectedRows.at(i).row());
            }
        }
    });

    connect(m_uiOstResources.chooseTitleImageButton, &QPushButton::clicked, [&](bool enable) {
        emit sigChooseTitle(true);
    });

    connect(m_uiOstResources.chooseTitleSoundButton, &QPushButton::clicked, [&](bool enable) {
        emit sigChooseTitle(false);
    });
}

void ResourcesDock::SetTitleImage(const QString &name)
{
    m_uiOstResources.titleImage->setText(name);
}

void ResourcesDock::SetTitleSound(const QString &name)
{
    m_uiOstResources.titleSound->setText(name);
}


