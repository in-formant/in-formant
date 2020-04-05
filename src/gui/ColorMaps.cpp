#include "ColorMaps.h"

std::map<QString, ColorMap> colorMaps;

static const std::vector<QColor> map1 = {
    };

void buildColorMaps()
{

    colorMaps.emplace(
        "iZotope",
        std::vector<QColor>{
            QColor(0, 0, 0),
            QColor(0, 12, 35),
            QColor(17, 32, 67),
            QColor(47, 50, 62),
            QColor(78, 61, 50),
            QColor(126, 71, 35),
            QColor(183, 86, 20),
            QColor(229, 115, 16),
            QColor(248, 151, 27),
            QColor(255, 206, 55),
            QColor(255, 255, 128),
            QColor(255, 255, 255),
        }
    );
   
    colorMaps.emplace(
        "Heatmap",
        std::vector<QColor>{
            QColor(0, 0, 0),
            QColor(0, 0, 79),
            QColor(80, 0, 123),
            QColor(153, 0, 118),
            QColor(210, 0, 64),
            QColor(245, 31, 0),
            QColor(255, 174, 0),
            QColor(255, 255, 107),
            QColor(255, 255, 255),
        }
    );

    colorMaps.emplace(
        "Legacy",
        std::vector<QColor>{
            QColor(0, 0, 0),
            QColor(38, 38, 128),
            QColor(77, 38, 191),
            QColor(153, 51, 128),
            QColor(255, 64, 38),
            QColor(230, 128, 0),
            QColor(230, 191, 26),
            QColor(230, 230, 128),
            QColor(255, 255, 255),
        }
    );

}
