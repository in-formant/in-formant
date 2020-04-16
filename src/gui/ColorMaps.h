#ifndef COLOR_MAPS_H
#define COLOR_MAPS_H

#include <QColor>
#include <QString>
#include "rpmalloc.h"

using ColorMap = rpm::vector<QColor>;

extern rpm::map<QString, ColorMap> colorMaps;

void buildColorMaps();

#endif // COLOR_MAPS_H
