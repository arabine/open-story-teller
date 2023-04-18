#include <iostream>

#include "pack_archive.h"
#include "command_line.h"
// 3ade540306254fffa22b9025ac3678d9
// be8949f60d854f54828419a1bdaed36a
static const std::string test_file = "be8949f60d854f54828419a1bdaed36a.pk";


int main(int argc, char** argv)
{
    std::cout << "OST convert tool" << std::endl;


    bool        oPrintHelp = false;
    std::string packFileName;
    bool extract = false;

      // First configure all possible command line options.
      CommandLine args("A demonstration of the simple command line parser.");
      args.addArgument({"-f", "--string"},   &packFileName,   "A string value");
//      args.addArgument({"-i", "--integer"},  &oInteger,  "A integer value");
//      args.addArgument({"-u", "--unsigned"}, &oUnsigned, "A unsigned value");
//      args.addArgument({"-d", "--double"},   &oDouble,   "A float value");
//      args.addArgument({"-f", "--float"},    &oFloat,    "A double value");
      args.addArgument({"-e", "--bool"},     &extract,     "A bool value");
      args.addArgument({"-h", "--help"},     &oPrintHelp,
          "Print this help. This help message is actually so long "
          "that it requires a line break!");

      // Then do the actual parsing.
      try {
        args.parse(argc, argv);
      } catch (std::runtime_error const& e) {
        std::cout << e.what() << std::endl;
        return -1;
      }

    // When oPrintHelp was set to true, we print a help message and exit.
    if (oPrintHelp)
    {
        args.printHelp();
        return 0;
    }

    PackArchive pack;

    if (extract)
    {
        std::cout << "Extracting pack: " << packFileName << std::endl;
        pack.DecipherAll(test_file);
    }

    return 0;
}
