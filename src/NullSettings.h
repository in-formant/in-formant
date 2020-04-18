#ifndef NULL_SETTINGS_H
#define NULL_SETTINGS_H

#include <QVariant>

class NullSettings {
public:
    inline NullSettings() {}

    template<typename A>
    inline void beginGroup(const A&) {}

    inline void endGroup() {}

    template<typename A, typename B>
    inline void setValue(const A&, const B&) {}

    template<typename A>
    inline QVariant value(const A&, const QVariant& v) { return v; }
};

#define QSettings NullSettings

#endif // NULL_SETTINGS_H
