library libstory;

import 'dart:ffi';
import 'dart:io';
import 'dart:async';
import 'dart:typed_data';

import 'package:ffi/ffi.dart';
import 'package:logger/logger.dart';
import 'package:event_bus/event_bus.dart';

var logger = Logger(printer: PrettyPrinter(methodCount: 0));

/// The global [EventBus] object.
EventBus eventBus = EventBus();

class MediaEvent {
  String image;
  String sound;

  MediaEvent(this.image, this.sound);
}

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

typedef VmSendEventType = Void Function(Int);
typedef VmSendEvent = void Function(int event);

enum VmEvent { evNoEvent, evStep, evOkButton, evPreviousButton, evNextButton, evAudioFinished, evStop }

class StoryVm {
  static late DynamicLibrary nativeApiLib;
  static late VmInitialize vmInitialize;
  static late VmStart vmStart;
  static late VmRun vmRun;
  static late VmSendEvent vmSendEvent;
  static String currentStoryPath = '';

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

    vmStart = dylib.lookup<NativeFunction<VmStartType>>('storyvm_start').asFunction();

    vmRun = dylib.lookup<NativeFunction<VmRunType>>('storyvm_run').asFunction();

    vmSendEvent = dylib.lookup<NativeFunction<VmSendEventType>>('storyvm_send_event').asFunction();

    return true;
  }

  static void mediaCallback(int i, Pointer<Utf8> str) {
    String file = str.toDartString();
    logger.d('Mediatype: $i, Media file: $file');

    if (i == 0) {
        eventBus.fire(MediaEvent(file, ""));
    } else {
        eventBus.fire(MediaEvent("", file));
    }
    
  }

  static void initialize() {
    vmInitialize(Pointer.fromFunction<MediaCallbackType>(mediaCallback));
  }

  static Completer<bool> task = Completer<bool>();

  static Future<bool> start(String storyBasePath) async{
    currentStoryPath = storyBasePath;
    final file = File('$storyBasePath/story.c32');
    if (!file.existsSync()) {
      logger.d('Le fichier n\'existe pas.');
      return false;
    }

    // Ouvrir le fichier en mode lecture binaire
    Uint8List fileBuffer = file.readAsBytesSync();

    Pointer<Uint8> dataPointer = malloc.allocate<Uint8>(fileBuffer.length);
    for (int i = 0; i < fileBuffer.length; i++) {
      dataPointer[i] = fileBuffer[i];
    }
    
    vmStart(dataPointer, fileBuffer.length);
    running = true;
    task = periodic(const Duration(milliseconds: 10), (cycle) async {
      if (running) {
        vmRun();
      }
    });

    return true;
  }

  static void endOfSound() {

      vmSendEvent(VmEvent.evAudioFinished.index);
  }

  static void okButton() {

      vmSendEvent(VmEvent.evOkButton.index);
  }

  static void previousButton() {

      vmSendEvent(VmEvent.evPreviousButton.index);
  }

  static void nextButton() {

      vmSendEvent(VmEvent.evNextButton.index);
  }

  static void stop() {
    logger.d('VM stop');
    running = false;
    task.complete(true);
  }
}
