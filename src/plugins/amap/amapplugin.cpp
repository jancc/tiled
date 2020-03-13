#include "amapplugin.h"

#include "grouplayer.h"
#include "map.h"
#include "savefile.h"
#include "tile.h"
#include "tilelayer.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

#include <vector>

using namespace Tiled;

static const unsigned char VERSION = 0;

bool Amap::AmapPlugin::write(const Tiled::Map *map, const QString &fileName, Tiled::FileFormat::Options options)
{
    Q_UNUSED(options)

    if(map->infinite()) {
        mError = QCoreApplication::translate("AMAP Error", "Format does not support infinite maps.");
        return false;
    }

    SaveFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = QCoreApplication::translate("File Errors", "Could not open file for writing.");
        return false;
    }

    QDataStream data(file.device());
    data.setByteOrder(QDataStream::LittleEndian);

    data << quint8('A') << quint8('M') << quint8('A') << quint8('P');
    data << quint8(VERSION);
    data << quint32(map->width()) << quint32(map->height()) << quint32(map->layerCount());

    std::vector<quint8> collisions(map->height() * map->width(), 0);

    for(Layer *layer : map->layers()) {
        if(!layer->isTileLayer()) continue;
        const TileLayer *tileLayer = static_cast<const TileLayer*>(layer);

        for(int y = 0; y < map->height(); y++) {
            for(int x = 0; x < map->width(); x++) {
                const Cell &cell = tileLayer->cellAt(x, y);
                const Tile *tile = cell.tile();
                if(tile) {
                    data << qint16(tile->id());

                    if(tile->objectGroup()) {
                        collisions[y * map->width() + x] = 1;
                    }
                } else {
                    data << qint16(-1);
                }
            }
        }
    }

    for(quint8 collision : collisions) data << quint8(collision);
    file.commit();
    return true;
}

QString Amap::AmapPlugin::shortName() const
{
    return tr("amap");
}

QString Amap::AmapPlugin::nameFilter() const
{
    return QLatin1String("AMAP files (*.amap)");
}
