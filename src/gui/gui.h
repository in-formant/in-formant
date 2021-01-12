#ifndef GUI_H
#define GUI_H

#include "../context/contextmanager.h"
#include "../modules/target/qt5quick/qt5quick.h"

#include "canvas.h"

#include <memory>
#include <QMutex>

extern std::unique_ptr<Main::ContextManager> pManager;
extern QMutex *pManagerMutex;

extern Module::Target::Qt5Quick *pTarget;

namespace Gui
{
    int runApp(int argc, char **argv);
}

#endif // GUI_H
