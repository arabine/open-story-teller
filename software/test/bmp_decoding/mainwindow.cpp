#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QPainter>
#include "picture.h"

#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , img(320, 240, QImage::Format_RGB32)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


/*
       // QPixmap img(width, height);
    //    img.fill(QColor("#38A0A2"));
        QPainter painter;

        painter.begin(&img);

        uint32_t x = 0, y = 0;
        for (uint32_t i = 0; i < pixel; i++)
        {
            uint8_t val = decompressed[i];
            if (val > 15)
            {
                std::cout << "Error!" << std::endl;
            }
            uint8_t *palettePtr = &palette[val * 4];
            QColor pixColor(palettePtr[0], palettePtr[1], palettePtr[2]);
            painter.setPen(pixColor);
            painter.drawPoint(x, y);

            x++;
            if (x >= width)
            {
                x = 0;
                y++;
            }
        }

        painter.end();

    //    std::ofstream outfile ("new.txt", std::ofstream::binary);
    //    outfile.write (reinterpret_cast<const char *>(decompressed), pixel / 2 );
    //    outfile.close();

        QImage finalImage = img.mirrored();
        finalImage.save("test_dec.bmp");
*/

}

void MainWindow::startTest()
{
    img.fill(QColor("yellow"));
    decompress();
    std::fflush(stdout);

    ui->label->setPixmap(QPixmap::fromImage(img));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::draw_line(uint16_t y, uint8_t *pixels, uint8_t *palette)
{
    QPainter painter(&img);
    uint32_t x = 0;
    uint32_t width = 320;

    for (uint32_t i = 0; i < width; i++)
    {
        uint8_t val = pixels[i];
        if (val > 15)
        {
            std::cout << "Error!" << std::endl;
        }
        const uint8_t *palettePtr = &palette[val * 4];

        int r = palettePtr[0];
        int g = palettePtr[1];
        int b = palettePtr[2];

        QColor pixColor(r, g, b);
        painter.setPen(pixColor);
        painter.drawPoint(x, y);

        x++;
        if (x >= width)
        {
            x = 0;
        }
    }
}

