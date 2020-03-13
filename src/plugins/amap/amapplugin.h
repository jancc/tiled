#pragma once

#include "mapformat.h"

#include "amap_global.h"

namespace Amap {

class AMAPSHARED_EXPORT AmapPlugin : public Tiled::WritableMapFormat
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.mapeditor.MapFormat" FILE "plugin.json")

public:
    bool write(const Tiled::Map *map, const QString &fileName, Options options) override;

    QString errorString() const override {
        return mError;
    };

    QString shortName() const override;

protected:
    QString nameFilter() const override;

private:
    QString mError;
};

}
