#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <functional>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void draw_line(uint16_t y, uint8_t *pixels, uint8_t *palette);
    void startTest();
private:
    QImage img;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
