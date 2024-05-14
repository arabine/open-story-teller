
#include "main_window.h"

// Main code
int main(int, char**)
{

    MainWindow w;
    if (w.Initialize())
    {
        w.Loop();
    }  

    return 0;
}
