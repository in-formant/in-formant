#include "modules/modules.h"
#include "analysis/analysis.h"
#include "context/contextmanager.h"
#include <iostream>
#include <atomic>
#include <memory>
#include <chrono>
#include <csignal>
#include <thread>

#include <QApplication>
#include <QMessageBox>

#ifdef _WIN32
# include <windows.h>
#endif

using namespace Module;
using namespace std::chrono_literals;

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

    QMetaObject::invokeMethod(qGuiApp, "quit");
}

int start_logger(const char *app_name);

int Main::argc;
char **Main::argv;

#ifdef _WIN32
#include <windows.h>
LONG WINAPI TopLevelExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo);
#ifdef DEBUG_THREAD
void windowsDebugCallback(std::future<void> future);
#endif
#endif

#ifndef _MSC_VER
__attribute__((visibility ("default")))
#endif
int main(int argc, char **argv)
{
#ifdef _WIN32
    SetUnhandledExceptionFilter(TopLevelExceptionHandler);
    srand(time(nullptr));
    #ifdef DEBUG_THREAD
    std::promise<void> exitSignal;
    std::future<void> exitSignalFuture = exitSignal.get_future();
    std::thread debugThread(windowsDebugCallback, std::move(exitSignalFuture));
    #endif
#endif

    start_logger("InFormant");

    std::signal(SIGTERM, signalHandler);
    std::signal(SIGINT, signalHandler);

    Main::argc = argc;
    Main::argv = argv;

    auto contextManager = std::make_unique<Main::ContextManager>(
            48'000,     // captureSampleRate
            50ms,       // playbackDuration
            48'000      // playbackSampleRate
    );

    int ret = contextManager->exec();

    #ifdef DEBUG_THREAD
    exitSignal.set_value();
    if (debugThread.joinable())
        debugThread.join();
    #endif

    return ret;
}

