#include "amapplugin.h"

#include "grouplayer.h"
#include "map.h"
#include "savefile.h"
#include "tile.h"
#include "tilelayer.h"
#include "objectgroup.h"
#include "mapobject.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <zlib.h>

#include <vector>

using namespace Tiled;

static const unsigned char VERSION = 1;

namespace Amap {

template <typename T>
std::vector<char> compressVector(const std::vector<T> &data)
{
    uLongf srcSize = data.size() * sizeof(T);

    std::vector<char> compressed(compressBound(srcSize));

    Bytef *dest = reinterpret_cast<Bytef*>(compressed.data());
    uLongf destSize = compressed.size();
    const Bytef *src = reinterpret_cast<const Bytef*>(data.data());

    compress(dest, &destSize, src, srcSize);

    compressed.resize(destSize);
    return compressed;
}

bool AmapPlugin::write(const Tiled::Map *map, const QString &fileName, Tiled::FileFormat::Options options)
{
    Q_UNUSED(options)

    if(map->infinite()) {
        mError = QCoreApplication::translate("AMAP Error", "Format does not support infinite maps.");
        return false;
    }

    SaveFile file(fileName);

    if (!file.open(QIODevice::WriteOnly)) {
        mError = QCoreApplication::translate("File Errors", "Could not open file for writing.");
        return false;
    }

    QDataStream data(file.device());
    data.setByteOrder(QDataStream::LittleEndian);

    quint32 tileLayerCount = 0;
    for(Layer *layer : map->layers()) if(layer->isTileLayer()) tileLayerCount++;

    data.writeRawData("AMAP", 4);
    data << quint8(VERSION);
    data << quint32(map->width()) << quint32(map->height()) << tileLayerCount;

    std::vector<quint8> collisions(map->height() * map->width(), 0);

    const ObjectGroup *objectGroup = nullptr;

    // write tiles

    for(Layer *layer : map->layers()) {
        if(!layer->isTileLayer() && layer->isObjectGroup()) {
            // since we iterate over all layers, might aswell use the time to extract the object layer here
            if(objectGroup) {
                mError = QCoreApplication::translate("AMAP Error", "More than one object group is not supported.");
                return false;
            }
            objectGroup = layer->asObjectGroup();
            continue;
        } else if(!layer->isTileLayer()) {
            mError = QCoreApplication::translate("AMAP Error", "Unsupported layer tile in map.");
            return false;
        }
        const TileLayer *tileLayer = layer->asTileLayer();

        std::vector<qint16> layerdata(map->height() * map->width(), 0);

        for(int y = 0; y < map->height(); y++) {
            for(int x = 0; x < map->width(); x++) {
                const Cell &cell = tileLayer->cellAt(x, y);
                const Tile *tile = cell.tile();
                if(tile) {
                    layerdata[y * map->width() + x] = qint16(tile->id());

                    if(tile->objectGroup()) {
                        collisions[y * map->width() + x] = 1;
                    }
                } else {
                    layerdata[y * map->width() + x] = -1;
                }
            }
        }

        std::vector<char> compressedLayerdata = compressVector(layerdata);
        data << quint32(compressedLayerdata.size());
        data.writeRawData(compressedLayerdata.data(), compressedLayerdata.size());
    }

    if(!objectGroup) {
        mError = QCoreApplication::translate("AMAP Error", "You need exactly one object group in your map.");
        return false;
    }

    // write collisions

    std::vector<char> compressedCollisions = compressVector(collisions);
    data << quint32(compressedCollisions.size());
    data.writeRawData(compressedCollisions.data(), compressedCollisions.size());

    // write objects

    data << quint32(objectGroup->objectCount());
    for(MapObject *object : objectGroup->objects()) {
        std::string name = object->name().toLatin1().toStdString();
        if(name.length() > 8) {
            mError = QCoreApplication::translate("AMAP Error", "Object name to long (max 8 Latin1 bytes).");
            return false;
        }
        name.resize(8, '\0');
        data.writeRawData(name.data(), name.length());

        data << quint16(object->x() / map->tileSize().width());
        data << quint16(object->y() / map->tileSize().height());
    }

    file.commit();
    return true;
}

QString AmapPlugin::shortName() const
{
    return tr("amap");
}

QString AmapPlugin::nameFilter() const
{
    return QLatin1String("AMAP files (*.amap)");
}

}
