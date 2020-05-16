#include <QtPlugin>

#if defined(Q_OS_WIN)
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#elif defined(Q_OS_ANDROID)
// Plugin is linked dynamically.
//Q_IMPORT_PLUGIN(QAndroidPlatformIntegrationPlugin);
#elif defined(Q_OS_LINUX)
//Q_IMPORT_PLUGIN(QXcbIntegrationPlugin);
//Q_IMPORT_PLUGIN(QXcbGlxIntegrationPlugin);
#elif defined(Q_OS_MACOS)
Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin);
#elif defined(Q_OS_WASM)
Q_IMPORT_PLUGIN(QWasmIntegrationPlugin);
#else
#error "Unsupported target operating system"
#endif


