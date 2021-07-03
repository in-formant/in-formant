#include <exception>
#include <excpt.h>
#ifdef _WIN32

#include <windows.h>
#include <iostream>
#include <sstream>
#include <windows.h>
#include <winternl.h>
#include <imagehlp.h>
#include <stdio.h>
#include <tchar.h>

static std::wstringstream ss;

#ifdef _WIN64

void DisplayError(std::wstringstream& ss, DWORD NTStatusMessage)
{
    LPVOID lpMessageBuffer;
    HMODULE Hand = LoadLibrary(L"NTDLL.DLL");

    FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_FROM_HMODULE,
        Hand, 
        RtlNtStatusToDosError(NTStatusMessage),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMessageBuffer,  
        0,  
        NULL );

    // Now display the string.
    ss << "Error 0x" << std::hex << NTStatusMessage << ": " << (LPTSTR) lpMessageBuffer << std::endl;

    // Free the buffer allocated by the system.
    LocalFree( lpMessageBuffer ); 
    FreeLibrary(Hand);
}

void addr2line(std::wstringstream& ss, void const * const addr)
{
    static constexpr size_t sizeOfStruct = sizeof(IMAGEHLP_SYMBOL) + 255;
    static char symbolBuffer[sizeOfStruct];
    
    memset(symbolBuffer, 0, sizeOfStruct);

    IMAGEHLP_SYMBOL64 *symbol = (IMAGEHLP_SYMBOL *) symbolBuffer;

    symbol->SizeOfStruct = sizeOfStruct;
    symbol->MaxNameLength = 254;

    DWORD64 displacement = 0;

    if (SymGetSymFromAddr64(GetCurrentProcess(),
                          (DWORD64) addr,
                          &displacement,
                          symbol)) {

        IMAGEHLP_LINE64 line;
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

        DWORD displacement;
        if (SymGetLineFromAddr64(GetCurrentProcess(),
                                (DWORD64) addr,
                                &displacement,
                                &line)) {
            ss << "0x" << std::hex << addr << ": " << symbol->Name << " line " << std::dec << line.LineNumber << std::endl;
        }
        else {
            ss << "0x" << std::hex << addr << ": " << symbol->Name << std::endl;
        }
    }
    else {
        ss << "0x" << std::hex << addr << std::endl;
    }
}

void windows_print_stacktrace(std::wstringstream& ss, CONTEXT* context)
{
    SymInitialize(GetCurrentProcess(), 0, true);

    STACKFRAME frame = { 0 };

    /* setup initial stack frame */
    frame.AddrPC.Offset         = context->Rip;
    frame.AddrPC.Mode           = AddrModeFlat;
    frame.AddrStack.Offset      = context->Rsp;
    frame.AddrStack.Mode        = AddrModeFlat;
    frame.AddrFrame.Offset      = context->Rbp;
    frame.AddrFrame.Mode        = AddrModeFlat;

    while (StackWalk(IMAGE_FILE_MACHINE_AMD64 ,
                     GetCurrentProcess(),
                     GetCurrentThread(),
                     &frame,
                     context,
                     0,
                     SymFunctionTableAccess,
                     SymGetModuleBase,
                     0 ) )
    {
        addr2line(ss, (void *) frame.AddrPC.Offset);
    }

    SymCleanup( GetCurrentProcess() );
}

#else // _WIN32

void DisplayError(std::wstringstream& ss, DWORD NTStatusMessage)
{
    LPVOID lpMessageBuffer;
    HMODULE Hand = LoadLibrary("NTDLL.DLL");

    FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_FROM_HMODULE,
        Hand, 
        RtlNtStatusToDosError(NTStatusMessage),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMessageBuffer,  
        0,  
        NULL );

    // Now display the string.
    ss << "Error: " << (LPTSTR) lpMessageBuffer << std::endl;

    // Free the buffer allocated by the system.
    LocalFree( lpMessageBuffer ); 
    FreeLibrary(Hand);
}

void addr2line(std::wstringstream& ss, void const * const addr)
{
    static constexpr size_t sizeOfStruct = sizeof(IMAGEHLP_SYMBOL) + 255;
    static char symbolBuffer[sizeOfStruct];
    
    memset(symbolBuffer, 0, sizeOfStruct);

    IMAGEHLP_SYMBOL *symbol = (IMAGEHLP_SYMBOL *) symbolBuffer;

    symbol->SizeOfStruct = sizeOfStruct;
    symbol->MaxNameLength = 254;

    DWORD displacement = 0;

    if (SymGetSymFromAddr(GetCurrentProcess(),
                          (DWORD) addr,
                          &displacement,
                          symbol)) {
        IMAGEHLP_LINE line;
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE);

        if (SymGetLineFromAddr(GetCurrentProcess(),
                                (DWORD) addr,
                                &displacement,
                                &line)) {
            ss << "0x" << std::hex << addr << ": " << symbol->Name << " line " << std::dec << line.LineNumber << std::endl;
        }
        else {
            ss << "0x" << std::hex << addr << ": " << symbol->Name << std::endl;
        }
    }
    else {
        ss << "0x" << std::hex << addr << std::endl;
    }
}

void windows_print_stacktrace(std::wstringstream& ss, CONTEXT* context)
{
    SymInitialize(GetCurrentProcess(), 0, true);

    STACKFRAME frame = { 0 };

    /* setup initial stack frame */
    frame.AddrPC.Offset         = context->Eip;
    frame.AddrPC.Mode           = AddrModeFlat;
    frame.AddrStack.Offset      = context->Esp;
    frame.AddrStack.Mode        = AddrModeFlat;
    frame.AddrFrame.Offset      = context->Ebp;
    frame.AddrFrame.Mode        = AddrModeFlat;

    while (StackWalk(IMAGE_FILE_MACHINE_I386 ,
                     GetCurrentProcess(),
                     GetCurrentThread(),
                     &frame,
                     context,
                     0,
                     SymFunctionTableAccess,
                     SymGetModuleBase,
                     0 ) )
    {
        addr2line(ss, (void *) frame.AddrPC.Offset);
    }

    SymCleanup( GetCurrentProcess() );
}

#endif // _WIN64

LONG WINAPI TopLevelExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo)
{
    auto exCode = pExceptionInfo->ExceptionRecord->ExceptionCode;

    std::wstringstream ss;

    DisplayError(ss, exCode);
    
    if (exCode != EXCEPTION_STACK_OVERFLOW) {
        windows_print_stacktrace(ss, pExceptionInfo->ContextRecord);
    }
    else {
        addr2line(ss, (void *) pExceptionInfo->ContextRecord->Rip);
    }

    MessageBoxW(nullptr, ss.str().c_str(), L"InFormant: unhandled exception", MB_OK | MB_ICONSTOP);

    exit(EXIT_FAILURE);

    return EXCEPTION_EXECUTE_HANDLER;
}

void StdExceptionHandler(const std::exception& e)
{
    std::wstringstream ss;

    ss << "Exception: " << e.what() << std::endl << std::endl;
    
    CONTEXT context;
    RtlCaptureContext(&context);
    
    windows_print_stacktrace(ss, &context);

    MessageBoxW(nullptr, ss.str().c_str(), L"InFormant: caught exception", MB_OK | MB_ICONSTOP);

    exit(EXIT_FAILURE);
}


#endif // _WIN32