#include <FL/Fl.H>

#include "gui.h"


int main(int argc, char *argv[])
{
    DebuggerGUI gui(argc, argv);
    return Fl::run();
}

