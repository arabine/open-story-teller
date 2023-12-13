#ifndef NEW_PROJECT_DIALOG_H
#define NEW_PROJECT_DIALOG_H

#include <QObject>
#include <QDialog>
#include "ui_new-project.h"
#include "story_project.h"

class NewProjectDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NewProjectDialog(QWidget *parent = nullptr);

    QString GetProjectFileName() const;
    QString GetProjectName() const;

    StoryProject::ImageFormat GetImageFormat() const;
    StoryProject::SoundFormat GetSoundFormat() const;
    QSize GetDisplayFormat() const;

    void Initialize();
signals:
    void sigAccepted();

private:
    Ui::newProjectDialog m_ui;

    QString m_dir;
};

#endif // NEW_PROJECT_DIALOG_H
