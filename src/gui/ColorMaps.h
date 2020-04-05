#ifndef COLOR_MAPS_H
#define COLOR_MAPS_H

#include <QColor>
#include <QString>
#include <vector>
#include <map>

using ColorMap = std::vector<QColor>;

extern std::map<QString, ColorMap> colorMaps;

void buildColorMaps();

#endif // COLOR_MAPS_H
