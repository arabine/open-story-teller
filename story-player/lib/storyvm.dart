import 'dart:ffi';
import 'dart:io';
import 'dart:async';
import 'dart:typed_data';

import 'package:ffi/ffi.dart';
import 'package:logger/logger.dart';

var logger = Logger(printer: PrettyPrinter(methodCount: 0));

Completer<bool> periodic(Duration interval, Function(int cycle) callback) {
  final done = Completer<bool>();
  () async {
    var cycle = 0;
    while (!done.isCompleted) {
      try {
        await callback(cycle);
      } catch (e, s) {
        logger.e("$e", stackTrace: s);
      }
      cycle++;
      await done.future.timeout(interval).onError((error, stackTrace) => false);
    }
  }();
  return done;
}

typedef MediaCallbackType = Void Function(Int32 i, Pointer<Utf8> str);
typedef MediaCallback = void Function(Int32 i, Pointer<Utf8> str);

typedef VmInitializeType = Void Function(
    Pointer<NativeFunction<MediaCallbackType>>);
typedef VmInitialize = void Function(
    Pointer<NativeFunction<MediaCallbackType>>);

typedef VmStartType = Void Function(Pointer<Uint8> data, Uint32 size);
typedef VmStart = void Function(Pointer<Uint8> data, int size);

typedef VmRunType = Void Function();
typedef VmRun = void Function();

class StoryVm {
  static late DynamicLibrary nativeApiLib;
  static late VmInitialize vmInitialize;
  static late VmStart vmStart;
  static late VmRun vmRun;

  static bool running = false;

  static bool loadLibrary() {
    String dllName = 'libstoryvm.so';

    if (Platform.isMacOS) {
      dllName = 'libstoryvm.dylib';
    } else if (Platform.isWindows) {
      dllName = 'storyvm.dll';
    }

    final dylib = DynamicLibrary.open(dllName);

    vmInitialize = dylib
        .lookup<NativeFunction<VmInitializeType>>('storyvm_initialize')
        .asFunction();

    vmStart =
        dylib.lookup<NativeFunction<VmStartType>>('storyvm_start').asFunction();

    vmRun =
        dylib.lookup<NativeFunction<VmRunType>>('storyvm_start').asFunction();

    return true;
  }

  static void mediaCallback(int i, Pointer<Utf8> str) {
    logger.d('Mediatype: $i, Media file: $str');
  }

  static void initialize() {
    vmInitialize(Pointer.fromFunction<MediaCallbackType>(mediaCallback));
  }

  static Completer<bool> task = Completer<bool>();

  static void start() {
    task = periodic(const Duration(milliseconds: 10), (cycle) async {
      if (running) {
        vmRun();
      }
    });
  }

  static void stop() {
    logger.d('VM stop');
    task.complete(true);
  }

  static Uint8List indexFileBuffer = Uint8List(0);
  static ByteData streamIndex = ByteData(0);
  static int indexFileVersion = 0;
  static int storiesCount = 0;
  static int readPtr = 0;
  static bool indexValid = false;

  static const int tlvArrayType = 0xAB;
  static const int tlvObjectType = 0xE7;
  static const int tlvIntegerType = 0x77;
  static const int tlvStringType = 0x3D;
  static const int tlvRealType = 0xB8;

  static List<
      ({
        String uuid,
        String titleImage,
        String titleSound,
        String title,
        String description,
        int version
      })> stories = [];

  // Returns the size, 0 if error
  static int getTl(int expectedType) {
    int size = 0;
    if (streamIndex.getUint8(readPtr) == expectedType) {
      readPtr++;
      size = streamIndex.getUint16(readPtr, Endian.little);
      readPtr += 2;
    } else {
      throw Exception("Expected type: $expectedType");
    }
    return size;
  }

  static int getIntegerValue() {
    int size = getTl(tlvIntegerType);

    if (size == 4) {
      int value = streamIndex.getUint32(readPtr, Endian.little);
      readPtr += 4;
      return value;
    } else {
      throw Exception("Expected an integer of size 4 bytes");
    }
  }

  static String getStringValue() {
    int size = getTl(tlvStringType);
    if (size > 0) {
      String value =
          ByteData.sublistView(streamIndex, readPtr, readPtr + size).toString();
      readPtr += size;
      return value;
    } else {
      return "";
    }
  }

  static void loadIndexFile(String filePath) async {
    final file = File(filePath);
    if (!await file.exists()) {
      logger.d('Le fichier n\'existe pas.');
      return;
    }

    // Ouvrir le fichier en mode lecture binaire
    indexFileBuffer = file.readAsBytesSync();
    readPtr = 0;
    streamIndex =
        ByteData.sublistView(indexFileBuffer, readPtr); // start at zero

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

              logger.d('Found story: $record.title');

              stories.add(record);
            } else {
              logger.e("Expected object of 6 elements at root");
            }
          }
        } else {
          logger.e("Expected object of 2 elements at root");
        }
      } catch (e) {
        logger.e('Exception reading index file: $e');
      } finally {}
    }
  }
}
