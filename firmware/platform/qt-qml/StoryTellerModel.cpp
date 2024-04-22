#include "StoryTellerModel.h"

#include "packarchive.h"

#include <QPainter>
#include <QDir>
#include "Util.h"
#include "Base64Util.h"
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <fstream>
#include "ni_parser.h"
#include "libutil.h"

StoryTellerModel::StoryTellerModel(QObject *parent) : QObject(parent)
{
    mPacksPath = settings.value("packs/path").toString();
    player = new QMediaPlayer;
    connect(player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(slotPlayerStateChanged(QMediaPlayer::State)));
}

void StoryTellerModel::ScanPacks()
{
    QDir dir(mPacksPath);
    QStringList filters;
    filters << "*.pk";
    dir.setNameFilters(filters);
    if (dir.exists())
    {
        // scan all pack files
        mListOfPacks = dir.entryList();

        for (const auto & p : mListOfPacks)
        {
            std::cout << "Found pack: " << p.toStdString() << std::endl;
        }

        if (mListOfPacks.size() == 0)
        {
            SetMessage("Mauvais emplacement des packs");
        }
        else
        {
            openFile();
        }
    }
    else
    {
        SetMessage("Mauvais emplacement des packs");
    }
}

QString StoryTellerModel::getImage()
{
    std::stringstream ss;
    std::string base64_str = Base64::Encode(std::string(reinterpret_cast<char*>(bmpImage), sizeof(bmpImage)));

    QString image = "data:image/bmp;base64,";
    image.append(QString::fromLatin1(base64_str.data()));
    return image;
}

void StoryTellerModel::openFile()
{
    if (mListOfPacks.size() > 0)
    {
        QString packName = mListOfPacks[mCurrentPackIndex];
        QString fullPackPath = mPacksPath + Util::DIR_SEPARATOR + packName;
        if (pack.Load(fullPackPath.toStdString()))
        {
            SetMessage("");
            /*
         * std::vector<std::string> imgs = pack.GetImages();
            uint32_t index = 0;
            for (const auto & im : imgs)
            {
                std::string image = pack.GetImage(im);
                DecryptImage(image);

                std::string fileName = std::to_string(index) + "_" + im + std::string(".bmp");
                Util::ReplaceCharacter(fileName, "\\", "_");
                SaveImage(fileName);
                index++;
            }*/

            ShowResources();
        }
        else
        {
            SetMessage("Pack load error: " + packName);
        }
    }
}

void StoryTellerModel::okButton()
{
    TestDecompress2();
//    pack.OkButton();
//    ShowResources();
}

void StoryTellerModel::previous()
{
    if (pack.IsWheelEnabled() && !pack.IsRoot())
    {
        pack.Previous();
        ShowResources();
    }
    else
    {
        if (mCurrentPackIndex > 0)
        {
            mCurrentPackIndex--;
        }
        else
        {
            mCurrentPackIndex = mListOfPacks.size() - 1;
        }

        openFile();
    }
}

void StoryTellerModel::next()
{
    if (pack.IsWheelEnabled() && !pack.IsRoot())
    {
        pack.Next();
        ShowResources();
    }
    else
    {
        mCurrentPackIndex++;
        if (mCurrentPackIndex >= static_cast<uint32_t>(mListOfPacks.size()))
        {
            mCurrentPackIndex = 0;
        }

        openFile();
    }
}

void StoryTellerModel::initialize()
{
    ScanPacks();
//    TestDecompress();
//    TestDecompress2();
}

void StoryTellerModel::ShowResources()
{
    if (pack.HasImage())
    {
        SetImage(pack.CurrentImage());
    }
    else
    {
        ClearScreen();
    }

    std::string fileName = pack.CurrentSoundName();
    Util::ReplaceCharacter(fileName, "\\", "_");

    Play(fileName, pack.CurrentSound());
}

void StoryTellerModel::saveSettings(const QString &packPath)
{
    mPacksPath = QUrl(packPath).toLocalFile();
    settings.setValue("packs/path", mPacksPath);
    ScanPacks();
}

void StoryTellerModel::ClearScreen()
{
    emit sigClearScreen();
}

void StoryTellerModel::DecryptImage(const std::string &bytes)
{
    uint32_t compressedSize = bytes.length() - 512;

    memcpy(bmpImage, bytes.data(), 512);
    ni_decode_block512(bmpImage);

    memcpy(bmpImage + 512, bytes.data() + 512, compressedSize);
}

void StoryTellerModel::SaveImage(const std::string &fileName)
{
    std::ofstream outfile (fileName, std::ofstream::binary);
    outfile.write (reinterpret_cast<char *>(bmpImage), 512 + 320*240);
    outfile.close();
}

void StoryTellerModel::SetImage(const std::string &bytes)
{
    DecryptImage(bytes);
    emit sigShowImage();
}

void StoryTellerModel::Play(const std::string &fileName, const QByteArray &ar)
{
    buffer.close();
    buffer.setData(ar);
    buffer.open(QIODevice::ReadOnly);

    player->setMedia(QUrl(fileName.c_str()), &buffer);
    player->setVolume(60);
    player->play();
}

void StoryTellerModel::SetMessage(const QString &newMessage)
{
    mCurrentMessage = newMessage;
    emit sigMessageChanged();
}

void StoryTellerModel::slotPlayerStateChanged(QMediaPlayer::State newState)
{
    if (newState == QMediaPlayer::StoppedState)
    {
        // next action!
        std::cout << "Sound ended" << std::endl;

        if (pack.AutoPlay())
        {
            // équivalent de Ok button
            okButton();
        }
    }
}

typedef struct {
   uint16_t type;                 /* Magic identifier            */
   uint32_t size;                       /* File size in bytes          */
   uint16_t reserved1;
   uint16_t reserved2;
   uint32_t offset;                     /* Offset to image data, bytes */
} bmp_header_t;

typedef struct {
   uint32_t size;               /* Header size in bytes      */
   uint32_t width;
   uint32_t height;                /* Width and height of image */
   uint16_t planes;       /* Number of colour planes   */
   uint16_t bits;         /* Bits per pixel            */
   uint32_t compression;        /* Compression type          */
   uint32_t imagesize;          /* Image size in bytes       */
   uint32_t xresolution;
   uint32_t yresolution;     /* Pixels per meter          */
   uint32_t ncolours;           /* Number of colours         */
   uint32_t importantcolours;   /* Important colours         */
   uint32_t rgb;
   uint32_t rgb2;
} bmp_infoheader_t;

static const uint32_t HEADER_SIZE = 14;
static const uint32_t INFO_HEADER_SIZE = 40;

uint8_t parse_bmp(const uint8_t *data, bmp_header_t *header, bmp_infoheader_t *info_header)
{
    uint8_t isBmp = 0;


    // Header is 14 bytes length
    isBmp = (data[0] == 'B') && (data[1] == 'M') ? 1 : 0;
    header->size = leu32_get(data + 2);
    header->offset = leu32_get(data + 10);

    isBmp = isBmp & 1;
    info_header->size = leu32_get(data + HEADER_SIZE);
    info_header->width = leu32_get(data + HEADER_SIZE + 4);
    info_header->height = leu32_get(data + HEADER_SIZE + 8);
    info_header->planes = leu16_get(data + HEADER_SIZE + 12);
    info_header->bits = leu16_get(data + HEADER_SIZE + 14);
    info_header->compression = leu32_get(data + HEADER_SIZE + 16);
    info_header->imagesize = leu32_get(data + HEADER_SIZE + 20);
    info_header->xresolution = leu32_get(data + HEADER_SIZE + 24);
    info_header->yresolution = leu32_get(data + HEADER_SIZE + 28);
    info_header->ncolours = leu32_get(data + HEADER_SIZE + 32);
    info_header->importantcolours = leu32_get(data + HEADER_SIZE + 36);
    info_header->rgb = leu32_get(data + HEADER_SIZE + 40);
    info_header->rgb2 = leu32_get(data + HEADER_SIZE + 44);

    return isBmp;
}

// Code de décompression
void StoryTellerModel::TestDecompress()
{
    uint32_t width = 320;
    uint32_t height = 240;

    // Première version: on charge toute l'image en RAM
    std::string bytes = Util::FileToString("/home/anthony/git/personnel/OpenStoryTeller/application/0_000_314CBAA1.bmp");

    if (bytes.size() <= 512)
    {
        return;
    }

    // Buffer d'entrée, bitmap compressé
    uint8_t bmpImage[bytes.size()];
    memcpy(bmpImage, bytes.data(), bytes.length());
    uint32_t fileSize = bytes.length();

    bmp_header_t header;
    bmp_infoheader_t info_header;
    parse_bmp(bmpImage, &header, &info_header);

    uint8_t *compressed = &bmpImage[header.offset];
    uint32_t compressedSize = fileSize - header.offset;

    uint32_t paletteSize = header.offset - (HEADER_SIZE + INFO_HEADER_SIZE);

    printf("File size (from header):%d\r\n", (uint32_t)header.size);
    printf("File size (from data):%d\r\n", (uint32_t)fileSize);
    printf("Data offset:%d\r\n", (uint32_t)header.offset);
    printf("Image size:%d\r\n", (uint32_t)info_header.size);
    printf("width:%d\r\n", (uint32_t)info_header.width);
    printf("height:%d\r\n", (uint32_t)info_header.height);
    printf("Planes:%d\r\n", (uint32_t)info_header.planes);
    printf("Bits:%d\r\n", (uint32_t)info_header.bits);
    printf("Compression:%d\r\n", (uint32_t)info_header.compression); // 2 - 4 bit run length encoding
    printf("Image size:%d\r\n", (uint32_t)info_header.imagesize);
    printf("X resolution:%d\r\n", (uint32_t)info_header.xresolution);
    printf("Y resolution:%d\r\n", (uint32_t)info_header.yresolution);
    printf("Colors:%d\r\n", (uint32_t)info_header.ncolours);
    printf("Important colors:%d\r\n", (uint32_t)info_header.importantcolours);
    printf("RGB :%d\r\n", (uint32_t)info_header.rgb);
    printf("RGB2 :%d\r\n", (uint32_t)info_header.rgb2);

    uint8_t *palette = &bmpImage[HEADER_SIZE + INFO_HEADER_SIZE]; // 16 * 4 bytes = 64

    // buffer de sortie, bitmap décompressé
    uint8_t decompressed[320 * 240];
    memset(decompressed, 0, 320*240);

  //  btea((uint32_t*) bmpImage, -128, key);

    uint32_t pixel = 0; // specify the pixel offset
    uint32_t i = 0;
    do
    {
        uint8_t rleCmd = compressed[i];
        if (rleCmd > 0)
        {
            uint8_t val = compressed[i + 1];
            // repeat number of pixels
            for (uint32_t j = 0; j < rleCmd; j++)
            {
                if ((j & 1) == 0)
                {
                    decompressed[pixel] = (val & 0xF0) >>4;
                }
                else
                {
                    decompressed[pixel] = (val & 0x0F);
                }
                pixel++;
            }
            i += 2; // jump pair instruction
        }
        else
        {
            uint8_t second = compressed[i + 1];
            if (second == 0)
            {
                if (pixel % width)
                {
                    // end of line
                    uint32_t lines = pixel / width;
                    uint32_t remaining = width - (pixel - (lines * width));

                    pixel += remaining;
                }
                i += 2;
            }
            else if (second == 1)
            {
                std::cout << "End of bitmap" << std::endl;
                goto end;
            }
            else if (second == 2)
            {
                // delta N pixels and M lines
                pixel += compressed[i + 2] + compressed[i + 3] * width;
                i += 4;
            }
            else
            {
                // absolute mode
                uint8_t *ptr = &compressed[i + 2];
                // repeat number of pixels
                for (uint32_t j = 0; j < second; j++)
                {
                    if ((j & 1) == 0)
                    {
                        decompressed[pixel] = (*ptr & 0xF0) >> 4;
                    }
                    else
                    {
                        decompressed[pixel] = (*ptr & 0x0F);
                        ptr++;
                    }
                    pixel++;
                }
                i += 2 + (second / 2);

                // padded in word boundary, jump if necessary
                if ((second / 2) % 2)
                {
                    i++;
                }
            }
        }

        if (pixel > (width * height))
        {
            std::cout << "Error!" << std::endl;
        }
    }
    while( i < compressedSize);

end:
    QImage img(width, height, QImage::Format_RGB32);
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
}

uint32_t x = 0, y = 0;
void StoryTellerModel::WriteLine(uint8_t *decompressed, const uint8_t *palette, QPainter &painter, uint32_t width)
{
    for (uint32_t i = 0; i < 320; i++)
    {
        uint8_t val = decompressed[i];
        if (val > 15)
        {
            std::cout << "Error!" << std::endl;
        }
        const uint8_t *palettePtr = &palette[val * 4];
        QColor pixColor(palettePtr[0], palettePtr[1], palettePtr[2]);
        painter.setPen(pixColor);
        painter.drawPoint(x, y);

        int r = palettePtr[0];
        int g = palettePtr[1];
        int b = palettePtr[2];

        emit sigDrawPixel(r, g, b, x, y);

        x++;
        if (x >= width)
        {
            x = 0;
            y++;
        }
    }
}


void StoryTellerModel::TestDecompress2()
{
    uint32_t width = 320;
    uint32_t height = 240;
    static uint8_t bmpImage[512];
    static uint8_t palette[16*4];
    static uint8_t decompressed[320*240];

    FILE *fil;
    uint32_t offset;
    fil = fopen("0_000_314CBAA1.bmp", "r");

    offset = 0;
    fseek(fil, offset, SEEK_SET );
    fread(bmpImage, 1, 512,fil);

    bmp_header_t header;
    bmp_infoheader_t info_header;
    parse_bmp(bmpImage, &header, &info_header);

    // Compute some sizes
    uint32_t fileSize = header.size;
    // uint32_t compressedSize = fileSize - header.offset;
    uint32_t paletteSize = header.offset - (HEADER_SIZE + INFO_HEADER_SIZE);

    // Copy palette
    offset = HEADER_SIZE + INFO_HEADER_SIZE;
    memcpy(palette, bmpImage + HEADER_SIZE + INFO_HEADER_SIZE, paletteSize);

    offset = header.offset;
    uint8_t *compressed = &bmpImage[header.offset];

/*
    printf("File size (from header):%d\r\n", (uint32_t)header.size);
    printf("File size (from data):%d\r\n", (uint32_t)fileSize);
    printf("Data offset:%d\r\n", (uint32_t)header.offset);
    printf("Image size:%d\r\n", (uint32_t)info_header.size);
    printf("width:%d\r\n", (uint32_t)info_header.width);
    printf("height:%d\r\n", (uint32_t)info_header.height);
    printf("Planes:%d\r\n", (uint32_t)info_header.planes);
    printf("Bits:%d\r\n", (uint32_t)info_header.bits);
    printf("Compression:%d\r\n", (uint32_t)info_header.compression); // 2 - 4 bit run length encoding
    printf("Image size:%d\r\n", (uint32_t)info_header.imagesize);
    printf("X resolution:%d\r\n", (uint32_t)info_header.xresolution);
    printf("Y resolution:%d\r\n", (uint32_t)info_header.yresolution);
    printf("Colors:%d\r\n", (uint32_t)info_header.ncolours);
    printf("Important colors:%d\r\n", (uint32_t)info_header.importantcolours);
    printf("RGB :%d\r\n", (uint32_t)info_header.rgb);
    printf("RGB2 :%d\r\n", (uint32_t)info_header.rgb2);
*/

    // buffer de sortie, bitmap décompressé
    memset(decompressed, 0, 320*240);

  //  btea((uint32_t*) bmpImage, -128, key);

    int nblines = 0;

    uint32_t pixel = 0; // specify the pixel offset
    bool end = false;
    uint32_t i = 0;

    QImage img(width, height, QImage::Format_RGB32);
   // QPixmap img(width, height);
//    img.fill(QColor("#38A0A2"));
    QPainter painter;

    painter.begin(&img);

    do
    {
        // if we are behond the middle of the buffer, read more data from file
        if (i > 256)
        {
            offset = offset + i;
            fseek(fil, offset, SEEK_SET );
            fread(bmpImage, 1, 512,fil);
            i = 0;
        }

        uint8_t rleCmd = compressed[i];
        if (rleCmd > 0)
        {
            uint8_t val = compressed[i + 1];
            // repeat number of pixels
            for (uint32_t j = 0; j < rleCmd; j++)
            {
                if ((j & 1) == 0)
                {
                    decompressed[pixel] = (val & 0xF0) >>4;
                }
                else
                {
                    decompressed[pixel] = (val & 0x0F);
                }
                pixel++;
            }

            i += 2; // jump pair instruction
        }
        else
        {
            uint8_t second = compressed[i + 1];
            if (second == 0)
            {
                if (pixel % info_header.width)
                {
                    // end of line
                    uint32_t lines = pixel / info_header.width;
                    uint32_t remaining = info_header.width - (pixel - (lines * info_header.width));

                    pixel += remaining;
                }
                nblines++;

                WriteLine(decompressed, palette, painter, 320);
                pixel = 0;

                i += 2;
            }
            else if (second == 1)
            {
                end = true;
            }
            else if (second == 2)
            {
                // delta N pixels and M lines
                pixel += compressed[i + 2] + compressed[i + 3] * info_header.width;
                i += 4;
            }
            else
            {
                // absolute mode
                uint8_t *ptr = &compressed[i + 2];
                // repeat number of pixels
                for (uint32_t j = 0; j < second; j++)
                {
                    if ((j & 1) == 0)
                    {
                        decompressed[pixel] = (*ptr & 0xF0) >> 4;
                    }
                    else
                    {
                        decompressed[pixel] = (*ptr & 0x0F);
                        ptr++;
                    }
                    pixel++;
                }
                i += 2 + (second / 2);

                // padded in word boundary, jump if necessary
                if ((second / 2) % 2)
                {
                    i++;
                }
            }
        }

        if (pixel > (info_header.width * info_header.height))
        {
           end = false; // error
        }
    }
    while((offset < fileSize) && !end);

    fclose(fil);

    if (end)
    {
/*
        QImage img(width, height, QImage::Format_RGB32);
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
*/
        painter.end();

    //    std::ofstream outfile ("new.txt", std::ofstream::binary);
    //    outfile.write (reinterpret_cast<const char *>(decompressed), pixel / 2 );
    //    outfile.close();

        QImage finalImage = img.mirrored();
        finalImage.save("test_dec.bmp");
    }
}

