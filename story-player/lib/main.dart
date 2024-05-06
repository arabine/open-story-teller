import 'package:flutter/material.dart' hide Router;

import 'dart:io';
import 'dart:convert';
import 'dart:async';

import 'package:shelf/shelf.dart';
import 'package:shelf/shelf_io.dart' as io;
import 'package:shelf_router/shelf_router.dart';
import 'package:mime/mime.dart';
import 'package:path/path.dart' as p;
import 'package:http_parser/http_parser.dart';
import 'package:saf/saf.dart';
import 'package:path_provider/path_provider.dart';

import 'package:file_picker/file_picker.dart';

import 'package:flutter_launcher/storyvm.dart';

import 'package:logger/logger.dart';

var logger = Logger(printer: PrettyPrinter(methodCount: 0));

void httpServer() async {
  final router = Router();

  // Route pour uploader des fichiers
  router.post('/upload', (Request request) async {
    var contentType = MediaType.parse(request.headers['content-type']!);
    var boundary = contentType.parameters['boundary']!;
    var transformer = MimeMultipartTransformer(boundary);
    var bodyStream = request.read();
    var parts = await transformer.bind(bodyStream).toList();

    for (var part in parts) {
      var contentDisp = part.headers['content-disposition']!;
      var filename = _extractFilename(contentDisp);
      if (filename != null) {
        var filePath = p.join('uploads', filename);
        await _saveFile(part, filePath);
        logger.d('File uploaded: $filePath');
      }
    }

    return Response.ok('File uploaded successfully');
  });

  // Route pour lister les fichiers
  router.get('/files', (Request request) async {
    final directory = Directory('uploads');
    final files =
        directory.listSync().map((file) => p.basename(file.path)).toList();
    final json = {'files': files};
    return Response.ok(jsonEncode(json),
        headers: {'Content-Type': 'application/json'});
  });

  // Démarrer le serveur
  io.serve(router, 'localhost', 8080).then((server) {
    logger.d('Serving at http://${server.address.host}:${server.port}');
  });
}

String? _extractFilename(String contentDisposition) {
  return RegExp('filename="([^"]*)"').firstMatch(contentDisposition)?.group(1);
}

Future<void> _saveFile(MimeMultipart part, String filename) async {
  var file = File(filename);
  await file.create(recursive: true);
  await part.pipe(file.openWrite());
}

void udpServer() async {
  // Créer un socket UDP
  var port = 8080; // Vous pouvez spécifier le port de votre choix
  var address =
      InternetAddress.anyIPv4; // Écouter sur toutes les interfaces IPv4
  RawDatagramSocket socket = await RawDatagramSocket.bind(address, port);
  logger.d('Serveur UDP écoutant sur ${address.address}:$port');

  // Gérer les événements du socket
  socket.listen((RawSocketEvent event) {
    if (event == RawSocketEvent.read) {
      Datagram? datagram = socket.receive();
      if (datagram != null) {
        var message = utf8.decode(datagram.data);
        logger.d(
            'Message reçu de ${datagram.address.address}:${datagram.port}: $message');

        // Envoyer une réponse
        String response = 'Reçu: $message';
        List<int> data = utf8.encode(response);
        socket.send(data, datagram.address, datagram.port);
        logger.d(
            'Réponse envoyée à ${datagram.address.address}:${datagram.port}');
      }
    }
  });
}

Future printIps() async {
  for (var interface in await NetworkInterface.list()) {
    logger.d('== Interface: ${interface.name} ==');
    for (var addr in interface.addresses) {
      logger.d(
          '${addr.address} ${addr.host} ${addr.isLoopback} ${addr.rawAddress} ${addr.type.name}');
    }
  }
}

void main() {
  StoryVm.loadLibrary();
  StoryVm.initialize();
  StoryVm.start();
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

class _MyHomePageState extends State<MyHomePage> {
  int _counter = 0;
  String myPath = 'fffff';

  void initPaths() async {
    Directory? dir;

    if (Platform.isAndroid) {
      dir = await getExternalStorageDirectory();
    } else {
      dir = await getApplicationDocumentsDirectory();
    }

    setState(() {
      myPath = dir.toString();
    });
    logger.d("===============+> $myPath");
  }

  _MyHomePageState() {
    initPaths();
  }

  void _incrementCounter() {
    setState(() {
      // This call to setState tells the Flutter framework that something has
      // changed in this State, which causes it to rerun the build method below
      // so that the display can reflect the updated values. If we changed
      // _counter without calling setState(), then the build method would not be
      // called again, and so nothing would appear to happen.
      _counter++;
    });
  }

  @override
  Widget build(BuildContext context) {
    // This method is rerun every time setState is called, for instance as done
    // by the _incrementCounter method above.
    //
    // The Flutter framework has been optimized to make rerunning build methods
    // fast, so that you can just rebuild anything that needs updating rather
    // than having to individually change instances of widgets.
    return Scaffold(
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
            const Text(
              'You have pushed the button this many times:',
            ),
            Text(
              '$_counter',
              style: Theme.of(context).textTheme.headlineMedium,
            ),
            Text(
              myPath,
              style: Theme.of(context).textTheme.headlineMedium,
            ),
            const Image(image: AssetImage('assets/320x240.png')),
          ],
        ),
      ),
      bottomNavigationBar: BottomAppBar(
        color: const Color(0xFF9ab4a4),
        child: Row(
          children: <Widget>[
            IconButton(
              tooltip: 'Open navigation menu',
              icon: const Icon(
                Icons.folder,
                size: 40,
              ),
              onPressed: () async {
                String? selectedDirectory =
                    await FilePicker.platform.getDirectoryPath();

                if (selectedDirectory == null) {
                  // User canceled the picker
                } else {
                  bool? isGranted = true;
                  logger.d(selectedDirectory);

                  if (Platform.isAndroid) {
                    Saf saf = Saf(selectedDirectory);

                    isGranted =
                        await saf.getDirectoryPermission(isDynamic: false);

                    if (isGranted != null && isGranted) {
                      // Perform some file operations
                      logger.d('Granted!');
                    } else {
                      // failed to get the permission
                    }
                  }

                  if (isGranted == true) {
                    StoryVm.loadIndexFile('$selectedDirectory/index.ost');
                  }
                }
              },
              color: const Color(0xFFb05728),
            ),
            IconButton(
              tooltip: 'Search',
              icon: const Icon(Icons.arrow_circle_left, size: 40),
              onPressed: () {},
              color: const Color(0xFFb05728),
            ),
            IconButton(
              tooltip: 'Favorite',
              icon: const Icon(Icons.arrow_circle_right, size: 40),
              onPressed: () {},
              color: const Color(0xFFb05728),
            ),
          ],
        ),
      ),
      floatingActionButton: FloatingActionButton(
        onPressed: _incrementCounter,
        tooltip: 'Ok',
        backgroundColor: const Color(0xFF0092c8),
        foregroundColor: Colors.white,
        child: const Text('O K'),
      ),
      floatingActionButtonLocation: FloatingActionButtonLocation.endContained,
    );
  }
}
