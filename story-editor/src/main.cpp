
#include "main_window.h"


#include <iostream>       // std::cerr
#include <exception>      // std::set_terminate
#include <cstdlib>        // std::abort
#include <csignal>

#include <iostream>

// Main code
int main(int, char**)
{

    MainWindow w;
    w.Initialize();

    w.Loop();

    return 0;
}
