import 'dart:io';
import 'dart:convert';
import 'dart:async';

import 'package:shelf/shelf.dart';
import 'package:shelf/shelf_io.dart' as io;
import 'package:shelf_router/shelf_router.dart';
import 'package:mime/mime.dart';
import 'package:path/path.dart' as p;
import 'package:http_parser/http_parser.dart'; // y'a un warning mais il faut laisser cet import
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
