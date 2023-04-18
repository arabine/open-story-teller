
#include "main_window.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow *w = new MainWindow();
    w->showNormal();

    return app.exec();
}
