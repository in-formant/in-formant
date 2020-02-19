#include <QtPlugin>

#ifdef _WIN32
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#endif

#ifdef __linux
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin);
#endif