#include <QtPlugin>

#ifdef Q_OS_WIN
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#endif

#ifdef Q_OS_LINUX
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin);
#endif

#ifdef Q_OS_MACOS
Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin);
#endif
