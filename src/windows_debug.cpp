#include <iostream>
#include <thread>
#include <future>
#include <chrono>
#include <windows.h>
#include <tlhelp32.h>
#include <QApplication>
#include "context/timings.h"

using namespace std::chrono_literals;

static int getCurrentThreadCount() {
    // first determine the id of the current process
    DWORD const  id = GetCurrentProcessId();

    // then get a process list snapshot.
    HANDLE const  snapshot = CreateToolhelp32Snapshot( TH32CS_SNAPALL, 0 );

    // initialize the process entry structure.
    PROCESSENTRY32 entry = { 0 };
    entry.dwSize = sizeof( entry );

    // get the first process info.
    BOOL  ret = true;
    ret = Process32First( snapshot, &entry );
    while( ret && entry.th32ProcessID != id ) {
        ret = Process32Next( snapshot, &entry );
    }
    CloseHandle( snapshot ); 
    return ret 
        ?   entry.cntThreads
        :   -1;
}

void windowsDebugCallback(std::future<void> future) {
    while (future.wait_for(2ms) == std::future_status::timeout) {
        /*if (qApp->allWindows().empty()) {
            break;
        }*/

        // Remove one to account for the debug thread.
        int threadCount = getCurrentThreadCount() - 1;
        
        std::cout << "# of threads: " << threadCount << std::endl;

        std::cout << "Timings:" << std::endl
                << " - Render: " << timings::render << std::endl
                << " - Update: " << timings::update << std::endl
                << "   - spectrogram:  " << timings::updateSpectrogram << std::endl
                << "   - pitch:        " << timings::updatePitch << std::endl
                << "   - formants:     " << timings::updateFormants << std::endl
                << "   - oscilloscope: " << timings::updateOscilloscope << std::endl
                << " - Synth: " << timings::synth << std::endl;

        std::cout << "===" << std::endl;

        std::this_thread::sleep_for(1000ms);
    }
}