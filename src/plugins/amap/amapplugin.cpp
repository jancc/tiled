#include "amapplugin.h"

bool Amap::AmapPlugin::write(const Tiled::Map *map, const QString &fileName, Tiled::FileFormat::Options options)
{

}

QString Amap::AmapPlugin::shortName() const
{
    return tr("AMAP files (*.amap)");
}

QString Amap::AmapPlugin::nameFilter() const
{
    return QLatin1String("amap");
}
