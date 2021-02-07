#include "modules/modules.h"
#include "nodes/nodes.h"
#include "analysis/analysis.h"
#include "context/contextmanager.h"
#include <iostream>
#include <atomic>
#include <memory>
#include <chrono>
#include <csignal>
#include <thread>

#if defined(__APPLE__)
#   include <mach-o/dyld.h>
#   include <unistd.h>
#   include <libgen.h>
#endif

#include <QApplication>

using namespace Module;
using namespace std::chrono_literals;

#if ! defined(__EMSCRIPTEN__)
static std::atomic_bool signalCaught(false);
static std::atomic_int signalStatus;

static void signalHandler(int signal) {
    signalCaught = true;
    signalStatus = signal;

    switch (signal) {
    case SIGTERM:
        std::cout << "Caught signal SIGTERM" << std::endl;
        break;
    case SIGINT:
        std::cout << "Caught signal SIGINT" << std::endl;
        break;
    }

    QMetaObject::invokeMethod(qApp, "quit");
}
#endif

int start_logger(const char *app_name);

int Main::argc;
char **Main::argv;

int main(int argc, char **argv)
{
#ifdef _WIN32
    srand(time(nullptr));
#endif

    start_logger("InFormant");

#if ! defined(__EMSCRIPTEN__)
    std::signal(SIGTERM, signalHandler);
    std::signal(SIGINT, signalHandler);
    //registerCrashHandler();
#endif

#if defined(__APPLE__)
    /*uint32_t bufSize = 32;
    rpm::vector<char> exePath(bufSize);
    int ret = _NSGetExecutablePath(exePath.data(), &bufSize);
    if (ret < 0) {
        exePath.resize(bufSize);
        _NSGetExecutablePath(exePath.data(), &bufSize);
    }
    chdir(dirname(exePath.data()));*/
#endif

    Main::argc = argc;
    Main::argv = argv;

    auto contextManager = std::make_unique<Main::ContextManager>(
            48'000,     // captureSampleRate
            //100ms,      // captureDuration
            10ms, 25ms, // playbackBlock(Min/Max)Duration
            50ms,       // playbackDuration
            48'000      // playbackSampleRate
    );

    return contextManager->exec();
}

