#pragma once

#include <QtCore/qglobal.h>

#if defined(EQUIPMENT_LIBRARY)
#  define EQUIPMENT_EXPORT Q_DECL_EXPORT
#else
#  define EQUIPMENT_EXPORT Q_DECL_IMPORT
#endif
