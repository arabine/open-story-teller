#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QToolBar>
#include <QMenuBar>

class ToolBar : public QToolBar
{
    Q_OBJECT

public:
    ToolBar();
    void createActions(QMenuBar *menuBar);

signals:
    void sigSave();
    void sigOpen();
    void sigAbout();
};

#endif // TOOLBAR_H
