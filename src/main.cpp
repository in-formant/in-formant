#include "gui/AnalyserWindow.h"

int main(int argc, char **argv) {

    Analyser analyser;
    AnalyserWindow window(analyser);

    analyser.startThread();
    window.mainLoop();

    return EXIT_SUCCESS;
}
