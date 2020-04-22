#include <QEvent>
#include <QApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QFormLayout>
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

    setWindowFlag(Qt::Window);
    setWindowTitle("Set bindings");

    auto layout = new QFormLayout(this);
    
    for (int i = 0; i < bindActionCount; ++i) {
        layout->addRow(mActionNames[i], new KeybindField(this, static_cast<BindAction>(i)));
    }
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

const BindInput& Keybinds::getBinding(BindAction action)
{
    return mActionMap[int(action)];
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
    if (event->type() == QEvent::MouseButtonPress) {
        const auto mouseEvent = static_cast<QMouseEvent *>(event);
        const auto button = static_cast<Qt::MouseButton>(mouseEvent->button());
        mPressedButtons.insert(button);
        if (mMouseBindings.contains(button)) {
            emitAction(mMouseBindings.value(button), obj, true);
            return true;
        }
    }
    else if (event->type() == QEvent::MouseButtonRelease) {
        const auto mouseEvent = static_cast<QMouseEvent *>(event);
        const auto button = static_cast<Qt::MouseButton>(mouseEvent->button());
        mPressedButtons.remove(button);
        if (mMouseBindings.contains(button)) {
            emitAction(mMouseBindings.value(button), obj, false);
            return true;
        }
    }
    else if (event->type() == QEvent::KeyPress) {
        const auto keyEvent = static_cast<QKeyEvent *>(event);
        const auto key = static_cast<Qt::Key>(keyEvent->key());
        mPressedKeys.insert(key);
        if (mKeyBindings.contains(key)) {
            emitAction(mKeyBindings.value(key), obj, true);
            return true;
        }
    }
    else if (event->type() == QEvent::KeyRelease) {
        const auto keyEvent = static_cast<QKeyEvent *>(event);
        const auto key = static_cast<Qt::Key>(keyEvent->key());
        mPressedKeys.remove(key);
        if (mKeyBindings.contains(key)) {
            emitAction(mKeyBindings.value(key), obj, false);
            return true;
        }
    }
    else if (event->type() == QEvent::MouseMove) {
        bool didSomething = false;

        BindInput actionCursor = mActionMap[int(BindAction::MoveCursor)];
        if ((actionCursor.type == BindType::Key && mPressedKeys.contains(actionCursor.key))
                || (actionCursor.type == BindType::Mouse && mPressedButtons.contains(actionCursor.button))) {
            emitAction(BindAction::MoveCursor, obj, true);
            didSomething = true;
        }

        BindInput actionSine = mActionMap[int(BindAction::SineWave)];
        if ((actionSine.type == BindType::Key && mPressedKeys.contains(actionSine.key))
                || (actionSine.type == BindType::Mouse && mPressedButtons.contains(actionSine.button))) {
            emitAction(BindAction::SineWave, obj, true);
            didSomething = true;
        }

        return didSomething;
    }

    return false;
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

QString bindToString(const BindInput& input)
{
    if (input.type == BindType::None) {
        return "None";
    }
    else if (input.type == BindType::Key) {
        return keyToString(input.key);
    }
    else if (input.type == BindType::Mouse) {
        return mouseButtonToString(input.button);
    }
    else {
        return "Unknown";
    }
}

QString keyToString(Qt::Key key)
{
    return QKeySequence(key).toString();
}

QString mouseButtonToString(Qt::MouseButton button)
{
    if (button == Qt::NoButton)      return "NoButton";
    if (button == Qt::LeftButton)    return "LeftButton";
    if (button == Qt::RightButton)   return "RightButton";
    if (button == Qt::MiddleButton)  return "MiddleButton";
    if (button == Qt::BackButton)    return "BackButton";
    if (button == Qt::ForwardButton) return "ForwardButton";
    if (button == Qt::TaskButton)    return "TaskButton";
    if (button == Qt::ExtraButton4)  return "ExtraButton4";
    if (button == Qt::ExtraButton5)  return "ExtraButton5";
    if (button == Qt::ExtraButton6)  return "ExtraButton6";
    if (button == Qt::ExtraButton7)  return "ExtraButton7";
    if (button == Qt::ExtraButton8)  return "ExtraButton8";
    if (button == Qt::ExtraButton9)  return "ExtraButton9";
    if (button == Qt::ExtraButton10) return "ExtraButton10";
    if (button == Qt::ExtraButton11) return "ExtraButton11";
    if (button == Qt::ExtraButton12) return "ExtraButton12";
    if (button == Qt::ExtraButton13) return "ExtraButton13";
    if (button == Qt::ExtraButton14) return "ExtraButton14";
    if (button == Qt::ExtraButton15) return "ExtraButton15";
    if (button == Qt::ExtraButton16) return "ExtraButton16";
    if (button == Qt::ExtraButton17) return "ExtraButton17";
    if (button == Qt::ExtraButton18) return "ExtraButton18";
    if (button == Qt::ExtraButton19) return "ExtraButton19";
    if (button == Qt::ExtraButton20) return "ExtraButton20";
    if (button == Qt::ExtraButton21) return "ExtraButton21";
    if (button == Qt::ExtraButton22) return "ExtraButton22";
    if (button == Qt::ExtraButton23) return "ExtraButton23";
    if (button == Qt::ExtraButton24) return "ExtraButton24";
    return "NoButton";
}

KeybindField::KeybindField(Keybinds *keybinds, BindAction action)
    : mKeybinds(keybinds), mAction(action), mSelected(false)
{
    setContextMenuPolicy(Qt::PreventContextMenu);
    setReadOnly(true);
    updateText();
}

void KeybindField::mousePressEvent(QMouseEvent *e)
{
    e->accept();

    if (!mSelected) {
        mSelected = true;
        
        grabMouse();
        grabKeyboard();
    }
    else {
        mKeybinds->unbind(mAction);
        mKeybinds->bindMouse(e->button(), mAction);

        releaseMouse();
        releaseKeyboard();

        updateText();

        mSelected = false;
    }
}

void KeybindField::keyPressEvent(QKeyEvent *e)
{
    e->accept();

    if (mSelected) { 
        mKeybinds->unbind(mAction);
        mKeybinds->bindKey(static_cast<Qt::Key>(e->key()), mAction);

        releaseMouse();
        releaseKeyboard();

        updateText();
        
        mSelected = false;
    }
}

void KeybindField::updateText()
{
    setText(bindToString(mKeybinds->getBinding(mAction)));
}
