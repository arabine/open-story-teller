library libstory;

import 'dart:io';
import 'dart:typed_data';

import 'package:logger/logger.dart';
import 'package:permission_handler/permission_handler.dart';

var logger = Logger(printer: PrettyPrinter(methodCount: 0));

class IndexFile {
  static const int tlvArrayType = 0xAB;
  static const int tlvObjectType = 0xE7;
  static const int tlvIntegerType = 0x77;
  static const int tlvStringType = 0x3D;
  static const int tlvRealType = 0xB8;

  // Index File stuff
  Uint8List indexFileBuffer = Uint8List(0);
  ByteData indexFileStream = ByteData(0);
  int readPtr = 0;
  int indexFileVersion = 0;
  bool indexFileIsValid = false;

  // Variables de gestion de l'index des histoires
  int storiesCount = 0;
  String libraryPath = '';
  int currentStoryIndex = 0;

  List<
      ({
        String uuid,
        String titleImage,
        String titleSound,
        String title,
        String description,
        int version
      })> stories = [];

  String getCurrentTitleImage() {
    String fileName = '';
    if (currentStoryIndex < storiesCount) {
      fileName =
          '$libraryPath/${stories[currentStoryIndex].uuid}/assets/${stories[currentStoryIndex].titleImage}';
    }
    return fileName;
  }

  String getCurrentSoundImage() {
    String fileName = '';
    if (currentStoryIndex < storiesCount) {
      fileName =
          '$libraryPath/${stories[currentStoryIndex].uuid}/assets/${stories[currentStoryIndex].titleSound}';
    }
    return fileName;
  }

  String getCurrentStoryPath() {
    String path = '';
    if (currentStoryIndex < storiesCount) {
      path =
          '$libraryPath/${stories[currentStoryIndex].uuid}';
    }
    return path;
  }

  void next() {
    currentStoryIndex++;
    if (currentStoryIndex >= storiesCount) {
      currentStoryIndex = 0;
    }
  }

  void previous() {
    currentStoryIndex--;
    if (currentStoryIndex < 0) {
      currentStoryIndex = storiesCount - 1;
    }
  }

  // Returns the size, 0 if error
  int getTl(int expectedType) {
    int size = 0;
    if (indexFileStream.getUint8(readPtr) == expectedType) {
      readPtr++;
      size = indexFileStream.getUint16(readPtr, Endian.little);
      readPtr += 2;
    } else {
      throw Exception("Expected type: $expectedType");
    }
    return size;
  }

  int getIntegerValue() {
    int size = getTl(tlvIntegerType);

    if (size == 4) {
      int value = indexFileStream.getUint32(readPtr, Endian.little);
      readPtr += 4;
      return value;
    } else {
      throw Exception("Expected an integer of size 4 bytes");
    }
  }

  String getStringValue() {
    int size = getTl(tlvStringType);
    if (size > 0) {
      String value = String.fromCharCodes(
          indexFileStream.buffer.asUint8List(), readPtr, readPtr + size);
      readPtr += size;
      return value;
    } else {
      return "";
    }
  }

  Future<bool> loadIndexFile(String libraryRoot) async {
    libraryPath = libraryRoot;
    indexFileIsValid = false;

    String indexFileName = '$libraryRoot/index.ost';

    bool isGranted = true;

    // if (Platform.isAndroid) {

    //   if (await Permission.manageExternalStorage.request().isGranted) {
    //     isGranted = true;
    //   }
    // } else {
    //   isGranted = true;
    // }

    if (isGranted) {
        final file = File(indexFileName);
        if (!await file.exists()) {
          logger.d('Le fichier n\'existe pas.');
          return false;
        }
        indexFileBuffer = file.readAsBytesSync();
    } else {
      logger.e("Cannot access to file: $indexFileName");
    }
   
    // Ouvrir le fichier en mode lecture binaire
    
    readPtr = 0;
    indexFileStream =
        ByteData.sublistView(indexFileBuffer, readPtr); // start at zero
    stories.clear();

    if (indexFileBuffer.lengthInBytes > 3) {
      // Root must be an object containing 2 elements

      try {
        if (getTl(tlvObjectType) == 2) {
          indexFileVersion = getIntegerValue();
          storiesCount = getTl(tlvArrayType);

          for (int i = 0; i < storiesCount; i++) {
            if (getTl(tlvObjectType) == 6) {
              var record = (
                uuid: getStringValue(),
                titleImage: getStringValue(),
                titleSound: getStringValue(),
                title: getStringValue(),
                description: getStringValue(),
                version: getIntegerValue()
              );

              logger.d('Found story: ${record.title.toString()}');

              stories.add(record);
            } else {
              throw Exception("Expected object of 6 elements at root");
            }
          }

          // If get through here, no exception raised so the file shoould be ok
          indexFileIsValid = true;
          currentStoryIndex = 0;
          return true;
        } else {
          throw Exception("Expected object of 2 elements at root");
        }
      } catch (e) {
        logger.e('Exception reading index file: $e');
      } finally {}
    }
    return false;
  }

  
}
