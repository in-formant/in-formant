#include <QEvent>
#include <QApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <iostream>
#include "Keybinds.h"
#include "../log/simpleQtLogger.h"

Keybinds::Keybinds()
{
    int i;

    i = int(BindAction::MoveCursor);
    mActionNames[i] = "Move canvas cursor";
    mDefaultBinds[i].type = BindType::Mouse;
    mDefaultBinds[i].button = Qt::LeftButton;

    i = int(BindAction::Close);
    mActionNames[i] = "Close current window";
    mDefaultBinds[i].type = BindType::Key;
    mDefaultBinds[i].key = Qt::Key_Escape;

    i = int(BindAction::Pause);
    mActionNames[i] = "Pause/Resume analysis";
    mDefaultBinds[i].type = BindType::Key;
    mDefaultBinds[i].key = Qt::Key_P;

    i = int(BindAction::Fullscreen);
    mActionNames[i] = "Toggle fullscreen";
    mDefaultBinds[i].type = BindType::Key;
    mDefaultBinds[i].key = Qt::Key_F;

    i = int(BindAction::SineWave);
    mActionNames[i] = "Play a sine wave";
    mDefaultBinds[i].type = BindType::Mouse;
    mDefaultBinds[i].button = Qt::RightButton;

    i = int(BindAction::NoiseFilter);
    mActionNames[i] = "Play formant noise";
    mDefaultBinds[i].type = BindType::Mouse;
    mDefaultBinds[i].button = Qt::MiddleButton;

    loadSettings();
}

Keybinds::~Keybinds()
{
    saveSettings();
}

void Keybinds::bindKey(Qt::Key key, BindAction action)
{
    if (isKeyBound(key)) {
        mKeyBindings.remove(key);
    }

    mKeyBindings.insert(key, action);
    mActionMap[int(action)].type = BindType::Key;
    mActionMap[int(action)].key = key;
}

void Keybinds::bindMouse(Qt::MouseButton button, BindAction action)
{
    if (isMouseBound(button)) {
        mMouseBindings.remove(button);
    }

    mMouseBindings.insert(button, action);
    mActionMap[int(action)].type = BindType::Mouse;
    mActionMap[int(action)].button = button;
}

void Keybinds::unbind(BindAction action)
{
    if (mActionMap[int(action)].type == BindType::Key) {
        mKeyBindings.remove(mActionMap[int(action)].key); 
    }
    else if (mActionMap[int(action)].type == BindType::Mouse) {
        mMouseBindings.remove(mActionMap[int(action)].button);
    }
    
    mActionMap[int(action)].type = BindType::None;
}

bool Keybinds::isKeyBound(Qt::Key key)
{
    return mKeyBindings.contains(key);
}

bool Keybinds::isMouseBound(Qt::MouseButton button)
{
    return mMouseBindings.contains(button);
}

bool Keybinds::eventFilter(QObject * obj, QEvent * event)
{
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
        const auto keyEvent = static_cast<QKeyEvent *>(event);
        const Qt::Key key = static_cast<Qt::Key>(keyEvent->key());

        mPressedKeys.insert(key, (event->type() == QEvent::KeyPress));

        if (mKeyBindings.contains(key)) {
            emitAction(mKeyBindings.value(key), obj, mPressedKeys.value(key));
            return true;
        }
    }

    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease) {
        const auto mouseEvent = static_cast<QMouseEvent *>(event);
        const Qt::MouseButton button = static_cast<Qt::MouseButton>(mouseEvent->button());

        if (mMouseBindings.contains(button)) {
            emitAction(mMouseBindings.value(button), obj, (event->type() == QEvent::MouseButtonPress));
            return true;
        }
    }
    
    if (event->type() == QEvent::MouseMove) {
        bool didSomething = false;
        
        // Move cursor.
        BindInput actionCursor = mActionMap[int(BindAction::MoveCursor)];
        if ((actionCursor.type == BindType::Key && mPressedKeys.value(actionCursor.key))
                || (actionCursor.type == BindType::Mouse && (QApplication::mouseButtons() & actionCursor.button) != 0)) {
            emitAction(BindAction::MoveCursor, obj, true);
            didSomething = true;
        }
        
        // Sine wave.
        BindInput actionSine = mActionMap[int(BindAction::SineWave)];
        if ((actionSine.type == BindType::Key && mPressedKeys.value(actionSine.key))
                || (actionSine.type == BindType::Mouse && (QApplication::mouseButtons() & actionSine.button) != 0)) {
            emitAction(BindAction::SineWave, obj, true);
            didSomething = true;
        }

        return didSomething;
    }

    return QObject::eventFilter(obj, event);
}

void Keybinds::emitAction(BindAction action, QObject * source, bool flag)
{
    switch (action) {
    case BindAction::MoveCursor:
        emit actionCursor(source, flag);
        break;
    case BindAction::Close:
        emit actionClose(source, flag);
        break;
    case BindAction::Pause:
        emit actionPause(source, flag);
        break;
    case BindAction::Fullscreen:
        emit actionFullscreen(source, flag);
        break;
    case BindAction::SineWave:
        emit actionSineWave(source, flag);
        break;
    case BindAction::NoiseFilter:
        emit actionNoiseFilter(source, flag);
        break;
    default:
        break;
    }
}

void Keybinds::loadSettings(QSettings& settings)
{
    L_INFO("Loading bindings settings...");

    if (settings.status() != QSettings::NoError) {
        QTimer::singleShot(10, [&]() { loadSettings(settings); });
    }
    else {
        settings.beginGroup("bindings");
       
        for (int i = 0; i < bindActionCount; ++i) {
            QString string = settings.value(QString("action-%1").arg(i), stringFromInput(mDefaultBinds[i])).toString();
            mActionMap[i] = inputFromString(string);

            if (mActionMap[i].type == BindType::Key) {
                mKeyBindings.insert(mActionMap[i].key, static_cast<BindAction>(i));
            }
            else if (mActionMap[i].type == BindType::Mouse) {
                mMouseBindings.insert(mActionMap[i].button, static_cast<BindAction>(i));
            }
        }

        settings.endGroup();
    }
}

void Keybinds::saveSettings(QSettings& settings)
{
    L_INFO("Saving bindings settings...");

    if (settings.status() != QSettings::NoError) {
        QTimer::singleShot(10, [&]() { saveSettings(settings); });
    }
    else {
        settings.beginGroup("bindings");
       
        for (int i = 0; i < bindActionCount; ++i) {
            settings.setValue(QString("action-%1").arg(i), stringFromInput(mActionMap[i]));
        }

        settings.endGroup();
    }
}

QString stringFromInput(const BindInput& input)
{
    switch (input.type) {
    case BindType::None:
        return "n:0";
    case BindType::Key:
        return QString("k:%1").arg(static_cast<int>(input.key));
    case BindType::Mouse:
        return QString("m:%1").arg(static_cast<int>(input.button));
    default:
        return "n:1";
    }
}

BindInput inputFromString(const QString& string)
{
    BindInput input;

    bool prefixNone = string.startsWith("n:");
    bool prefixKey = string.startsWith("k:");
    bool prefixMouse = string.startsWith("m:");

    if (prefixNone || prefixKey || prefixMouse) {
        bool ok;
        int value = string.rightRef(string.size() - 2).toInt(&ok);

        if (!ok) {
            input.type = BindType::Invalid;
        }
        else {
            if (prefixNone) {
                if (value == 0) {
                    input.type = BindType::None;
                }
                else {
                    input.type = BindType::Invalid;
                }
            }
            else if (prefixKey) {
                input.type = BindType::Key;
                input.key = static_cast<Qt::Key>(value);
            }
            else if (prefixMouse) {
                input.type = BindType::Mouse;
                input.button = static_cast<Qt::MouseButton>(value);
            }
        }
    }
    else {
        input.type = BindType::Invalid;
    }

    return input;
}
