#pragma once

#include <QtCore/qglobal.h>

#if defined(AMAP_LIBRARY)
#  define AMAPSHARED_EXPORT Q_DECL_EXPORT
#else
#  define AMAPSHARED_EXPORT Q_DECL_IMPORT
#endif
