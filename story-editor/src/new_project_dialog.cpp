#include "new_project_dialog.h"
#include <QFileDialog>
#include <QDir>

NewProjectDialog::NewProjectDialog(QWidget *parent)
    : QDialog{parent}
{
    m_ui.setupUi(this);

    m_ui.imageFormatCombo->addItem(tr("BMP (compressed 4-bit palette)"), StoryProject::IMG_FORMAT_BMP_4BITS);
    m_ui.imageFormatCombo->addItem(tr("QOIF (Quite Ok Image Format)"), StoryProject::IMG_FORMAT_QOIF);

    m_ui.soundFormatCombo->addItem(tr("WAV"), StoryProject::SND_FORMAT_WAV);
    m_ui.soundFormatCombo->addItem(tr("QOA (Quite Ok Audio)"), StoryProject::SND_FORMAT_QOAF);

    m_ui.displaySizeCombo->addItem("320x240", QSize(320,240));

    connect(m_ui.selectDirectoryButton, &QPushButton::clicked, this, [&]() {

        m_dir = QFileDialog::getExistingDirectory(this, tr("Project Directory"),
                                                        QDir::homePath(),
                                                        QFileDialog::ShowDirsOnly
                                                            | QFileDialog::DontResolveSymlinks);
        m_ui.directoryPath->setText(m_dir);

    });

    connect(m_ui.buttonBox, &QDialogButtonBox::accepted, this, [&]() {
        QDir projDir(m_dir);
        if (projDir.exists())
        {
            // Le répertoire doit être vide
            bool empty = projDir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0;

            if (empty)
            {
                // Test project name
                if (!m_ui.projectName->text().isEmpty())
                {
                    accept();
                }
                else
                {
                    m_ui.errorMessage->setText(R"(<p style="color:red">)" + tr("Error, project name empty.") + R"(</p>)");
                }
            }
            else
            {
                m_ui.errorMessage->setText(R"(<p style="color:red">)" + tr("Error, directory is not empty.") + R"(</p>)");
            }
        }
        else
        {
            m_ui.errorMessage->setText(R"(<p style="color:red">)" + tr("Error, directory does not exist.") + R"(</p>)");
        }


    });

    connect(m_ui.buttonBox, &QDialogButtonBox::rejected, this, [&]() {

        this->close();
    });
}

QString NewProjectDialog::GetProjectFileName() const
{
    return m_dir + QDir::separator() + "project.json";
}

QString NewProjectDialog::GetProjectName() const { return m_ui.projectName->text(); }

StoryProject::ImageFormat NewProjectDialog::GetImageFormat() const
{
    StoryProject::ImageFormat img{StoryProject::IMG_FORMAT_BMP_4BITS};
    int idx = m_ui.imageFormatCombo->currentData().toInt();

    if (idx < StoryProject::IMG_FORMAT_COUNT) {
        img = static_cast<StoryProject::ImageFormat>(idx);
    }

    return img;
}

StoryProject::SoundFormat NewProjectDialog::GetSoundFormat() const
{
    StoryProject::SoundFormat img{StoryProject::SND_FORMAT_WAV};
    int idx = m_ui.soundFormatCombo->currentData().toInt();

    if (idx < StoryProject::IMG_FORMAT_COUNT) {
        img = static_cast<StoryProject::SoundFormat>(idx);
    }

    return img;
}

QSize NewProjectDialog::GetDisplayFormat() const
{
    return m_ui.displaySizeCombo->currentData().toSize();
}

void NewProjectDialog::Initialize()
{
    m_ui.errorMessage->clear();
    m_ui.directoryPath->clear();
}
