#ifndef KEYBINDS_H
#define KEYBINDS_H

#include <Qt>
#include <QObject>
#include <QSettings>
#include <QTimer>
#include <QSet>
#include <QMap>

enum class BindType { None = 0, Key, Mouse, Invalid };

struct BindInput {
    BindType type;
    union {
        Qt::Key key;
        Qt::MouseButton button;
    };
};

QString stringFromInput(const BindInput& input);
BindInput inputFromString(const QString& string);

enum class BindAction {
    MoveCursor = 0,
    Close,
    Pause,
    Fullscreen,
    SineWave,
    NoiseFilter,
    LastBindAction
};

constexpr int bindActionCount = static_cast<int>(BindAction::LastBindAction);

class Keybinds : public QObject {
    Q_OBJECT
public:
    Keybinds();
    ~Keybinds();

    void bindKey(Qt::Key key, BindAction action);
    void bindMouse(Qt::MouseButton button, BindAction action);
    void unbind(BindAction action);
    
    bool isKeyBound(Qt::Key key);
    bool isMouseBound(Qt::MouseButton button);

    void loadSettings() { QSettings s; loadSettings(s); }
    void saveSettings() { QSettings s; saveSettings(s); }

protected:
    bool eventFilter(QObject * obj, QEvent * event) override;

signals:
    void actionCursor(QObject *, bool);

    void actionClose(QObject *, bool);
    void actionPause(QObject *, bool);
    void actionFullscreen(QObject *, bool);

    void actionSineWave(QObject *, bool);
    void actionNoiseFilter(QObject *, bool);

private:
    void emitAction(BindAction action, QObject * source, bool flag);

    void loadSettings(QSettings& settings);
    void saveSettings(QSettings& settings);

    QMap<Qt::Key, BindAction> mKeyBindings;
    QMap<Qt::MouseButton, BindAction> mMouseBindings;

    std::array<BindInput, bindActionCount> mActionMap;
    
    std::array<QString, bindActionCount> mActionNames;
    std::array<BindInput, bindActionCount> mDefaultBinds;

    QMap<Qt::Key, bool> mPressedKeys;
};

#endif // KEYBINDS_H
