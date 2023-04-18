/*
 This reads a wave file from an SD card and plays it using the I2S interface to
 a MAX08357 I2S Amp Breakout board.

Circuit:
    * Arduino/Genuino Zero, MKRZero or MKR1000 board
            * SD breakout or shield connected
                   * MAX08357:
    * GND connected GND
        * VIN connected 5V
    * LRC connected to pin 0 (Zero) or pin 3 (MKR1000, MKRZero)
               * BCLK connected to pin 1 (Zero) or pin 2 (MKR1000, MKRZero)
               * DIN connected to pin 9 (Zero) or pin A6 (MKR1000, MKRZero)

               created 15 November 2016
               by Sandeep Mistry
                   */

#include <SdFat.h>
#include <ArduinoSound.h>

// filename of wave file to play
const char filename[] = "castemere_mono.wav";

// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 2
//
// Set DISABLE_CHIP_SELECT to disable a second SPI device.
// For example, with the Ethernet shield, set DISABLE_CHIP_SELECT
// to 10 to disable the Ethernet controller.
const int8_t DISABLE_CHIP_SELECT = -1;
//
// Test with reduced SPI speed for breadboards.  SD_SCK_MHZ(4) will select
// the highest speed supported by the board that is not over 4 MHz.
// Change SPI_SPEED to SD_SCK_MHZ(50) for best performance.
#define SPI_SPEED SD_SCK_MHZ(24)
//------------------------------------------------------------------------------

SdFat sd;
// SD card chip select
int chipSelect = 28;

// variable representing the Wave File
SDWaveFile waveFile(sd, filename);

void setup() {
    // Open serial communications and wait for port to open:
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }

    if (!sd.begin(chipSelect, SPI_SPEED)) {
        if (sd.card()->errorCode()) {
            Serial.println("SD initialization failed.");
//            cout << F(
//                "\nSD initialization failed.\n"
//                "Do not reformat the card!\n"
//                "Is the card correctly inserted?\n"
//                "Is chipSelect set to the correct value?\n"
//                "Does another SPI device need to be disabled?\n"
//                "Is there a wiring/soldering problem?\n");
//            cout << F("\nerrorCode: ") << hex << showbase;
//            cout << int(sd.card()->errorCode());
//            cout << F(", errorData: ") << int(sd.card()->errorData());
//            cout << dec << noshowbase << endl;
            return;
        }
        Serial.println("Card successfully initialized.");
        if (sd.vol()->fatType() == 0) {
            Serial.println("Can't find a valid FAT16/FAT32 partition.");
            return;
        }
        Serial.println("Can't determine error type.");
        return;
    }

    // check if the WaveFile is valid
    if (!waveFile) {
        Serial.println("wave file is invalid!");
        while (1); // do nothing
    }

    // print out some info. about the wave file
    Serial.print("Bits per sample = ");
    Serial.println(waveFile.bitsPerSample());

    long channels = waveFile.channels();
    Serial.print("Channels = ");
    Serial.println(channels);

    long sampleRate = waveFile.sampleRate();
    Serial.print("Sample rate = ");
    Serial.print(sampleRate);
    Serial.println(" Hz");

    long duration = waveFile.duration();
    Serial.print("Duration = ");
    Serial.print(duration);
    Serial.println(" seconds");

    // adjust the playback volume
    AudioOutI2S.volume(5);

    // check if the I2S output can play the wave file
    if (!AudioOutI2S.canPlay(waveFile)) {
        Serial.println("unable to play wave file using I2S!");
        while (1); // do nothing
    }

    // start playback
    Serial.println("starting playback");
    AudioOutI2S.play(waveFile);
}

void loop() {
    // check if playback is still going on
    if (!AudioOutI2S.isPlaying()) {
        // playback has stopped

        Serial.println("playback stopped");
        while (1); // do nothing
    }
}


/**
 *

// ============================================       SD EXAMPLE

// Quick hardware test for SPI card access.
//
#include <SPI.h>
#include "SdFat.h"
#include "sdios.h"

// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 2
//
// Set DISABLE_CHIP_SELECT to disable a second SPI device.
// For example, with the Ethernet shield, set DISABLE_CHIP_SELECT
// to 10 to disable the Ethernet controller.
const int8_t DISABLE_CHIP_SELECT = -1;
//
// Test with reduced SPI speed for breadboards.  SD_SCK_MHZ(4) will select
// the highest speed supported by the board that is not over 4 MHz.
// Change SPI_SPEED to SD_SCK_MHZ(50) for best performance.
#define SPI_SPEED SD_SCK_MHZ(24)
//------------------------------------------------------------------------------
#if SD_FAT_TYPE == 0
SdFat sd;
File file;
#elif SD_FAT_TYPE == 1
SdFat32 sd;
File32 file;
#elif SD_FAT_TYPE == 2
SdExFat sd;
ExFile file;
#elif SD_FAT_TYPE == 3
SdFs sd;
FsFile file;
#else  // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif  // SD_FAT_TYPE
// Serial streams
ArduinoOutStream cout(Serial);

// input buffer for line
char cinBuf[40];
ArduinoInStream cin(Serial, cinBuf, sizeof(cinBuf));

// SD card chip select
int chipSelect = 28;

void cardOrSpeed() {
    cout << F("Try another SD card or reduce the SPI bus speed.\n");
    cout << F("Edit SPI_SPEED in this program to change it.\n");
}

void reformatMsg() {
    cout << F("Try reformatting the card.  For best results use\n");
    cout << F("the SdFormatter program in SdFat/examples or download\n");
    cout << F("and use SDFormatter from www.sdcard.org/downloads.\n");
}

void setup() {
    Serial.begin(115200);

    // Wait for USB Serial
    while (!Serial) {
        SysCall::yield();
    }
    cout << F("\nSPI pins:\n");
    cout << F("MISO: ") << int(MISO) << endl;
    cout << F("MOSI: ") << int(MOSI) << endl;
    cout << F("SCK:  ") << int(SCK) << endl;
    cout << F("SS:   ") << int(SS) << endl;
#ifdef SDCARD_SS_PIN
    cout << F("SDCARD_SS_PIN:   ") << int(SDCARD_SS_PIN) << endl;
#endif  // SDCARD_SS_PIN

    if (DISABLE_CHIP_SELECT < 0) {
        cout << F(
            "\nBe sure to edit DISABLE_CHIP_SELECT if you have\n"
            "a second SPI device.  For example, with the Ethernet\n"
            "shield, DISABLE_CHIP_SELECT should be set to 10\n"
            "to disable the Ethernet controller.\n");
    }
    cout << F(
        "\nSD chip select is the key hardware option.\n"
        "Common values are:\n"
        "Arduino Ethernet shield, pin 4\n"
        "Sparkfun SD shield, pin 8\n"
        "Adafruit SD shields and modules, pin 10\n");
}

bool firstTry = true;
void loop() {
    // Read any existing Serial data.
    do {
        delay(10);
    } while (Serial.available() && Serial.read() >= 0);

    if (!firstTry) {
        cout << F("\nRestarting\n");
    }
    firstTry = false;


//    cout << F("\nEnter the chip select pin number: ");
//    while (!Serial.available()) {
//        SysCall::yield();
//    }
//    cin.readline();
//    if (cin >> chipSelect) {
//        cout << chipSelect << endl;
//    } else {
//        cout << F("\nInvalid pin number\n");
//        return;
//    }


    if (DISABLE_CHIP_SELECT < 0) {
        cout << F(
            "\nAssuming the SD is the only SPI device.\n"
            "Edit DISABLE_CHIP_SELECT to disable another device.\n");
    } else {
        cout << F("\nDisabling SPI device on pin ");
        cout << int(DISABLE_CHIP_SELECT) << endl;
        pinMode(DISABLE_CHIP_SELECT, OUTPUT);
        digitalWrite(DISABLE_CHIP_SELECT, HIGH);
    }
    if (!sd.begin(chipSelect, SPI_SPEED)) {
        if (sd.card()->errorCode()) {
            cout << F(
                "\nSD initialization failed.\n"
                "Do not reformat the card!\n"
                "Is the card correctly inserted?\n"
                "Is chipSelect set to the correct value?\n"
                "Does another SPI device need to be disabled?\n"
                "Is there a wiring/soldering problem?\n");
            cout << F("\nerrorCode: ") << hex << showbase;
            cout << int(sd.card()->errorCode());
            cout << F(", errorData: ") << int(sd.card()->errorData());
            cout << dec << noshowbase << endl;
            return;
        }
        cout << F("\nCard successfully initialized.\n");
        if (sd.vol()->fatType() == 0) {
            cout << F("Can't find a valid FAT16/FAT32 partition.\n");
            reformatMsg();
            return;
        }
        cout << F("Can't determine error type\n");
        return;
    }
    cout << F("\nCard successfully initialized.\n");
    cout << endl;

    uint32_t size = sd.card()->sectorCount();
    if (size == 0) {
        cout << F("Can't determine the card size.\n");
        cardOrSpeed();
        return;
    }
    uint32_t sizeMB = 0.000512 * size + 0.5;
    cout << F("Card size: ") << sizeMB;
    cout << F(" MB (MB = 1,000,000 bytes)\n");
    cout << endl;
    cout << F("Volume is FAT") << int(sd.vol()->fatType());
    cout << F(", Cluster size (bytes): ") << sd.vol()->bytesPerCluster();
    cout << endl << endl;

    cout << F("Files found (date time size name):\n");
    sd.ls(LS_R | LS_DATE | LS_SIZE);

    if ((sizeMB > 1100 && sd.vol()->sectorsPerCluster() < 64)
        || (sizeMB < 2200 && sd.vol()->fatType() == 32)) {
        cout << F("\nThis card should be reformatted for best performance.\n");
        cout << F("Use a cluster size of 32 KB for cards larger than 1 GB.\n");
        cout << F("Only cards larger than 2 GB should be formatted FAT32.\n");
        reformatMsg();
        return;
    }
    // Read any extra Serial data.
    do {
        delay(10);
    } while (Serial.available() && Serial.read() >= 0);
    cout << F("\nSuccess!  Type any character to restart.\n");
    while (!Serial.available()) {
        SysCall::yield();
    }
}

*/




/*
 *  EXEMPLE 1

#include "Arduino.h"

// the setup function runs once when you press reset or power the board
void setup() {
    // initialize digital pin LED_BUILTIN as an output.
    pinMode(0, OUTPUT);

    SerialUSB.begin(9600);
}

// the loop function runs over and over again forever
void loop()
{
    digitalWrite(0, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(1000);                       // wait for a second
    digitalWrite(0, LOW);    // turn the LED off by making the voltage LOW
    delay(1000);                       // wait for a second
    SerialUSB.println("Hello, Arduino!");
}

*/
