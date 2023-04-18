#include "mainwindow.h"
#include <iostream>
#include <QApplication>

MainWindow *w_ptr = nullptr;

extern "C" void ost_display_draw_h_line(uint16_t y, uint8_t *pixels, uint8_t *palette)
{
    if (w_ptr != nullptr)
    {
        w_ptr->draw_line(y, pixels, palette);
    }
}



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;

    w_ptr = &w;

    w.startTest();

    w.show();
    return a.exec();
}
