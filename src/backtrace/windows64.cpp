#include "backtrace.h"

#include <windows.h>
#include <winternl.h>
#include <imagehlp.h>
#include <stdio.h>
#include <tchar.h>

void DisplayError(DWORD NTStatusMessage)
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
    _ftprintf(stderr, _T("Error: %s\n"), (LPTSTR) lpMessageBuffer);

    // Free the buffer allocated by the system.
    LocalFree( lpMessageBuffer ); 
    FreeLibrary(Hand);
}

void addr2line(void const * const addr)
{
    static constexpr size_t sizeOfStruct = sizeof(IMAGEHLP_SYMBOL) + 255;
    static char symbolBuffer[sizeOfStruct];
    
    memset(symbolBuffer, 0, sizeOfStruct);

    IMAGEHLP_SYMBOL *symbol = (IMAGEHLP_SYMBOL *) symbolBuffer;

    symbol->SizeOfStruct = sizeOfStruct;
    symbol->MaxNameLength = 254;

    DWORD64 displacement = 0;

    if (SymGetSymFromAddr(GetCurrentProcess(),
                          (DWORD64) addr,
                          &displacement,
                          symbol)) {
        fprintf(stderr, "0x%08x %s\n", addr, symbol->Name);
    }
    else {
        fprintf(stderr, "0x%08x\n", addr);
    }
}

void windows_print_stacktrace(CONTEXT* context)
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
        addr2line((void *) frame.AddrPC.Offset);
    }

    SymCleanup( GetCurrentProcess() );
}

LONG WINAPI windows_exception_handler(EXCEPTION_POINTERS * ExceptionInfo)
{
    auto exCode = ExceptionInfo->ExceptionRecord->ExceptionCode;
    
    DisplayError(exCode);

    if (exCode != EXCEPTION_STACK_OVERFLOW) {
        windows_print_stacktrace(ExceptionInfo->ContextRecord);
    }
    else {
        addr2line((void *) ExceptionInfo->ContextRecord->Rip);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

void registerCrashHandler()
{
    SetUnhandledExceptionFilter(windows_exception_handler);
}
