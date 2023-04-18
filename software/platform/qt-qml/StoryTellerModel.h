#ifndef STORY_TELLER_MODEL_H
#define STORY_TELLER_MODEL_H

#include <QObject>
#include <QImage>
#include <QSettings>
#include <QBuffer>
#include <QByteArray>
#include <QtMultimedia/QMediaPlayer>

#include "packarchive.h"

class StoryTellerModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString message READ getMessage NOTIFY sigMessageChanged)

public:
    explicit StoryTellerModel(QObject *parent = nullptr);

    void SetImage(const std::string &bytes);

    Q_INVOKABLE QString getImage();
    Q_INVOKABLE void saveSettings(const QString &packPath);
    Q_INVOKABLE void openFile();
    Q_INVOKABLE void okButton();
    Q_INVOKABLE void previous();
    Q_INVOKABLE void next();
    Q_INVOKABLE QString getMessage() { return mCurrentMessage; }
    Q_INVOKABLE void initialize();

    void ClearScreen();
    void DecryptImage(const std::string &bytes);
    void SaveImage(const std::string &fileName);

signals:
    void sigShowImage();
    void sigClearScreen();
    void sigMessageChanged();
    void sigDrawPixel(int r, int g, int b, int x, int y);

private:
    QString mImage;
    QSettings settings;
    PackArchive pack;
    QMediaPlayer *player;
    QBuffer buffer;
    QString mCurrentMessage;
    uint8_t bmpImage[512 + 320*240];

    // Circulation dans les différents packs
    QString mPacksPath;
    QStringList mListOfPacks;
    uint32_t mCurrentPackIndex = 0;

    void ShowResources();
    void Play(const std::string &fileName, const QByteArray &ar);
    void SetMessage(const QString &newMessage);
    void ScanPacks();


    void TestDecompress();
    void TestDecompress2();
    void WriteLine(uint8_t *decompressed, const uint8_t *palette, QPainter &painter, uint32_t width);
private slots:
    void slotPlayerStateChanged(QMediaPlayer::State newState);
};

#endif // STORY_TELLER_MODEL_H
