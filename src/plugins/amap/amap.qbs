import qbs 1.0

TiledPlugin {
    cpp.defines: base.concat(["AMAP_LIBRARY"])

    files: [
        "amap_global.h",
        "amapplugin.cpp",
        "amapplugin.h",
        "plugin.json",
    ]
}
