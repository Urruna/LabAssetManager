#pragma once

#include <QtCore/qglobal.h>

#if defined(MAINTENANCE_LIBRARY)
#  define MAINTENANCE_EXPORT Q_DECL_EXPORT
#else
#  define MAINTENANCE_EXPORT Q_DECL_IMPORT
#endif
