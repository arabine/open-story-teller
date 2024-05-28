import 'package:flutter/material.dart' hide Router;
import 'package:path/path.dart';

import 'dart:io';
import 'dart:convert';
import 'dart:async';
import 'dart:typed_data';

import 'package:saf/saf.dart';
import 'package:path_provider/path_provider.dart';
import 'package:audioplayers/audioplayers.dart';
import 'package:file_picker/file_picker.dart';
import 'package:external_path/external_path.dart';
import 'package:permission_handler/permission_handler.dart';
import 'package:logger/logger.dart';
import 'package:file_picker/file_picker.dart';
import 'package:shared_preferences/shared_preferences.dart';

import 'libstory/storyvm.dart';
import 'libstory/indexfile.dart';
import 'httpserver.dart';

class ProductionFilter extends LogFilter {
  @override
  bool shouldLog(LogEvent event) {
    return true;
  }
}

var logger = Logger(
  printer: PrettyPrinter(methodCount: 0),
  filter: ProductionFilter(), // Use the ProductionFilter to enable logging in release mode
  
);


void main() {
  StoryVm.loadLibrary();
  StoryVm.initialize();

  udpServer();
  httpServer();
  printIps();
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter Demo',
      theme: ThemeData(
        // This is the theme of your application.
        //
        // TRY THIS: Try running your application with "flutter run". You'll see
        // the application has a blue toolbar. Then, without quitting the app,
        // try changing the seedColor in the colorScheme below to Colors.green
        // and then invoke "hot reload" (save your changes or press the "hot
        // reload" button in a Flutter-supported IDE, or press "r" if you used
        // the command line to start the app).
        //
        // Notice that the counter didn't reset back to zero; the application
        // state is not lost during the reload. To reset the state, use hot
        // restart instead.
        //
        // This works for code too, not just values: Most code changes can be
        // tested with just a hot reload.
        scaffoldBackgroundColor: const Color(0xFF9ab4a4),

        useMaterial3: true,
      ),
      home: const MyHomePage(title: 'Flutter Demo Home Page'),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key, required this.title});

  // This widget is the home page of your application. It is stateful, meaning
  // that it has a State object (defined below) that contains fields that affect
  // how it looks.

  // This class is the configuration for the state. It holds the values (in this
  // case the title) provided by the parent (in this case the App widget) and
  // used by the build method of the State. Fields in a Widget subclass are
  // always marked "final".

  final String title;

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

enum PlayerState { disabled, indexFile, inStory }

class _MyHomePageState extends State<MyHomePage> {
  String libraryDir = 'fffff';
  IndexFile indexFile = IndexFile();
  String currentImage = 'assets/logo.png';
  final player = AudioPlayer();
  StreamSubscription? mediaPub;
  PlayerState state = PlayerState.disabled;
  StreamSubscription? audioPlayerSub;

  

  Image img = const Image(image: AssetImage('assets/logo.png'));


  void initPaths() async {
    

    final SharedPreferences prefs = await SharedPreferences.getInstance();
    final String? libDir = prefs.getString('library-directory');

    if (libDir != null) {
      setState(() {
        libraryDir = libDir;
      });
      bool success = await indexFile.loadIndexFile(libDir);
      if (success) {
        showCurrentStoryIndex();
        state = PlayerState.indexFile;
      }
    } else {
      logger.d("No library directory found");
      Directory? dir;
       if (Platform.isAndroid) {
        dir = await getExternalStorageDirectory();
      } else {
        dir = await getApplicationDocumentsDirectory();
      }
      if (dir != null) {
        setState(() {
          libraryDir = '${dir.toString()}/stories';
        });
      }
    }
    
    logger.d("===============+> $libraryDir");
  }

  _MyHomePageState() {
    initPaths();
    mediaPub = eventBus.on<MediaEvent>().listen((event) {
      setState(() {
          if (event.image.isNotEmpty) {
            currentImage = '${indexFile.getCurrentStoryPath()}/assets/${event.image}';
          }
      });

      img = Image.file(File(currentImage));

        if (event.sound.isNotEmpty) {
            player.play(DeviceFileSource('${indexFile.getCurrentStoryPath()}/assets/${event.sound}'));
        }
    });

    eventBus.on<SignalEvent>().listen((event) {
      if (event.signal == 1) {
        state = PlayerState.indexFile;
        showCurrentStoryIndex();
      }
    });


    audioPlayerSub = player.onPlayerComplete.listen((event) {
      if (state == PlayerState.inStory) {
        // Send end of music event
        StoryVm.endOfSound();
      }
    });

    state = PlayerState.indexFile;
  }

  Uint8List soundBuffer = Uint8List(0);

  void showCurrentStoryIndex() async {
    setState(() {
      currentImage = indexFile.getCurrentTitleImage();
      logger.d('Current image: $currentImage');
    });

    img = Image.file(File(currentImage));

    // File sound = File(indexFile.getCurrentSoundImage());
    // soundBuffer = sound.readAsBytesSync();
    // // logger.d('Asset: ${asset.toString()}');
    // await player.play(BytesSource(soundBuffer));

    player.play(DeviceFileSource(indexFile.getCurrentSoundImage()));
  }


  void chooseLibraryDirectory() async {
     // FilePickerResult? result = await FilePicker.platform.pickFiles();
      String ?path = await FilePicker.platform.getDirectoryPath();

      if (path != null) {
        logger.d("Selected directory: $path");
        final SharedPreferences prefs = await SharedPreferences.getInstance();
        prefs.setString('library-directory', path);
          setState(() {
            libraryDir = path;
        });
        bool success = await indexFile.loadIndexFile(path);
        if (success) {
            showCurrentStoryIndex();
            state = PlayerState.indexFile;
        }
      }
  }

  void handleClick(String value) async {
    switch (value) {
      case 'Library':
        chooseLibraryDirectory();
        break;
      case 'Settings':
        break;
    }
}

  @override
  Widget build(BuildContext context) {
    // This method is rerun every time setState is called, for instance
    //
    // The Flutter framework has been optimized to make rerunning build methods
    // fast, so that you can just rebuild anything that needs updating rather
    // than having to individually change instances of widgets.
    return Scaffold(
      appBar: AppBar(
        // title: Text('Homepage'),
        backgroundColor: const Color(0xFF9ab4a4),
        actions: <Widget>[
          PopupMenuButton<String>(
            onSelected: handleClick,
            itemBuilder: (BuildContext context) {
              return {'Library'}.map((String choice) {
                return PopupMenuItem<String>(
                  value: choice,
                  child: Text(choice),
                );
              }).toList();
            },
            
          ),
          const Divider()
          , const Text("v1.1")
        ],
      ),
      body: Center(
        // Center is a layout widget. It takes a single child and positions it
        // in the middle of the parent.
        child: Column(
          // Column is also a layout widget. It takes a list of children and
          // arranges them vertically. By default, it sizes itself to fit its
          // children horizontally, and tries to be as tall as its parent.
          //
          // Column has various properties to control how it sizes itself and
          // how it positions its children. Here we use mainAxisAlignment to
          // center the children vertically; the main axis here is the vertical
          // axis because Columns are vertical (the cross axis would be
          // horizontal).
          //
          // TRY THIS: Invoke "debug painting" (choose the "Toggle Debug Paint"
          // action in the IDE, or press "p" in the console), to see the
          // wireframe for each widget.

          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            // Text(
            //   libraryDir,
            //   style: Theme.of(context).textTheme.bodySmall,
            // ),
            img,
          ],
        ),
      ),
      bottomNavigationBar: BottomAppBar(
        color: const Color(0xFF9ab4a4),
        child: Row(
          children: <Widget>[
            IconButton(
              tooltip: 'Previous',
              icon: const Icon(Icons.arrow_circle_left, size: 40),
              onPressed: () {
                if (state == PlayerState.inStory) {
                    StoryVm.previousButton();
                } else if (state == PlayerState.indexFile) {
                  indexFile.previous();
                  showCurrentStoryIndex();
                }
              },
              color: const Color(0xFFb05728),
            ),
            IconButton(
              tooltip: 'Next',
              icon: const Icon(Icons.arrow_circle_right, size: 40),
              onPressed: () {

                if (state == PlayerState.inStory) {
                    StoryVm.nextButton();
                } else if (state == PlayerState.indexFile) { 
                  indexFile.next();
                  showCurrentStoryIndex();
                }
              },
              color: const Color(0xFFb05728),
            ),
            IconButton(
              tooltip: 'Home',
              icon: const Icon(Icons.home_filled, size: 40),
              onPressed: () {

                if (state == PlayerState.inStory) {
                  StoryVm.homeButton();
                } else if (state == PlayerState.indexFile) { 
                  player.stop();
                }
              },
              color: const Color(0xFFb05728),
            ),
          ],
        ),
      ),
      floatingActionButton: FloatingActionButton(
        onPressed: () {

           if (state == PlayerState.inStory) {
              StoryVm.okButton();
           } else if (state == PlayerState.indexFile) {
              String path = indexFile.getCurrentStoryPath();

              if (path.isNotEmpty) {
                StoryVm.start(path);
                state = PlayerState.inStory;
              }
           }
        
        },
        tooltip: 'Ok',
        backgroundColor: const Color(0xFF0092c8),
        foregroundColor: Colors.white,
        child: const Text('O K'),
      ),
      floatingActionButtonLocation: FloatingActionButtonLocation.endContained,
    );
  }
}
