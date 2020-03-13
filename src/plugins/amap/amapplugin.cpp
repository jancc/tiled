#include "amapplugin.h"

#include "grouplayer.h"
#include "map.h"
#include "savefile.h"
#include "tile.h"
#include "tilelayer.h"

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

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = QCoreApplication::translate("File Errors", "Could not open file for writing.");
        return false;
    }

    QDataStream data(file.device());
    data.setByteOrder(QDataStream::LittleEndian);

    data.writeRawData("AMAP", 4);
    data << quint8(VERSION);
    data << quint32(map->width()) << quint32(map->height()) << quint32(map->layerCount());

    std::vector<quint8> collisions(map->height() * map->width(), 0);

    for(Layer *layer : map->layers()) {
        if(!layer->isTileLayer()) continue;
        const TileLayer *tileLayer = static_cast<const TileLayer*>(layer);

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

    std::vector<char> compressedCollisions = compressVector(collisions);
    data << quint32(compressedCollisions.size());
    data.writeRawData(compressedCollisions.data(), compressedCollisions.size());

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
