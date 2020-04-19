#ifndef QWASMSETTINGS_H
#define QWASMSETTINGS_H

#include <emscripten/val.h>
#include <QString>
#include <QVariant>

class QWasmSettings
{
public:
    enum Status { NoError };

    QWasmSettings()
        : mStore(emscripten::val::global("localStorage"))
    {
        mDontPersist = mStore.isNull();
    }

    void beginGroup(const QString& group) {
        mGroup = group;
    }

    void endGroup() {
        mGroup = "";
    }

    constexpr Status status() { return NoError; }

    void setValue(const QString& key, const QVariant& value) {
        if (!mDontPersist) {
            QString realKey = mGroup.isEmpty() ? key : QString("%1.%2").arg(mGroup, key);
            mStore.call<void>("setItem",
                              realKey.toStdString(),
                              value.toString().toStdString());
        }
    }

    QVariant value(const QString& key, const QVariant& defaultValue) {
        if (mDontPersist) {
            return defaultValue;
        }
        QString realKey = mGroup.isEmpty() ? key : QString("%1.%2").arg(mGroup, key);
        emscripten::val value = mStore.call<emscripten::val>("getItem", realKey.toStdString());
        if (value.isNull()) {
            setValue(key, defaultValue);
            return defaultValue;
        }
        return QString::fromStdString(value.as<std::string>());
    }

private:
    emscripten::val mStore;
    bool mDontPersist;
    QString mGroup;
};

#define QSettings QWasmSettings

#endif // QWASMSETTINGS_H
